#include "tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

/*
 * ============================================================
 *  算法说明
 *
 *  使用 TIM2 CH1(上升沿) + CH2(下降沿) 同时捕获 PA0 信号。
 *  不使用从模式复位，而是记录两次上升沿时间戳差值作为周期，
 *  下降沿时间戳与前一上升沿之差作为高电平时间。
 *
 *  为支持低至 1Hz（周期 1s = 1,000,000 µs），16位计数器
 *  (ARR=0xFFFF, 溢出周期 65.536ms) 会产生约 15 次溢出。
 *  通过软件溢出计数器将时间戳扩展为 32 位：
 *
 *    timestamp_32 = ovf_cnt × 65536 + CCR 值
 *
 *  处理溢出与捕获同时发生的竞态：
 *    - Update 中断先于 CC 中断处理
 *    - 若本次 IRQ 已处理过溢出(update_now=1)，
 *      且 CCR < 0x8000，说明溢出发生在捕获之前 → 当前 ovf 正确
 *      且 CCR >= 0x8000，说明溢出发生在捕获之后 → ovf 需 -1 修正
 *
 *  无信号检测：每次有效上升沿捕获时清零 g_nosig_ovf；
 *  否则每次溢出 +1；超过阈值(≈2s)则判定无信号。
 * ============================================================
 */

/* ---- 中断与主程序共享变量 ---- */
static volatile uint32_t s_ovf_cnt       = 0;   /* 溢出计数（连续累加） */
static volatile uint32_t s_prev_rising   = 0;   /* 前一上升沿 32位时间戳 */
static volatile uint32_t s_falling_ext   = 0;   /* 最近下降沿 32位时间戳 */
static volatile uint8_t  s_first_cap     = 1;   /* 首次捕获标志（跳过第一周期） */

volatile uint32_t g_period_us  = 0;
volatile uint32_t g_high_us    = 0;
volatile uint8_t  g_cap_done   = 0;
volatile uint32_t g_nosig_ovf  = 0;    /* 自上次有效捕获后的溢出次数 */

/* 无信号判定阈值：31 次溢出 × 65.536ms ≈ 2.03s */
#define NOSIG_THRESHOLD  31u

/* ================================================================
 *  初始化
 * ================================================================ */
void PWM_Capture_Init(void) {
    GPIO_InitTypeDef      gp;
    TIM_TimeBaseInitTypeDef tb;
    TIM_ICInitTypeDef     ic;
    NVIC_InitTypeDef      nv;

    /* PA0 内部下拉输入：悬空时引脚稳定低电平，不产生假边沿；
     * 接入信号后信号源低阻抗可轻松驱动，不影响测量 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    gp.GPIO_Pin   = GPIO_Pin_0;
    gp.GPIO_Mode  = GPIO_Mode_IPD;   /* 内部下拉，防悬空噪声 */
    gp.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gp);

    /* TIM2 时钟（APB1 × 2 = 72MHz） */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /* 时基：PSC=71 → 1MHz 计数；ARR=0xFFFF 最大 16 位 */
    tb.TIM_Period        = 0xFFFF;
    tb.TIM_Prescaler     = 71;
    tb.TIM_ClockDivision = TIM_CKD_DIV1;
    tb.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &tb);

    /* CH1：上升沿捕获 TI1 (PA0) */
    ic.TIM_Channel     = TIM_Channel_1;
    ic.TIM_ICPolarity  = TIM_ICPolarity_Rising;
    ic.TIM_ICSelection = TIM_ICSelection_DirectTI;   /* IC1 ← TI1 */
    ic.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    ic.TIM_ICFilter    = 0x04;    /* 4 采样滤波，抑制毛刺 */
    TIM_ICInit(TIM2, &ic);

    /* CH2：下降沿捕获 TI1 (PA0) — 通过 IndirectTI 复用同一引脚 */
    ic.TIM_Channel     = TIM_Channel_2;
    ic.TIM_ICPolarity  = TIM_ICPolarity_Falling;
    ic.TIM_ICSelection = TIM_ICSelection_IndirectTI; /* IC2 ← TI1 */
    ic.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    ic.TIM_ICFilter    = 0x04;
    TIM_ICInit(TIM2, &ic);

    /* 使能中断：CC1(上升) + CC2(下降) + Update(溢出) */
    TIM_ClearFlag(TIM2, TIM_FLAG_CC1 | TIM_FLAG_CC2 | TIM_FLAG_Update);
    TIM_ITConfig(TIM2, TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_Update, ENABLE);

    /* NVIC —— 抢占优先级 1，低于 SysTick(0) */
    nv.NVIC_IRQChannel                   = TIM2_IRQn;
    nv.NVIC_IRQChannelPreemptionPriority = 1;
    nv.NVIC_IRQChannelSubPriority        = 0;
    nv.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nv);

    TIM_Cmd(TIM2, ENABLE);
}

