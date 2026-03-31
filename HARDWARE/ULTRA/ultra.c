#include "ultra.h"
#include "delay.h"

#define TRIG_PORT   GPIOA
#define TRIG_PIN    GPIO_Pin_8

/* TIM4 @ 1MHz (72MHz/72 = 1us/tick, 16-bit → max 65535us ≈ 65ms) */

static const uint16_t tim_channel[4] = {
    TIM_Channel_1,   /* PB6  TIM4_CH1 → ECHO1 可回收 */
    TIM_Channel_2,   /* PB7  TIM4_CH2 → ECHO2 有害   */
    TIM_Channel_3,   /* PB8  TIM4_CH3 → ECHO3 厨余   */
    TIM_Channel_4    /* PB9  TIM4_CH4 → ECHO4 其他   */
};

static const uint16_t cap_flag[4] = {
    TIM_FLAG_CC1, TIM_FLAG_CC2, TIM_FLAG_CC3, TIM_FLAG_CC4
};

static uint16_t GetCapture(uint8_t ch)
{
    switch (ch) {
        case 1:  return TIM_GetCapture1(TIM4);
        case 2:  return TIM_GetCapture2(TIM4);
        case 3:  return TIM_GetCapture3(TIM4);
        default: return TIM_GetCapture4(TIM4);
    }
}

void ULTRA_Init(void)
{
    GPIO_InitTypeDef        gi;
    TIM_TimeBaseInitTypeDef tb;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    /* TRIG：推挽输出，初始低 */
    gi.GPIO_Pin   = TRIG_PIN;
    gi.GPIO_Mode  = GPIO_Mode_Out_PP;
    gi.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TRIG_PORT, &gi);
    GPIO_ResetBits(TRIG_PORT, TRIG_PIN);

    /* ECHO1~4：浮空输入（TIM4 输入捕获不需要复用推挽）*/
    gi.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gi.GPIO_Pin  = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_Init(GPIOB, &gi);

    /* TIM4 时基：72MHz / 72 = 1MHz → 1us/tick */
    tb.TIM_Period        = 0xFFFF;
    tb.TIM_Prescaler     = 71;
    tb.TIM_ClockDivision = TIM_CKD_DIV1;
    tb.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &tb);
    TIM_Cmd(TIM4, ENABLE);
}

/* 返回距离（毫米），超时或异常返回 0xFFFF
 * 原理：TIM4 输入捕获，硬件记录 ECHO 上升/下降沿 tick 值
 *       duration(us) = t_fall - t_rise（uint16 减法自动处理溢出）
 *       distance(mm) = duration * 10 / 58
 */
uint16_t ULTRA_GetDistance_mm(uint8_t ch)
{
    TIM_ICInitTypeDef ic;
    uint16_t t_rise, t_fall, t_start, duration;
    uint16_t channel, flag;

    if (ch < 1 || ch > 4) return 0xFFFF;
    channel = tim_channel[ch - 1];
    flag    = cap_flag[ch - 1];

    ic.TIM_Channel     = channel;
    ic.TIM_ICSelection = TIM_ICSelection_DirectTI;
    ic.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    ic.TIM_ICFilter    = 0x00;   /* 无滤波，ECHO 信号干净 */

    /* --- 第一步：捕获上升沿 --- */
    ic.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInit(TIM4, &ic);
    TIM_ClearFlag(TIM4, flag);

    /* 发送 10us TRIG 脉冲 */
    GPIO_SetBits(TRIG_PORT, TRIG_PIN);
    delay_us(10);
    GPIO_ResetBits(TRIG_PORT, TRIG_PIN);

    /* 等待上升沿（超时 30ms）*/
    t_start = TIM4->CNT;
    while (!TIM_GetFlagStatus(TIM4, flag)) {
        if ((uint16_t)(TIM4->CNT - t_start) >= 30000) return 0xFFFF;
    }
    t_rise = GetCapture(ch);
    TIM_ClearFlag(TIM4, flag);

    /* --- 第二步：捕获下降沿 --- */
    ic.TIM_ICPolarity = TIM_ICPolarity_Falling;
    TIM_ICInit(TIM4, &ic);

    t_start = TIM4->CNT;
    while (!TIM_GetFlagStatus(TIM4, flag)) {
        if ((uint16_t)(TIM4->CNT - t_start) >= 30000) return 0xFFFF;
    }
    t_fall = GetCapture(ch);

    /* uint16 减法自动处理 TIM4 计数器 0xFFFF→0 溢出 */
    duration = t_fall - t_rise;

    /* distance_mm = duration(us) × 10 / 58 */
    return (uint16_t)((uint32_t)duration * 10 / 58);
}

uint16_t ULTRA_GetDistance_cm(uint8_t ch)
{
    uint16_t d = ULTRA_GetDistance_mm(ch);
    if (d == 0xFFFF) return 0xFFFF;
    return d / 10;
}

uint8_t ULTRA_IsFull(uint8_t ch)
{
    uint16_t d = ULTRA_GetDistance_mm(ch);
    if (d == 0xFFFF) return 0;
    return (d < ULTRA_FULL_MM) ? 1 : 0;
}
