#ifndef __SG90_H
#define __SG90_H

#include "stm32f10x.h"
#include "stm32f10x_tim.h"

// ---- 引脚: PA6  TIM3_CH1 ----
#define SG90_GPIO_CLK   RCC_APB2Periph_GPIOA
#define SG90_TIM_CLK    RCC_APB1Periph_TIM3
#define SG90_PORT       GPIOA
#define SG90_PIN        GPIO_Pin_6    // TIM3_CH1

// ---- 定时器: 72MHz/72=1MHz, ARR=20000 → 50Hz ----
#define SG90_TIM_PRESCALER  71
#define SG90_TIM_PERIOD     19999

// ---- 脉宽 / 角度 ----
#define SG90_PULSE_MIN   500    // 0°
#define SG90_PULSE_MAX   2500   // 180°
#define SG90_ANGLE_MIN   0
#define SG90_ANGLE_MAX   180

#define SG90_ANGLE_OPEN  0      // 开
#define SG90_ANGLE_CLOSE 90     // 关

void    SG90_Init(void);
void    SG90_SetPulse(uint16_t pulse_us);
void    SG90_SetAngle(uint8_t angle);
void    SG90_Open(void);
void    SG90_Close(void);

#endif
