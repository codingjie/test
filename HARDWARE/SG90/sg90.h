#ifndef __SG90_H
#define __SG90_H

#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include <stdint.h>

/* SG90 舵机 — TIM3_CH1 PWM
 * PA6: TIM3_CH1
 * PWM 频率: 50Hz (20ms 周期)
 * 脉宽范围: 500us ~ 2500us 对应 0° ~ 180°
 */
#define SG90_GPIO_PORT      GPIOA
#define SG90_GPIO_CLK       RCC_APB2Periph_GPIOA
#define SG90_GPIO_PIN       GPIO_Pin_6
#define SG90_TIM            TIM3
#define SG90_TIM_CLK        RCC_APB1Periph_TIM3

/* TIM3 预分频: 72MHz / 72 = 1MHz, 周期 ARR=20000 → 50Hz */
#define SG90_TIM_PRESCALER  71
#define SG90_TIM_PERIOD     19999

#define SERVO_CENTER  100
#define SERVO_MIN      40
#define SERVO_MAX     160

void    SG90_Init(void);
void    SG90_SetAngle(uint8_t angle);   /* angle: 0 ~ 180 */
void    SG90_SetPulse(uint16_t pulse_us); /* pulse_us: 500 ~ 2500 */

#endif
