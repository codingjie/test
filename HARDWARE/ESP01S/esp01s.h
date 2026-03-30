#ifndef __ESP01S_H
#define __ESP01S_H

#include "stm32f10x.h"
#include <stdint.h>

// ESP01S WiFi模块，使用 USART1
// PA9  (USART1_TX) -> ESP01S RX
// PA10 (USART1_RX) -> ESP01S TX
// 波特率：115200

void ESP01S_Init(void);
void ESP01S_SendByte(uint8_t byte);
void ESP01S_SendString(const char *str);

#endif
