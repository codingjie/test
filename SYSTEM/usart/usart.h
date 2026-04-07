#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"
#include <stdint.h>

// 调试串口 USART1
// TX: PA9   RX: PA10
// 波特率: 115200, 8N1
//
// 初始化后重定向 printf 到 USART1。

void USART1_Init(uint32_t baudrate);
void USART1_SendByte(uint8_t byte);
void USART1_SendString(const char *str);

#endif