/* ================================================================
 *  中断服务核心（由 stm32f10x_it.c 中 TIM2_IRQHandler 调用）
 * ================================================================ */
void PWM_TIM2_IRQHandler(void) {
    uint8_t update_now = 0;

    /* ① 先处理溢出，确保 s_ovf_cnt 已更新 */
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        s_ovf_cnt++;
        g_nosig_ovf++;
        update_now = 1;
    }

    /* ② 下降沿捕获 → 记录高电平结束时间戳 */
    if (TIM_GetITStatus(TIM2, TIM_IT_CC2) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);
        uint32_t ccr2 = (uint16_t)TIM_GetCapture2(TIM2);
        uint32_t ovf  = s_ovf_cnt;
        /*
         * 竞态修正：若本次 ISR 已处理溢出，且 ccr2 较大，
         * 说明溢出实际发生在捕获之后，ovf 被多算了一次
         */
        if (update_now && (ccr2 >= 0x8000u)) ovf--;
        s_falling_ext = (ovf << 16) | ccr2;
    }

    /* ③ 上升沿捕获 → 计算周期与占空比 */
    if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);
        uint32_t ccr1 = (uint16_t)TIM_GetCapture1(TIM2);
        uint32_t ovf  = s_ovf_cnt;
        if (update_now && (ccr1 >= 0x8000u)) ovf--;
        uint32_t rising = (ovf << 16) | ccr1;

        if (!s_first_cap) {
            uint32_t period    = rising - s_prev_rising;   /* unsigned 自动处理回绕 */
            uint32_t high_time = s_falling_ext - s_prev_rising;

            /*
             * 有效性校验：
             *   1Hz ≤ f ≤ 10kHz  →  period ≤ 1,100,000µs
             *   下限取 80µs(≈12.5kHz)，为 10kHz 边界预留 ±1 计数抖动裕量
             *   high_time 必须 ≤ period (duty ≤ 100%)
             */
            if (period >= 80u && period <= 1100000u && high_time <= period) {
                g_period_us = period;
                g_high_us   = high_time;
                g_cap_done  = 1;
                g_nosig_ovf = 0;   /* 有信号，复位无信号计数器 */
            }
        }
        s_prev_rising = rising;
        s_first_cap   = 0;
    }
}

/* ================================================================
 *  主程序接口
 * ================================================================ */
uint8_t PWM_Capture_Ready(void) {
    if (g_cap_done) { g_cap_done = 0; return 1; }
    return 0;
}

uint8_t PWM_Signal_Present(void) {
    return (g_nosig_ovf < NOSIG_THRESHOLD);
}

uint32_t PWM_Get_Freq(void) {
    uint32_t p = g_period_us;
    if (p == 0) return 0;
    return 1000000UL / p;
}

uint8_t PWM_Get_Duty(void) {
    uint32_t p = g_period_us;
    uint32_t h = g_high_us;
    uint32_t d;
    if (p == 0) return 0;
    d = (h * 100UL + p / 2) / p;   /* 四舍五入 */
    return (d > 100u) ? 100u : (uint8_t)d;
}
