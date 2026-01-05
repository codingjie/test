#ifndef __LED_H
#define __LED_H	 

#include  "stm32f10x.h"
#include "stm32f10x_gpio.h"

// 引脚定义
#define SNGREEN_PIN    GPIO_Pin_0
#define SNRED_PIN    GPIO_Pin_1
#define SNYELLOW_PIN    GPIO_Pin_2
#define EWGREEN_PIN    GPIO_Pin_8
#define EWRED_PIN    GPIO_Pin_9
#define EWYELLOW_PIN    GPIO_Pin_10

// 南北方向 (SN)
#define SNGREEN_ON    GPIO_ResetBits(GPIOA, SNGREEN_PIN)
#define SNGREEN_OFF   GPIO_SetBits(GPIOA, SNGREEN_PIN)
#define SNRED_ON      GPIO_ResetBits(GPIOA, SNRED_PIN)
#define SNRED_OFF     GPIO_SetBits(GPIOA, SNRED_PIN)
#define SNYELLOW_ON   GPIO_ResetBits(GPIOA, SNYELLOW_PIN)
#define SNYELLOW_OFF  GPIO_SetBits(GPIOA, SNYELLOW_PIN)

// 东西方向 (EW)
#define EWGREEN_ON    GPIO_ResetBits(GPIOA, EWGREEN_PIN)
#define EWGREEN_OFF   GPIO_SetBits(GPIOA, EWGREEN_PIN)
#define EWRED_ON      GPIO_ResetBits(GPIOA, EWRED_PIN)
#define EWRED_OFF     GPIO_SetBits(GPIOA, EWRED_PIN)
#define EWYELLOW_ON   GPIO_ResetBits(GPIOA, EWYELLOW_PIN)
#define EWYELLOW_OFF  GPIO_SetBits(GPIOA, EWYELLOW_PIN)

void LED_Init(void);

#endif
