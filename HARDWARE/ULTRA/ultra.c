#include "ultra.h"
#include "delay.h"

#define TRIG_PORT   GPIOA
#define TRIG_PIN    GPIO_Pin_8

/* TIM4 @ 1MHz (72MHz/72 = 1us/tick, 16-bit counter) */

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

    /* ECHO1~4：浮空输入（TIM4 输入捕获）*/
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

/*
 * ULTRA_MeasureAll - 发一次 TRIG，TIM4 四通道同时捕获所有 ECHO
 *
 * 原理：TRIG 共用 → 一次脉冲同时触发 4 个传感器 → 各自 ECHO 返回时刻
 *       由 TIM4_CH1~CH4 硬件独立捕获上升/下降沿，1us 分辨率
 *
 * dist_mm[0~3]：各通道距离(mm)；无回波或超时时置 0xFFFF
 */
void ULTRA_MeasureAll(uint16_t dist_mm[4])
{
    TIM_ICInitTypeDef ic;
    uint16_t t_rise[4], t_fall[4];
    uint8_t  got_rise, got_fall;
    uint16_t t_start;
    uint8_t  i;

    /* 默认全部超时 */
    for (i = 0; i < 4; i++) dist_mm[i] = 0xFFFF;

    /* --- 第一步：4个通道全部配置为上升沿捕获 --- */
    ic.TIM_ICSelection = TIM_ICSelection_DirectTI;
    ic.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    ic.TIM_ICFilter    = 0x00;
    ic.TIM_ICPolarity  = TIM_ICPolarity_Rising;
    for (i = 0; i < 4; i++) {
        ic.TIM_Channel = tim_channel[i];
        TIM_ICInit(TIM4, &ic);
        TIM_ClearFlag(TIM4, cap_flag[i]);
    }

    /* --- 第二步：发一次 10us TRIG，同时触发 4 个传感器 --- */
    GPIO_SetBits(TRIG_PORT, TRIG_PIN);
    delay_us(10);
    GPIO_ResetBits(TRIG_PORT, TRIG_PIN);

    /* --- 第三步：等待 4 路上升沿（超时 30ms）--- */
    got_rise = 0;
    t_start  = TIM4->CNT;
    while (got_rise != 0x0F) {
        for (i = 0; i < 4; i++) {
            if (!(got_rise & (1 << i)) && TIM_GetFlagStatus(TIM4, cap_flag[i])) {
                t_rise[i] = GetCapture(i + 1);
                TIM_ClearFlag(TIM4, cap_flag[i]);
                got_rise |= (uint8_t)(1 << i);
            }
        }
        if ((uint16_t)(TIM4->CNT - t_start) >= 30000) break;
    }

    if (got_rise == 0) return;   /* 没有任何传感器响应 */

    /* --- 第四步：已捕获上升沿的通道切换为下降沿捕获 --- */
    ic.TIM_ICPolarity = TIM_ICPolarity_Falling;
    for (i = 0; i < 4; i++) {
        if (got_rise & (1 << i)) {
            ic.TIM_Channel = tim_channel[i];
            TIM_ICInit(TIM4, &ic);
            TIM_ClearFlag(TIM4, cap_flag[i]);
        }
    }

    /* --- 第五步：等待已响应通道的下降沿（超时 30ms）--- */
    got_fall = 0;
    t_start  = TIM4->CNT;
    while (got_fall != got_rise) {
        for (i = 0; i < 4; i++) {
            if ((got_rise & (1 << i)) && !(got_fall & (1 << i)) &&
                TIM_GetFlagStatus(TIM4, cap_flag[i])) {
                t_fall[i] = GetCapture(i + 1);
                TIM_ClearFlag(TIM4, cap_flag[i]);
                got_fall |= (uint8_t)(1 << i);
            }
        }
        if ((uint16_t)(TIM4->CNT - t_start) >= 30000) break;
    }

    /* --- 第六步：计算距离 --- */
    for (i = 0; i < 4; i++) {
        if ((got_rise & (1 << i)) && (got_fall & (1 << i))) {
            /* uint16 减法自动处理计数器 0xFFFF→0 溢出 */
            uint16_t duration = t_fall[i] - t_rise[i];
            /* distance_mm = duration(us) * 10 / 58 */
            dist_mm[i] = (uint16_t)((uint32_t)duration * 10 / 58);
        }
    }
}

uint16_t ULTRA_GetDistance_mm(uint8_t ch)
{
    uint16_t d[4];
    if (ch < 1 || ch > 4) return 0xFFFF;
    ULTRA_MeasureAll(d);
    return d[ch - 1];
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
