#ifndef __RGB_H
#define __RGB_H

#include "stm32f10x.h"
#include <stdint.h>

// 8路状态LED，每个垃圾桶2个（红=满仓，绿=未满）
// LED1  PA0   1号桶-可回收  红
// LED2  PA1   1号桶-可回收  绿
// LED3  PB13  2号桶-有害    红
// LED4  PB14  2号桶-有害    绿
// LED5  PB15  3号桶-厨余    红
// LED6  PB3   3号桶-厨余    绿（需禁用JTAG）
// LED7  PB4   4号桶-其他    红（需禁用JTAG）
// LED8  PB5   4号桶-其他    绿
//
// 注意：使用PB3/PB4前必须调用 GPIO_Remap_SWJ_JTAGDisable
//       KEY_Init() 已处理，LED_Init() 也会再次调用以确保安全

void LED_Init(void);
void LED_Set(uint8_t led, uint8_t on);              // led: 1~8, on: 1=亮 0=灭
void LED_SetBinStatus(uint8_t bin, uint8_t full);   // bin: 1~4, full: 1=满 0=未满

#endif
