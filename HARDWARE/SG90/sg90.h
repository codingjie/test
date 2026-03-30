#ifndef __SG90_H
#define __SG90_H

#include "stm32f10x.h"
#include <stdint.h>
#include "stm32f10x_tim.h"

// 4路SG90舵机由TIM3 PWM驱动，50Hz（20ms周期），1us分辨率
// 舵机1: PA6  TIM3_CH1  -> 可回收垃圾桶盖
// 舵机2: PA7  TIM3_CH2  -> 有害垃圾桶盖
// 舵机3: PB0  TIM3_CH3  -> 厨余垃圾桶盖
// 舵机4: PB1  TIM3_CH4  -> 其他垃圾桶盖
//
// 脉宽范围：500us（0°）~ 2500us（180°）

// TIM3预分频：72MHz / 72 = 1MHz，ARR=19999 -> 50Hz
#define SG90_TIM_PRESCALER  71
#define SG90_TIM_PERIOD     19999

#define SERVO_OPEN_ANGLE    90   // 开盖角度（度）
#define SERVO_CLOSE_ANGLE    0   // 关盖角度（度）

void SG90_Init(void);
void SG90_SetAngle(uint8_t servo, uint8_t angle);  // servo: 1~4, angle: 0~180
void SG90_Open(uint8_t servo);
void SG90_Close(uint8_t servo);

#endif
