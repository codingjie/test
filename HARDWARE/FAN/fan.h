#ifndef __FAN_H
#define __FAN_H

#include "stm32f10x.h"
#include "stm32f10x_tim.h"

// 风扇 PWM 引脚：PA6 -> TIM3_CH1
// 使用 TIM3，预分频 0，周期 2879 → 72MHz / 2880 = 25kHz PWM
// 低速档：占空比 30%（CCR = 864）
// 高速档：占空比 80%（CCR = 2304）

#define FAN_TIM_PERIOD   2879     // 25kHz PWM 周期值
#define FAN_DUTY_LOW     864      // 30% 占空比（低速/散热）
#define FAN_DUTY_HIGH    2304     // 80% 占空比（高速/排烟）
#define FAN_DUTY_OFF     0        // 关闭

// 风扇档位
typedef enum {
    FAN_OFF  = 0,   // 关闭
    FAN_LOW  = 1,   // 低速（一级预警散热）
    FAN_HIGH = 2    // 高速（二级报警排烟）
} FanSpeed_t;

void FAN_Init(void);
void FAN_SetSpeed(FanSpeed_t speed);

#endif
