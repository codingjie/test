#ifndef __KEY_H
#define __KEY_H	 

#include "stm32f10x.h"
#include "stm32f10x_exti.h"

#define KEY1_GPIO_PIN GPIO_Pin_2
#define KEY2_GPIO_PIN GPIO_Pin_3
#define KEY3_GPIO_PIN GPIO_Pin_4
#define KEY4_GPIO_PIN GPIO_Pin_5
#define KEY_GPIO_PORT GPIOB
#define KEY_GPIO_CLK RCC_APB2Periph_GPIOB

void KEY_GPIO_Config(void);
void KEY_EXTI_Init(void);

#endif
