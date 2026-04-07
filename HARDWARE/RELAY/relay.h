#ifndef __RELAY_H
#define __RELAY_H

#include "stm32f10x.h"

// 继电器1: PC4  工作模式3s后触发（高电平有效）
// 继电器2: PC5  备用              （高电平有效）

#define RELAY1_ON()    GPIO_SetBits(GPIOC,   GPIO_Pin_4)
#define RELAY1_OFF()   GPIO_ResetBits(GPIOC, GPIO_Pin_4)

#define RELAY2_ON()    GPIO_SetBits(GPIOC,   GPIO_Pin_5)
#define RELAY2_OFF()   GPIO_ResetBits(GPIOC, GPIO_Pin_5)

void RELAY_Init(void);

#endif
