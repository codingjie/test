#ifndef __TIM_H
#define __TIM_H

#include "stm32f10x.h"
#include "stm32f10x_tim.h"

/*
 * TIM2 双通道输入捕获测量 PWM 信号
 *   PA0  →  TIM2_CH1 (上升沿捕获，用于测量周期)
 *   PA0  →  TIM2_CH2 (下降沿捕获，通过 IndirectTI 复用)
 *
 *  预分频 PSC=71 → 计数频率 1MHz (1count=1µs)
 *  测量精度：
 *    10kHz → 周期100µs → ±1%
 *     1Hz  → 周期1s   → ±0.0001%
 */

void    PWM_Capture_Init(void);

/* 供 stm32f10x_it.c 中的 TIM2_IRQHandler 调用 */
void    PWM_TIM2_IRQHandler(void);

/* 主循环调用接口 */
uint8_t  PWM_Capture_Ready(void);   /* 1=有新数据(自动清标志) */
uint8_t  PWM_Signal_Present(void);  /* 1=信号存在 */
uint32_t PWM_Get_Freq(void);        /* Hz */
uint8_t  PWM_Get_Duty(void);        /* 0-100 % */

#endif /* __TIM_H */
