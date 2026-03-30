#ifndef __ULTRA_H
#define __ULTRA_H

#include "stm32f10x.h"
#include <stdint.h>

// 4路HC-SR04超声波传感器，TRIG共用PA8
// ECHO1: PB6  -> 可回收垃圾桶
// ECHO2: PB7  -> 有害垃圾桶
// ECHO3: PB8  -> 厨余垃圾桶
// ECHO4: PB9  -> 其他垃圾桶
//
// 依次触发各路传感器（每次触发一路）
// 距离 = echo高电平时间(us) / 58（厘米）

#define ULTRA_FULL_CM   10   // 距离小于10cm视为垃圾桶已满

void     ULTRA_Init(void);
uint16_t ULTRA_GetDistance_cm(uint8_t ch);  // ch: 1~4；超时返回0xFFFF
uint8_t  ULTRA_IsFull(uint8_t ch);          // 1=已满, 0=未满

#endif
