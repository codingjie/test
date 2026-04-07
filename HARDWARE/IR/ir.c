#include "ir.h"
#include "delay.h"
#include "sys.h"

/*
 * 红外发送  PA1  TIM2_CH2  38kHz PWM 载波
 *   TIM2: 72MHz / (PSC+1) / (ARR+1) = 38kHz
 *         PSC=0, ARR=1893 → 72000000/1894 ≈ 38016 Hz
 *         占空比 50%: CCR = 947
 *   调制方式: 开关 TIM2 CH2 输出使能来发送"有载波/无载波"
 *
 * 红外接收  PB8  EXTI8（EXTI9_5_IRQHandler）
 *   接收模块输出低有效（有38kHz信号时输出低电平）
 *   → 上升沿=结束，下降沿=开始
 *   解码: NEC 协议
 *     引导码: 9ms低 + 4.5ms高
 *     数据0:  560us低 + 560us高
 *     数据1:  560us低 + 1690us高
 *
 * 注: 使用 g_tick_ms 做时间基准，需先调用 App_TickInit()。
 */

/* ---- NEC 发送 ---- */

static void ir_mark(uint32_t us)    /* 发射载波 */
{
    TIM_CCxCmd(TIM2, TIM_Channel_2, TIM_CCx_Enable);
    delay_us(us);
    TIM_CCxCmd(TIM2, TIM_Channel_2, TIM_CCx_Disable);
}

static void ir_space(uint32_t us)   /* 无载波 */
{
    TIM_CCxCmd(TIM2, TIM_Channel_2, TIM_CCx_Disable);
    delay_us(us);
}

void IR_TX_Enable(void)
{
    TIM_CCxCmd(TIM2, TIM_Channel_2, TIM_CCx_Enable);
}

void IR_TX_Disable(void)
{
    TIM_CCxCmd(TIM2, TIM_Channel_2, TIM_CCx_Disable);
}

void IR_SendNEC(uint8_t addr, uint8_t cmd)
{
    uint8_t  data[4] = { addr, (uint8_t)~addr, cmd, (uint8_t)~cmd };
    uint8_t  i, j;

    /* 引导码 */
    ir_mark(9000);
    ir_space(4500);

    /* 32位数据 */
    for (j = 0; j < 4; j++) {
        for (i = 0; i < 8; i++) {
            ir_mark(560);
            if (data[j] & (1u << i))
                ir_space(1690);
            else
                ir_space(560);
        }
    }

    /* 停止位 */
    ir_mark(560);
    ir_space(560);
}

/* ---- NEC 接收 ---- */

static volatile uint8_t  rx_addr    = 0;
static volatile uint8_t  rx_cmd     = 0;
static volatile uint8_t  rx_ready   = 0;

/* 内部状态机 */
#define RX_IDLE     0
#define RX_LEAD_L   1
#define RX_LEAD_H   2
#define RX_DATA     3

static uint8_t  rx_state    = RX_IDLE;
static uint32_t rx_tick     = 0;
static uint32_t rx_bits     = 0;
static uint8_t  rx_bit_cnt  = 0;

void IR_RX_IRQHandler(void)
{
    uint32_t now = g_tick_ms;
    uint32_t dt;
    uint8_t  level = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8); /* 中断后当前电平 */

    dt = now - rx_tick;
    rx_tick = now;

    switch (rx_state) {
    case RX_IDLE:
        if (level == 0) {              /* 下降沿：开始引导低 */
            rx_state = RX_LEAD_L;
        }
        break;

    case RX_LEAD_L:
        if (level == 1) {              /* 上升沿：引导低结束 */
            if (dt >= 8 && dt <= 10)   /* 约9ms */
                rx_state = RX_LEAD_H;
            else
                rx_state = RX_IDLE;
        }
        break;

    case RX_LEAD_H:
        if (level == 0) {              /* 下降沿：引导高结束 */
            if (dt >= 4 && dt <= 5) {  /* 约4.5ms */
                rx_state   = RX_DATA;
                rx_bits    = 0;
                rx_bit_cnt = 0;
            } else {
                rx_state = RX_IDLE;
            }
        }
        break;

    case RX_DATA:
        if (level == 1) {              /* 上升沿：低电平脉冲结束（均约560us，忽略） */
            /* do nothing */
        } else {                       /* 下降沿：高电平间隔结束，判断0/1 */
            if (rx_bit_cnt < 32) {
                rx_bits >>= 1;
                if (dt >= 1 && dt <= 1)       /* 560us ≈ 1ms → 逻辑0 */
                    rx_bits &= ~(1u << 31);
                else if (dt >= 2)              /* 1690us ≈ 2ms → 逻辑1 */
                    rx_bits |= (1u << 31);
                rx_bit_cnt++;
            }
            if (rx_bit_cnt == 32) {
                uint8_t a  = (uint8_t)(rx_bits);
                uint8_t na = (uint8_t)(rx_bits >> 8);
                uint8_t c  = (uint8_t)(rx_bits >> 16);
                uint8_t nc = (uint8_t)(rx_bits >> 24);
                if ((a ^ na) == 0xFF && (c ^ nc) == 0xFF) {
                    rx_addr  = a;
                    rx_cmd   = c;
                    rx_ready = 1;
                }
                rx_state = RX_IDLE;
            }
        }
        break;
    }
}

uint8_t IR_RX_HasData(void)
{
    if (rx_ready) { rx_ready = 0; return 1; }
    return 0;
}

uint8_t IR_RX_GetAddr(void) { return rx_addr; }
uint8_t IR_RX_GetCmd(void)  { return rx_cmd;  }

/* ---- 初始化 ---- */

void IR_Init(void)
{
    GPIO_InitTypeDef        gi;
    TIM_TimeBaseInitTypeDef tb;
    TIM_OCInitTypeDef       oc;
    EXTI_InitTypeDef        ei;
    NVIC_InitTypeDef        ni;

    /* --- TX: PA1 复用推挽 TIM2_CH2 --- */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,  ENABLE);

    gi.GPIO_Pin   = GPIO_Pin_1;
    gi.GPIO_Mode  = GPIO_Mode_AF_PP;
    gi.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gi);

    /* TIM2 38kHz: PSC=0, ARR=1893 → 72MHz/1894≈38kHz, CCR=947(50%) */
    tb.TIM_Period        = 1893;
    tb.TIM_Prescaler     = 0;
    tb.TIM_ClockDivision = TIM_CKD_DIV1;
    tb.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &tb);

    oc.TIM_OCMode      = TIM_OCMode_PWM1;
    oc.TIM_OCPolarity  = TIM_OCPolarity_High;
    oc.TIM_OutputState = TIM_OutputState_Enable;
    oc.TIM_Pulse       = 947;
    TIM_OC2Init(TIM2, &oc);
    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM2, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
    TIM_CCxCmd(TIM2, TIM_Channel_2, TIM_CCx_Disable); /* 默认关闭载波 */

    /* --- RX: PB8 上拉输入 + EXTI8 双边沿 --- */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    gi.GPIO_Pin  = GPIO_Pin_8;
    gi.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &gi);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource8);

    ei.EXTI_Line    = EXTI_Line8;
    ei.EXTI_Mode    = EXTI_Mode_Interrupt;
    ei.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    ei.EXTI_LineCmd = ENABLE;
    EXTI_Init(&ei);

    ni.NVIC_IRQChannel                   = EXTI9_5_IRQn;
    ni.NVIC_IRQChannelPreemptionPriority = 2;
    ni.NVIC_IRQChannelSubPriority        = 0;
    ni.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&ni);
}
