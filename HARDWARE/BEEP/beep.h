#ifndef __BEEP_H
#define __BEEP_H

#include "stm32f10x.h"

/* Buzzer on PB12, active LOW (transistor driver inverts signal) */
#define BEEP_GPIO_PORT  GPIOB
#define BEEP_GPIO_CLK   RCC_APB2Periph_GPIOB
#define BEEP_PIN        GPIO_Pin_12

#define BEEP_ON()   GPIO_ResetBits(BEEP_GPIO_PORT, BEEP_PIN)
#define BEEP_OFF()  GPIO_SetBits(BEEP_GPIO_PORT, BEEP_PIN)

void BEEP_GPIO_Config(void);

#endif
