#ifndef __BSP_TIMER_ENCODER_H
#define __BSP_TIMER_ENCODER_H

#include "stm32f4xx.h"

/************通用定时器TIM参数定义************/
#define TIM_ENCODER                         TIM3        
#define TIM_ENCODER_RCC_CLK_ENABLE          RCC_APB1PeriphClockCmd
#define TIM_ENCODER_RCC_CLK                 RCC_APB1Periph_TIM3

#define ENCODER_RCC_CLK_ENABLE              RCC_AHB1PeriphClockCmd
#define ENCODER_RCC_CLK                     RCC_AHB1Periph_GPIOC

/* TIMER3_CH1:PC6----> Ecoder_A */
#define ENCODER_A_GPIO_PORT                 GPIOC
#define ENCODER_A_GPIO_PIN                  GPIO_Pin_6
#define ENCODER_A_GPIO_PINSOURCE            GPIO_PinSource6
#define ENCODER_A_GPIO_AF                   GPIO_AF_TIM3

/* TIMER3_CH2:PC7----> Ecoder_B */
#define ENCODER_B_GPIO_PORT                 GPIOC
#define ENCODER_B_GPIO_PIN                  GPIO_Pin_7
#define ENCODER_B_GPIO_PINSOURCE            GPIO_PinSource7
#define ENCODER_B_GPIO_AF                   GPIO_AF_TIM3

#define ENCODER_A_CHANNEL                   TIM_Channel_1
#define ENCODER_B_CHANNEL                   TIM_Channel_2

/* Ecoder_SW */
#define ENCODER_KEY_RCC_CLK_ENABLE          RCC_AHB1PeriphClockCmd
#define ENCODER_KEY_RCC_CLK                 RCC_AHB1Periph_GPIOA
#define ENCODER_KEY_GPIO_PORT               GPIOA
#define ENCODER_KEY_GPIO_PIN                GPIO_Pin_8

#define ENCODER_TASK_TIME   200     //编码器任务时间单位ms

#define STILLNESS           0       //静止
#define POSITIVE_DIRECTION  1       //正方向
#define REVERSE_DIRECTION   2       //反方向

#define PRESS               1       //按键按下
#define LOOSEN              2       //按键松开

void TIMX_Encoder_Init(void); 
void Encoder_Get_Val(float *cycle_count);

extern uint8_t dirction_flag;

#endif /* __BSP_TIMER_ENCODER_H */
