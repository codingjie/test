#include "sg90.h"

static uint16_t angle_to_pulse(uint8_t angle)
{
    if (angle > 180) angle = 180;
    // 线性映射：0° -> 500us，180° -> 2500us
    return 500 + (uint16_t)((uint32_t)angle * 2000 / 180);
}

// 初始化TIM3 CH1-CH4 PWM，驱动4路舵机
void SG90_Init(void)
{
    GPIO_InitTypeDef        GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB
                           | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // PA6（CH1）、PA7（CH2）- 复用推挽输出
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // PB0（CH3）、PB1（CH4）- 复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // TIM3时基：1MHz节拍，20ms周期（50Hz）
    TIM_TimeBaseStructure.TIM_Period        = SG90_TIM_PERIOD;
    TIM_TimeBaseStructure.TIM_Prescaler     = SG90_TIM_PRESCALER;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    // PWM模式1，4路通道初始位置均为关盖角度
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse       = angle_to_pulse(SERVO_CLOSE_ANGLE);
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;

    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);

    TIM_OC2Init(TIM3, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);

    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);

    TIM_OC4Init(TIM3, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

void SG90_SetAngle(uint8_t servo, uint8_t angle)
{
    uint16_t pulse = angle_to_pulse(angle);
    switch (servo) {
        case 1: TIM_SetCompare1(TIM3, pulse); break;
        case 2: TIM_SetCompare2(TIM3, pulse); break;
        case 3: TIM_SetCompare3(TIM3, pulse); break;
        case 4: TIM_SetCompare4(TIM3, pulse); break;
        default: break;
    }
}

void SG90_Open(uint8_t servo)
{
    SG90_SetAngle(servo, SERVO_OPEN_ANGLE);
}

void SG90_Close(uint8_t servo)
{
    SG90_SetAngle(servo, SERVO_CLOSE_ANGLE);
}
