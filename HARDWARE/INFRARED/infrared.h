#ifndef __INFRARED_H
#define __INFRARED_H

#include "stm32f10x.h"
#include <stdint.h>

/* 4 IR obstacle sensors, active LOW on detection
 * IR1: PA4  -> Recyclable bin
 * IR2: PA5  -> Hazardous bin
 * IR3: PA11 -> Kitchen waste bin
 * IR4: PA12 -> Other waste bin
 */

void    IR_Init(void);
uint8_t IR_Detected(uint8_t ch);   /* ch: 1~4; returns 1 if person detected */

#endif
