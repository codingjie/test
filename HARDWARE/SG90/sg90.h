#ifndef __SG90_H
#define __SG90_H

#include "stm32f10x.h"
#include <stdint.h>

/* 4 SG90 servos driven by TIM3 PWM, 50Hz (20ms period), 1us resolution
 * Servo1: PA6  TIM3_CH1  -> Recyclable bin lid
 * Servo2: PA7  TIM3_CH2  -> Hazardous bin lid
 * Servo3: PB0  TIM3_CH3  -> Kitchen waste bin lid
 * Servo4: PB1  TIM3_CH4  -> Other waste bin lid
 *
 * Pulse range: 500us (0deg) ~ 2500us (180deg)
 */

/* TIM3 preset: 72MHz / 72 = 1MHz tick, ARR=19999 -> 50Hz */
#define SG90_TIM_PRESCALER  71
#define SG90_TIM_PERIOD     19999

#define SERVO_OPEN_ANGLE    90   /* lid open  position (degrees) */
#define SERVO_CLOSE_ANGLE    0   /* lid close position (degrees) */

void SG90_Init(void);
void SG90_SetAngle(uint8_t servo, uint8_t angle);  /* servo: 1~4, angle: 0~180 */
void SG90_Open(uint8_t servo);
void SG90_Close(uint8_t servo);

#endif
