#ifndef __JY61P_H
#define __JY61P_H

#include "stm32f10x.h"
#include <stdint.h>

// JY61P 姿态传感器 (USART3)
// STM32 PB10 → USART3_TX → JY61P RX
// STM32 PB11 → USART3_RX → JY61P TX
// 波特率: 9600, 8N1
//
// 帧格式: 11字节  0x55 <type> D0L D0H D1L D1H D2L D2H D3L D3H SUM
//   type=0x51: 加速度  type=0x52: 角速度  type=0x53: 角度
//
// 使用方式:
//   JY61P_Init() 初始化；
//   USART3_IRQHandler 中调用 JY61P_IRQHandler()；
//   主循环中读取 jy61p_data 结构。

typedef struct {
    float ax, ay, az;       // 加速度 g
    float wx, wy, wz;       // 角速度 °/s
    float roll, pitch, yaw; // 角度 °
} JY61P_Data_t;

extern volatile JY61P_Data_t jy61p_data;

void    JY61P_Init(void);
void    JY61P_IRQHandler(void);   // 在 USART3_IRQHandler 中调用

#endif
