#include "bsp_led.h"

void LED_PWM_Config(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    // 1. 开启时钟
    RCC_AHB1PeriphClockCmd(LED_GPIO_CLK, ENABLE);
    RCC_APB1PeriphClockCmd(LED_TIM_CLK, ENABLE);

    // 2. GPIO 复用配置
    GPIO_PinAFConfig(LED_GPIO_PORT, LED1_PINSOURCE, LED_AF);
    GPIO_PinAFConfig(LED_GPIO_PORT, LED2_PINSOURCE, LED_AF);
    GPIO_PinAFConfig(LED_GPIO_PORT, LED3_PINSOURCE, LED_AF);

    GPIO_InitStructure.GPIO_Pin = LED1_PIN | LED2_PIN | LED3_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        // 复用模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(LED_GPIO_PORT, &GPIO_InitStructure);

    // 3. 定时器基础配置
    TIM_TimeBaseStructure.TIM_Period = PWM_PERIOD;
    TIM_TimeBaseStructure.TIM_Prescaler = PWM_PRESCALER;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(LED_TIM, &TIM_TimeBaseStructure);

    // 4. PWM 模式配置 (通道1, 2, 3)
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;                  // 初始亮度为0
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; // 低电平有效(点亮)

    TIM_OC1Init(LED_TIM, &TIM_OCInitStructure);
    TIM_OC2Init(LED_TIM, &TIM_OCInitStructure);
    TIM_OC3Init(LED_TIM, &TIM_OCInitStructure);

    // 使能预装载
    TIM_OC1PreloadConfig(LED_TIM, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(LED_TIM, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(LED_TIM, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(LED_TIM, ENABLE);

    // 5. 开启定时器
    TIM_Cmd(LED_TIM, ENABLE);
}

/**
 * @brief 设置RGB颜色亮度 (范围: 0-99)
 */
void LED_SetRGB(uint16_t r, uint16_t g, uint16_t b) {
    TIM_SetCompare1(LED_TIM, r); // R
    TIM_SetCompare2(LED_TIM, g); // G
    TIM_SetCompare3(LED_TIM, b); // B
}
