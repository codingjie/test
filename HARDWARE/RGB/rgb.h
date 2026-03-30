#ifndef __RGB_H
#define __RGB_H

#include "stm32f10x.h"
#include <stdint.h>

/* 8 status LEDs, 2 per bin (RED = full, GREEN = not full)
 * LED1  PA0   Bin1-Recyclable  RED
 * LED2  PA1   Bin1-Recyclable  GREEN
 * LED3  PB13  Bin2-Hazardous   RED
 * LED4  PB14  Bin2-Hazardous   GREEN
 * LED5  PB15  Bin3-Kitchen     RED
 * LED6  PB3   Bin3-Kitchen     GREEN  (JTAG disabled)
 * LED7  PB4   Bin4-Other       RED    (JTAG disabled)
 * LED8  PB5   Bin4-Other       GREEN
 *
 * Note: GPIO_Remap_SWJ_JTAGDisable must be called before using PB3/PB4.
 *       KEY_Init() already does this; LED_Init() calls it as well for safety.
 */

void LED_Init(void);
void LED_Set(uint8_t led, uint8_t on);              /* led: 1~8, on: 1=on 0=off */
void LED_SetBinStatus(uint8_t bin, uint8_t full);   /* bin: 1~4, full: 1=full 0=ok */

#endif
