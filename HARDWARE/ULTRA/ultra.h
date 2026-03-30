#ifndef __ULTRA_H
#define __ULTRA_H

#include "stm32f10x.h"
#include <stdint.h>

/* 4 HC-SR04 ultrasonic sensors, TRIG shared on PA8
 * ECHO1: PB6  -> Recyclable bin
 * ECHO2: PB7  -> Hazardous bin
 * ECHO3: PB8  -> Kitchen waste bin
 * ECHO4: PB9  -> Other waste bin
 *
 * Sensors are triggered sequentially (one at a time).
 * Distance = echo_high_us / 58  (cm)
 */

#define ULTRA_FULL_CM   10   /* distance < 10 cm -> bin is considered full */

void     ULTRA_Init(void);
uint16_t ULTRA_GetDistance_cm(uint8_t ch);  /* ch: 1~4; 0xFFFF on timeout */
uint8_t  ULTRA_IsFull(uint8_t ch);          /* 1 = full, 0 = not full     */

#endif
