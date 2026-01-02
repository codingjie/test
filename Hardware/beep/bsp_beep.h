#ifndef __BEEP_H
#define __BEEP_H

#include "stm32f4xx.h"

/* 定义蜂鸣器连接的GPIO端口 */
#define BEEP_GPIO_PORT          GPIOB
#define BEEP_GPIO_CLK           RCC_AHB1Periph_GPIOB    // F4的GPIO在AHB1总线上
#define BEEP_GPIO_PIN           GPIO_Pin_9

/* 高电平时，蜂鸣器响 */
#define BEEP(a) if (a) \
                    GPIO_SetBits(BEEP_GPIO_PORT, BEEP_GPIO_PIN); \
                else \
                    GPIO_ResetBits(BEEP_GPIO_PORT, BEEP_GPIO_PIN)
                    
/* 定义控制IO的宏 */
#define BEEP_ON     GPIO_SetBits(BEEP_GPIO_PORT, BEEP_GPIO_PIN)
#define BEEP_OFF    GPIO_ResetBits(BEEP_GPIO_PORT, BEEP_GPIO_PIN)
#define BEEP_TOGGLE GPIO_ToggleBits(BEEP_GPIO_PORT, BEEP_GPIO_PIN)

void BEEP_GPIO_Config(void);
					
#endif
