#ifndef __BEEP_H
#define __BEEP_H

#include "stm32f10x.h"

// 蜂鸣器接 PA5，高电平有效（三极管驱动），1Hz 振动由主循环控制
#define BEEP_GPIO_PORT  GPIOA
#define BEEP_GPIO_CLK   RCC_APB2Periph_GPIOA
#define BEEP_PIN        GPIO_Pin_5

#define BEEP_ON()   GPIO_SetBits(BEEP_GPIO_PORT, BEEP_PIN)
#define BEEP_OFF()  GPIO_ResetBits(BEEP_GPIO_PORT, BEEP_PIN)

void BEEP_GPIO_Config(void);

#endif
