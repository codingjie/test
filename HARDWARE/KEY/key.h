#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"

#define KEY1_PORT       GPIOA
#define KEY1_PIN        GPIO_Pin_5
#define KEY1_CLK        RCC_APB2Periph_GPIOA

#define KEY2_PORT       GPIOA
#define KEY2_PIN        GPIO_Pin_8
#define KEY2_CLK        RCC_APB2Periph_GPIOA

#define KEY3_PORT       GPIOA
#define KEY3_PIN        GPIO_Pin_15
#define KEY3_CLK        RCC_APB2Periph_GPIOA

#define KEY4_PORT       GPIOB
#define KEY4_PIN        GPIO_Pin_0
#define KEY4_CLK        RCC_APB2Periph_GPIOB

/* 读取按键状态，低电平（0）表示按下 */
#define KEY1_READ()     GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN)
#define KEY2_READ()     GPIO_ReadInputDataBit(KEY2_PORT, KEY2_PIN)
#define KEY3_READ()     GPIO_ReadInputDataBit(KEY3_PORT, KEY3_PIN)
#define KEY4_READ()     GPIO_ReadInputDataBit(KEY4_PORT, KEY4_PIN)

void    KEY_Init(void);
uint8_t KEY_Scan(void);

#endif
