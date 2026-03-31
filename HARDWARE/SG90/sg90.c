#include "sg90.h"

void SG90_Init(void) {
    GPIO_InitTypeDef        gi;
    TIM_TimeBaseInitTypeDef tb;
    TIM_OCInitTypeDef       oc;

    /* 时钟 */
    RCC_APB2PeriphClockCmd(SG90_GPIO_CLK_A | SG90_GPIO_CLK_B, ENABLE);
    RCC_APB1PeriphClockCmd(SG90_TIM_CLK, ENABLE);

    /* PA6 PA7 复用推挽 */
    gi.GPIO_Mode  = GPIO_Mode_AF_PP;
    gi.GPIO_Speed = GPIO_Speed_50MHz;
    gi.GPIO_Pin   = SG90_1_PIN | SG90_2_PIN;
    GPIO_Init(GPIOA, &gi);

    /* PB0 PB1 复用推挽 */
    gi.GPIO_Pin = SG90_3_PIN | SG90_4_PIN;
    GPIO_Init(GPIOB, &gi);

    /* TIM3 时基 */
    tb.TIM_Period        = SG90_TIM_PERIOD;
    tb.TIM_Prescaler     = SG90_TIM_PRESCALER;
    tb.TIM_ClockDivision = TIM_CKD_DIV1;
    tb.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &tb);

    /* 四路 PWM，初始脉宽 1500us（90°） */
    oc.TIM_OCMode      = TIM_OCMode_PWM1;
    oc.TIM_OutputState = TIM_OutputState_Enable;
    oc.TIM_Pulse       = 1500;
    oc.TIM_OCPolarity  = TIM_OCPolarity_High;

    TIM_OC1Init(TIM3, &oc); TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_OC2Init(TIM3, &oc); TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_OC3Init(TIM3, &oc); TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_OC4Init(TIM3, &oc); TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

void SG90_SetPulse(uint8_t ch, uint16_t pulse_us)
{
    if (pulse_us < SG90_PULSE_MIN) pulse_us = SG90_PULSE_MIN;
    if (pulse_us > SG90_PULSE_MAX) pulse_us = SG90_PULSE_MAX;

    switch (ch) {
        case 1: TIM_SetCompare1(TIM3, pulse_us); break;
        case 2: TIM_SetCompare2(TIM3, pulse_us); break;
        case 3: TIM_SetCompare3(TIM3, pulse_us); break;
        case 4: TIM_SetCompare4(TIM3, pulse_us); break;
        default: break;
    }
}

void SG90_SetAngle(uint8_t ch, uint8_t angle)
{
    uint16_t pulse;
    if (angle > SG90_ANGLE_MAX) angle = SG90_ANGLE_MAX;
    /* 线性映射: 0° → 500us, 180° → 2500us */
    pulse = SG90_PULSE_MIN + (uint16_t)((uint32_t)angle * 2000 / 180);
    SG90_SetPulse(ch, pulse);
}
