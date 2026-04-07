#ifndef __JW01_H
#define __JW01_H

#include "stm32f10x.h"
#include "stm32f10x_usart.h"

// JW01 VOC 传感器，接 USART1 RX（PA10，FT 引脚，可承受 5V）
// 波特率 9600，8N1
// 数据帧格式（9 字节）：
//   [0]=0xFF  [1]=0x86  [2]=浓度高字节  [3]=浓度低字节
//   [4]~[7]=状态字节   [8]=校验和
// VOC 浓度（ppb）= [2]*256 + [3]

#define JW01_USART          USART1
#define JW01_USART_CLK      RCC_APB2Periph_USART1
#define JW01_GPIO_CLK       RCC_APB2Periph_GPIOA
#define JW01_RX_PIN         GPIO_Pin_10
#define JW01_RX_PORT        GPIOA
#define JW01_BAUD           9600

#define JW01_FRAME_LEN      9
#define JW01_HEADER1        0xFF
#define JW01_HEADER2        0x86

void     JW01_Init(void);                     // 初始化 USART1
void     JW01_IRQHandler(void);               // 在 USART1 中断中调用
uint8_t  JW01_GetData(uint16_t *voc_ppb);     // 获取最新 VOC 值，有新数据返回 1

#endif
