#ifndef __BEEP_H
#define __BEEP_H

#include "stm32f10x.h"

// 蜂鸣器接 PB12，低电平有效（三极管驱动信号取反）
#define BEEP_GPIO_PORT  GPIOB
#define BEEP_GPIO_CLK   RCC_APB2Periph_GPIOB
#define BEEP_PIN        GPIO_Pin_12

#define BEEP_ON()   GPIO_SetBits(BEEP_GPIO_PORT, BEEP_PIN)
#define BEEP_OFF()  GPIO_ResetBits(BEEP_GPIO_PORT, BEEP_PIN)

void BEEP_GPIO_Config(void);

#endif
