#ifndef __SG90_H
#define __SG90_H

#include "stm32f10x.h"
#include "stm32f10x_tim.h"

/* ---- 引脚宏定义 ---- */
#define SG90_GPIO_CLK_A     RCC_APB2Periph_GPIOA
#define SG90_GPIO_CLK_B     RCC_APB2Periph_GPIOB
#define SG90_TIM_CLK        RCC_APB1Periph_TIM3

#define SG90_1_PORT         GPIOA           /* TIM3_CH1 PA6 */
#define SG90_1_PIN          GPIO_Pin_6

#define SG90_2_PORT         GPIOA           /* TIM3_CH2 PA7 */
#define SG90_2_PIN          GPIO_Pin_7

#define SG90_3_PORT         GPIOB           /* TIM3_CH3 PB0 */
#define SG90_3_PIN          GPIO_Pin_0

#define SG90_4_PORT         GPIOB           /* TIM3_CH4 PB1 */
#define SG90_4_PIN          GPIO_Pin_1

/* ---- 定时器参数: 72MHz/72=1MHz, ARR=20000 → 50Hz ---- */
#define SG90_TIM_PRESCALER  71
#define SG90_TIM_PERIOD     19999

/* ---- 脉宽 / 角度范围 ---- */
#define SG90_PULSE_MIN      500     /* 0°   → 500us  */
#define SG90_PULSE_MAX      2500    /* 180° → 2500us */
#define SG90_ANGLE_MIN      0
#define SG90_ANGLE_MAX      180

/* ---- 桶盖角度 ---- */
#define SG90_ANGLE_OPEN     0       /* 0°  → 开盖 */
#define SG90_ANGLE_CLOSE    90      /* 90° → 关盖 */
#define SG90_ANGLE_CLOSE_2  80      /* 80° → 有害垃圾桶关盖 */

void    SG90_Init(void);
void    SG90_SetPulse(uint8_t ch, uint16_t pulse_us);   /* ch: 1~4 */
void    SG90_SetAngle(uint8_t ch, uint8_t angle);       /* ch: 1~4, angle: 0~180 */
void    SG90_Open(uint8_t ch);                          /* ch: 1~4, 开盖 */
void    SG90_Close(uint8_t ch);                         /* ch: 1~4, 关盖 */

#endif
