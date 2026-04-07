#ifndef __DELAY_H
#define __DELAY_H 		

#include "sys.h"
#include "stm32f10x.h"
#include "stm32f10x_tim.h"

void delay_init(void);
void delay_us(uint32_t nus);
void delay_ms(uint16_t nms);

#endif





























