#ifndef __ULTRA_H
#define __ULTRA_H

#include "stm32f10x.h"
#include <stdint.h>

// 4路HC-SR04超声波传感器，TRIG共用PA8，ECHO接TIM4输入捕获
// ECHO1: PB6 (TIM4_CH1) -> 可回收垃圾桶
// ECHO2: PB7 (TIM4_CH2) -> 有害垃圾桶
// ECHO3: PB8 (TIM4_CH3) -> 厨余垃圾桶
// ECHO4: PB9 (TIM4_CH4) -> 其他垃圾桶
//
// 硬件输入捕获精确测量ECHO脉宽，1us分辨率
// 距离(mm) = echo脉宽(us) * 10 / 58

#define ULTRA_FULL_CM   2   // 距离小于视为垃圾桶已满（cm）
#define ULTRA_FULL_MM   20  // 距离小于视为垃圾桶已满（mm）

void     ULTRA_Init(void);
uint16_t ULTRA_GetDistance_cm(uint8_t ch);   // ch: 1~4；超时返回0xFFFF
uint16_t ULTRA_GetDistance_mm(uint8_t ch);   // ch: 1~4；超时返回0xFFFF
uint8_t  ULTRA_IsFull(uint8_t ch);           // 1=已满, 0=未满

#endif
