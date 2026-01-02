#ifndef __BSP_CS100A_H
#define __BSP_CS100A_H

#include "stm32f4xx.h"

// 定时器预分频（90MHz / 90 = 1MHz，1us计数）
#define GENERAL_TIM_PRESCALER               89
// 定时器周期
#define GENERAL_TIM_PERIOD                  0xFFFF

/************通用定时器TIM参数定义************/
#define GENERAL_TIMx                        TIM4
#define GENERAL_TIM_CLK                     RCC_APB1Periph_TIM4
#define GENERAL_TIM_IRQn                    TIM4_IRQn
#define GENERAL_TIM_IRQHANDLER              TIM4_IRQHandler

// 输入捕获通道
#define GENERAL_TIM_CHANNEL                 TIM_Channel_3
#define GENERAL_TIM_IT_CCx                  TIM_IT_CC3
#define GENERAL_TIM_GetCapturex             TIM_GetCapture3

// ECHO引脚定义（TIM4_CH3 -> PB8）
#define ECHO_GPIO_CLK                       RCC_AHB1Periph_GPIOB
#define ECHO_GPIO_PORT                      GPIOB
#define ECHO_GPIO_PIN                       GPIO_Pin_8
#define ECHO_GPIO_PINSOURCE                 GPIO_PinSource8
#define ECHO_GPIO_AF                        GPIO_AF_TIM4

// TRIG引脚定义
#define TRIG_GPIO_CLK                       RCC_AHB1Periph_GPIOB
#define TRIG_GPIO_PORT                      GPIOB
#define TRIG_GPIO_PIN                       GPIO_Pin_5

// 测量的起始边沿
#define GENERAL_TIM_START_ICPolarity        TIM_ICPolarity_Rising
// 测量的结束边沿
#define GENERAL_TIM_END_ICPolarity          TIM_ICPolarity_Falling

// 定时器输入捕获用户自定义变量结构体声明
typedef struct              
{   
    uint8_t   ucFinishFlag;   // 捕获结束标志位
    uint8_t   ucStartFlag;    // 捕获开始标志位
    uint16_t  usCtr;          // 捕获寄存器的值
    uint16_t  usPeriod;       // 自动重装载寄存器更新标志 
}STRUCT_CAPTURE; 

extern STRUCT_CAPTURE TIM_ICUserValueStructure;

void CS100A_TRIG(void);
void CS100A_Init(void);
float CS100A_GetDistance(void);

#endif /* __BSP_CS100A_H */
