#ifndef __INFRARED_H
#define __INFRARED_H

#include "stm32f10x.h"
#include <stdint.h>

// 4路红外障碍物传感器，检测到障碍物时输出低电平
// IR1: PA4  -> 可回收垃圾桶
// IR2: PA5  -> 有害垃圾桶
// IR3: PA11 -> 厨余垃圾桶
// IR4: PA12 -> 其他垃圾桶

void    IR_Init(void);
uint8_t IR_Detected(uint8_t ch);   // ch: 1~4；检测到人返回1

#endif
