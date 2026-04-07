#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"

// 绿色LED1  PA0  上电常亮
// 绿色LED2  PB0  备用
// 蓝色LED1  PA6  Z轴积分达5m亮（注：PA6同时用作TIM3_CH1舵机PWM，
//                调用SG90_Init后该引脚切换为复用推挽，LED不再可控）
// 蓝色LED2  PB1  备用
// 黄色LED   PA2  1Hz闪烁（由主循环控制）
// 红色LED1  PA3  测试模式按键后亮
// 红色LED2  PA4  备用
//
// 所有LED 高电平亮。

#define LED_GREEN1_ON()    GPIO_SetBits(GPIOA,   GPIO_Pin_0)
#define LED_GREEN1_OFF()   GPIO_ResetBits(GPIOA, GPIO_Pin_0)
#define LED_GREEN1_TOG()   GPIO_WriteBit(GPIOA,  GPIO_Pin_0, \
    (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_0)))

#define LED_GREEN2_ON()    GPIO_SetBits(GPIOB,   GPIO_Pin_0)
#define LED_GREEN2_OFF()   GPIO_ResetBits(GPIOB, GPIO_Pin_0)
#define LED_GREEN2_TOG()   GPIO_WriteBit(GPIOB,  GPIO_Pin_0, \
    (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_0)))

#define LED_BLUE1_ON()     GPIO_SetBits(GPIOA,   GPIO_Pin_6)
#define LED_BLUE1_OFF()    GPIO_ResetBits(GPIOA, GPIO_Pin_6)
#define LED_BLUE1_TOG()    GPIO_WriteBit(GPIOA,  GPIO_Pin_6, \
    (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_6)))

#define LED_BLUE2_ON()     GPIO_SetBits(GPIOB,   GPIO_Pin_1)
#define LED_BLUE2_OFF()    GPIO_ResetBits(GPIOB, GPIO_Pin_1)
#define LED_BLUE2_TOG()    GPIO_WriteBit(GPIOB,  GPIO_Pin_1, \
    (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_1)))

#define LED_YELLOW_ON()    GPIO_SetBits(GPIOA,   GPIO_Pin_2)
#define LED_YELLOW_OFF()   GPIO_ResetBits(GPIOA, GPIO_Pin_2)
#define LED_YELLOW_TOG()   GPIO_WriteBit(GPIOA,  GPIO_Pin_2, \
    (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_2)))

#define LED_RED1_ON()      GPIO_SetBits(GPIOA,   GPIO_Pin_3)
#define LED_RED1_OFF()     GPIO_ResetBits(GPIOA, GPIO_Pin_3)
#define LED_RED1_TOG()     GPIO_WriteBit(GPIOA,  GPIO_Pin_3, \
    (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_3)))

#define LED_RED2_ON()      GPIO_SetBits(GPIOA,   GPIO_Pin_4)
#define LED_RED2_OFF()     GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define LED_RED2_TOG()     GPIO_WriteBit(GPIOA,  GPIO_Pin_4, \
    (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_4)))

void LED_Init(void);

#endif
