#include "sg90.h"

void SG90_Init(void)
{
    GPIO_InitTypeDef        gi;
    TIM_TimeBaseInitTypeDef tb;
    TIM_OCInitTypeDef       oc;

    RCC_APB2PeriphClockCmd(SG90_GPIO_CLK, ENABLE);
    RCC_APB1PeriphClockCmd(SG90_TIM_CLK,  ENABLE);

    /* PA6 复用推挽（TIM3_CH1 PWM） */
    gi.GPIO_Pin   = SG90_PIN;
    gi.GPIO_Mode  = GPIO_Mode_AF_PP;
    gi.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SG90_PORT, &gi);

    /* TIM3 时基: 50Hz */
    tb.TIM_Period        = SG90_TIM_PERIOD;
    tb.TIM_Prescaler     = SG90_TIM_PRESCALER;
    tb.TIM_ClockDivision = TIM_CKD_DIV1;
    tb.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &tb);

    /* CH1 PWM 初始关盖 90° */
    oc.TIM_OCMode      = TIM_OCMode_PWM1;
    oc.TIM_OCPolarity  = TIM_OCPolarity_High;
    oc.TIM_OutputState = TIM_OutputState_Enable;
    oc.TIM_Pulse       = SG90_PULSE_MIN + (uint16_t)((uint32_t)SG90_ANGLE_CLOSE * 2000 / 180);
    TIM_OC1Init(TIM3, &oc);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

void SG90_SetPulse(uint16_t pulse_us)
{
    if (pulse_us < SG90_PULSE_MIN) pulse_us = SG90_PULSE_MIN;
    if (pulse_us > SG90_PULSE_MAX) pulse_us = SG90_PULSE_MAX;
    TIM_SetCompare1(TIM3, pulse_us);
}

void SG90_SetAngle(uint8_t angle)
{
    uint16_t pulse;
    if (angle > SG90_ANGLE_MAX) angle = SG90_ANGLE_MAX;
    pulse = SG90_PULSE_MIN + (uint16_t)((uint32_t)angle * 2000 / 180);
    SG90_SetPulse(pulse);
}

void SG90_Open(void)  { SG90_SetAngle(SG90_ANGLE_OPEN);  }
void SG90_Close(void) { SG90_SetAngle(SG90_ANGLE_CLOSE); }
