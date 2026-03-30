#include "sg90.h"

// 初始化 SG90 舵机 PWM (TIM3_CH1, PA6)
// PWM 频率 50Hz，计数单位 1us
void SG90_Init(void) {
    GPIO_InitTypeDef        GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(SG90_GPIO_CLK | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(SG90_TIM_CLK, ENABLE);

    /* PA6 复用推挽输出 */
    GPIO_InitStructure.GPIO_Pin   = SG90_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SG90_GPIO_PORT, &GPIO_InitStructure);

    /* TIM3 时基: 1MHz, 20ms 周期 */
    TIM_TimeBaseStructure.TIM_Period        = SG90_TIM_PERIOD;
    TIM_TimeBaseStructure.TIM_Prescaler     = SG90_TIM_PRESCALER;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(SG90_TIM, &TIM_TimeBaseStructure);

    /* PWM 模式1 通道1 */
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse       = 1611;   /* 默认 100°, 1611us */
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
    TIM_OC1Init(SG90_TIM, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(SG90_TIM, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(SG90_TIM, ENABLE);
    TIM_Cmd(SG90_TIM, ENABLE);
}

/**
 * @brief 设置舵机脉宽 (直接控制)
 * @param pulse_us  脉冲宽度，单位 us，范围 500 ~ 2500
 */
void SG90_SetPulse(uint16_t pulse_us)
{
    if (pulse_us < 500)  pulse_us = 500;
    if (pulse_us > 2500) pulse_us = 2500;
    TIM_SetCompare1(SG90_TIM, pulse_us);
}

/**
 * @brief 按角度控制舵机
 * @param angle  0 ~ 180 度
 */
void SG90_SetAngle(uint8_t angle)
{
    uint16_t pulse;
    if (angle > 180) angle = 180;
    /* 线性映射: 0° → 500us, 180° → 2500us */
    pulse = 500 + (uint16_t)((uint32_t)angle * 2000 / 180);
    SG90_SetPulse(pulse);
}
