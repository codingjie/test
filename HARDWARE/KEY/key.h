#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"
#include "stm32f10x_exti.h"

/* 引脚定义 */
#define KEY_PORT        GPIOB
#define KEY1_PIN        GPIO_Pin_10
#define KEY2_PIN        GPIO_Pin_11
#define KEY_CLK         RCC_APB2Periph_GPIOB

/* 函数声明 */
void KEY_EXTI_Init(void);

#endif
