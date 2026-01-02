#ifndef __LED_PWM_H
#define __LED_PWM_H

#include "stm32f4xx.h"

// 引脚及定时器定义 (PH10, PH11, PH12 对应 TIM5 CH1, CH2, CH3)
#define LED_TIM            TIM5
#define LED_TIM_CLK        RCC_APB1Periph_TIM5
#define LED_GPIO_CLK       RCC_AHB1Periph_GPIOH
#define LED_GPIO_PORT      GPIOH

#define LED1_PIN           GPIO_Pin_10
#define LED1_PINSOURCE     GPIO_PinSource10
#define LED2_PIN           GPIO_Pin_11
#define LED2_PINSOURCE     GPIO_PinSource11
#define LED3_PIN           GPIO_Pin_12
#define LED3_PINSOURCE     GPIO_PinSource12

#define LED_AF             GPIO_AF_TIM5

// PWM 周期与频率：频率 = TIM_CLK / (PSC+1) / (ARR+1)
#define PWM_PERIOD         (100 - 1)
#define PWM_PRESCALER      (1800 - 1)

// 函数声明
void LED_PWM_Config(void);
void LED_SetRGB(uint16_t r, uint16_t g, uint16_t b);

#endif /* __LED_PWM_H */
