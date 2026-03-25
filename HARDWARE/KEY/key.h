#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"

/* 引脚：PB10=KEY1, PB11=KEY2，下拉输入，高电平=按下 */
#define KEY_PORT   GPIOB
#define KEY1_PIN   GPIO_Pin_10
#define KEY2_PIN   GPIO_Pin_11
#define KEY_CLK    RCC_APB2Periph_GPIOB

#define KEY1_PRESSED  (GPIO_ReadInputDataBit(KEY_PORT, KEY1_PIN) != 0)
#define KEY2_PRESSED  (GPIO_ReadInputDataBit(KEY_PORT, KEY2_PIN) != 0)

void KEY_Init(void);

#endif /* __KEY_H */
