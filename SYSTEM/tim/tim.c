#include "tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

/* ================================================================
 *  测量方法：TIM2 双通道输入捕获 + 多周期累积平均
 *
 *  计数器频率：72MHz / (PSC+1) = 72MHz / 8 = 9MHz
 *              → 1 count ≈ 111ns，比原 1MHz 精度提高 9 倍
 *
 *  精度（9MHz + 8周期平均，等效计数值）：
 *    10kHz → 单周期 900cnt → 8周期平均 7200cnt → ±0.014%
 *     1kHz → 单周期 9000cnt                      → ±0.011%
 *     1Hz  → 单周期 9,000,000cnt → N=1 立即更新  → ±0.000011%
 *
 *  自适应更新策略（避免低频等待过久）：
 *    累积 ≥ ACCUM_MAX_N 个周期，或累积时间 ≥ 500ms，取先到者更新显示
 *    → 1Hz 每秒更新一次（N=1）；10kHz 每 0.9ms 更新一次（N=8）
 *
 *  无信号检测：
 *    连续 200 次溢出（≈1.46s）无有效捕获 → 判定无信号
 *    溢出周期 = 65536 / 9MHz ≈ 7.28ms
 *
 *  32位时间戳扩展：
 *    timestamp = (overflow_count << 16) | CCR
 *    支持最长 9,900,000 counts（≈1.1s = 0.91Hz）的周期无回绕溢出
 * ================================================================ */

#define TIMER_FREQ_HZ    9000000UL   /* 72MHz / 8 = 9MHz */
#define PSC_VALUE        7           /* TIM_Prescaler 寄存器值 */

#define ACCUM_MAX_N      8           /* 最多累积周期数 */
#define ACCUM_MAX_CNT    4500000UL   /* 累积时间上限：500ms × 9MHz */

/* 无信号阈值：200 × 7.28ms ≈ 1.46s */
#define NOSIG_THRESHOLD  200u

/* 有效周期区间（timer counts）：
 *   下限 720 ≈ 12.5kHz，为 10kHz 边界预留 ±1 计数抖动裕量
 *   上限 9,900,000 ≈ 0.91Hz，略低于 1Hz 以拒绝过长无效捕获 */
#define PERIOD_MIN_CNT   720u
#define PERIOD_MAX_CNT   9900000UL

/* ---- 中断与主程序共享变量 ---- */
static volatile uint32_t s_ovf_cnt       = 0;
static volatile uint32_t s_prev_rising   = 0;
static volatile uint32_t s_falling_ext   = 0;
static volatile uint8_t  s_first_cap     = 1;

/* 多周期累积缓冲 */
static volatile uint32_t s_accum_period  = 0;
static volatile uint32_t s_accum_high    = 0;
static volatile uint8_t  s_accum_n       = 0;

/* 对外输出（单位：timer counts） */
static volatile uint32_t g_period_cnt    = 0;
static volatile uint32_t g_high_cnt      = 0;
volatile uint8_t         g_cap_done      = 0;
volatile uint32_t        g_nosig_ovf     = 0;

/* ================================================================
 *  初始化
 * ================================================================ */
void PWM_Capture_Init(void) {
    GPIO_InitTypeDef        gp;
    TIM_TimeBaseInitTypeDef tb;
    TIM_ICInitTypeDef       ic;
    NVIC_InitTypeDef        nv;

    /* PA0 内部下拉：悬空时稳定低电平，不产生假边沿 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    gp.GPIO_Pin   = GPIO_Pin_0;
    gp.GPIO_Mode  = GPIO_Mode_IPD;
    gp.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gp);

    /* TIM2 时钟（APB1×2 = 72MHz） */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /* 时基：PSC=7 → 9MHz；ARR=0xFFFF（16位最大） */
    tb.TIM_Period        = 0xFFFF;
    tb.TIM_Prescaler     = PSC_VALUE;
    tb.TIM_ClockDivision = TIM_CKD_DIV1;
    tb.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &tb);

    /* CH1：上升沿捕获 TI1 (PA0) — 周期测量 */
    ic.TIM_Channel     = TIM_Channel_1;
    ic.TIM_ICPolarity  = TIM_ICPolarity_Rising;
    ic.TIM_ICSelection = TIM_ICSelection_DirectTI;
    ic.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    ic.TIM_ICFilter    = 0x04;   /* 4采样滤波，抑制毛刺 */
    TIM_ICInit(TIM2, &ic);

    /* CH2：下降沿捕获 TI1 (PA0) — 占空比测量 */
    ic.TIM_Channel     = TIM_Channel_2;
    ic.TIM_ICPolarity  = TIM_ICPolarity_Falling;
    ic.TIM_ICSelection = TIM_ICSelection_IndirectTI; /* 复用同一引脚 */
    ic.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    ic.TIM_ICFilter    = 0x04;
    TIM_ICInit(TIM2, &ic);

    TIM_ClearFlag(TIM2, TIM_FLAG_CC1 | TIM_FLAG_CC2 | TIM_FLAG_Update);
    TIM_ITConfig(TIM2, TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_Update, ENABLE);

    nv.NVIC_IRQChannel                   = TIM2_IRQn;
    nv.NVIC_IRQChannelPreemptionPriority = 1;
    nv.NVIC_IRQChannelSubPriority        = 0;
    nv.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nv);

    TIM_Cmd(TIM2, ENABLE);
}

