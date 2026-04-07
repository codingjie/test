#include "fan.h"

// 初始化 TIM3_CH1 PWM 输出（PA6），25kHz，初始关闭
void FAN_Init(void) {
    GPIO_InitTypeDef       GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCInitStructure;

    // 开启时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,  ENABLE);

    // PA6 复用推挽输出（TIM3_CH1）
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // TIM3 时基：72MHz / (0+1) / (2879+1) = 25kHz
    TIM_TimeBaseStructure.TIM_Period        = FAN_TIM_PERIOD;
    TIM_TimeBaseStructure.TIM_Prescaler     = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    // PWM 模式 1，初始占空比 0（关闭）
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse       = FAN_DUTY_OFF;
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

// 设置风扇档位
void FAN_SetSpeed(FanSpeed_t speed) {
    uint16_t duty;

    switch (speed) {
        case FAN_LOW:  duty = FAN_DUTY_LOW;  break;
        case FAN_HIGH: duty = FAN_DUTY_HIGH; break;
        default:       duty = FAN_DUTY_OFF;  break;
    }

    // 更新 CCR1 比较值改变占空比
    TIM_SetCompare1(TIM3, duty);
}
