#ifndef __KEY_H
#define __KEY_H

#include "stm32f4xx.h"

// 引脚定义保持不变
#define KEY1_PIN                  GPIO_Pin_0                  
#define KEY1_GPIO_PORT            GPIOA                       
#define KEY1_GPIO_CLK             RCC_AHB1Periph_GPIOA

#define KEY2_PIN                  GPIO_Pin_13                 
#define KEY2_GPIO_PORT            GPIOC                       
#define KEY2_GPIO_CLK             RCC_AHB1Periph_GPIOC

void Key_EXTI_Config(void);

#endif