/* ================================================================
 *  中断服务核心（由 stm32f10x_it.c 的 TIM2_IRQHandler 调用）
 * ================================================================ */
void PWM_TIM2_IRQHandler(void) {
    /* C90: 所有变量声明必须在函数/块开头 */
    uint8_t  update_now = 0;
    uint32_t ccr2, ccr1, ovf, rising, period, high_time;

    /* ① 溢出（Update）：先处理，保证 s_ovf_cnt 在后续读取时已更新 */
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        s_ovf_cnt++;
        g_nosig_ovf++;
        update_now = 1;
    }

    /* ② 下降沿：记录高电平结束时间戳 */
    if (TIM_GetITStatus(TIM2, TIM_IT_CC2) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);
        ccr2 = (uint16_t)TIM_GetCapture2(TIM2);
        ovf  = s_ovf_cnt;
        /* 竞态修正：本次 ISR 已计溢出，但捕获发生在溢出之前 */
        if (update_now && (ccr2 >= 0x8000u)) ovf--;
        s_falling_ext = (ovf << 16) | ccr2;
    }

    /* ③ 上升沿：计算周期与高电平时间，累积平均后更新结果 */
    if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);
        ccr1 = (uint16_t)TIM_GetCapture1(TIM2);
        ovf  = s_ovf_cnt;
        if (update_now && (ccr1 >= 0x8000u)) ovf--;
        rising = (ovf << 16) | ccr1;

        /* 若无信号超时已触发，清除历史状态重新同步 */
        if (g_nosig_ovf >= NOSIG_THRESHOLD) {
            g_nosig_ovf    = 0;
            s_first_cap    = 1;
            s_accum_n      = 0;
            s_accum_period = 0;
            s_accum_high   = 0;
        }

        if (!s_first_cap) {
            period    = rising - s_prev_rising;   /* 无符号自动处理回绕 */
            high_time = s_falling_ext - s_prev_rising;

            if (period >= PERIOD_MIN_CNT && period <= PERIOD_MAX_CNT &&
                high_time <= period) {

                /* 有效捕获 → 复位无信号计数器 */
                g_nosig_ovf = 0;

                /* 累积 */
                s_accum_period += period;
                s_accum_high   += high_time;
                s_accum_n++;

                /* 满足更新条件：
                 *   a) 累积了足够多的周期（高频时提高精度）
                 *   b) 累积时间超过 500ms（低频时避免等待太久）*/
                if (s_accum_n >= ACCUM_MAX_N ||
                    s_accum_period >= ACCUM_MAX_CNT) {
                    g_period_cnt   = s_accum_period / s_accum_n;
                    g_high_cnt     = s_accum_high   / s_accum_n;
                    g_cap_done     = 1;
                    s_accum_period = 0;
                    s_accum_high   = 0;
                    s_accum_n      = 0;
                }
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

/* 频率（Hz）：四舍五入整除 */
uint32_t PWM_Get_Freq(void) {
    uint32_t p = g_period_cnt;
    if (p == 0) return 0;
    return (TIMER_FREQ_HZ + p / 2) / p;
}

/* 占空比（0-100%）：四舍五入 */
uint8_t PWM_Get_Duty(void) {
    uint32_t p = g_period_cnt;
    uint32_t h = g_high_cnt;
    uint32_t d;
    if (p == 0) return 0;
    d = (h * 100UL + p / 2) / p;
    return (d > 100u) ? 100u : (uint8_t)d;
}
