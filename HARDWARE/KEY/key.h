#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"
#include <stdint.h>

// 4个按键，低电平有效（内部上拉）
// KEY1: PC13
// KEY2: PC14
// KEY3: PC15
// KEY4: PA15（禁用JTAG后用作GPIO）
#define KEY1_PORT   GPIOC
#define KEY1_PIN    GPIO_Pin_13
#define KEY2_PORT   GPIOC
#define KEY2_PIN    GPIO_Pin_14
#define KEY3_PORT   GPIOC
#define KEY3_PIN    GPIO_Pin_15
#define KEY4_PORT   GPIOA
#define KEY4_PIN    GPIO_Pin_15

#define KEY1_READ() GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN)
#define KEY2_READ() GPIO_ReadInputDataBit(KEY2_PORT, KEY2_PIN)
#define KEY3_READ() GPIO_ReadInputDataBit(KEY3_PORT, KEY3_PIN)
#define KEY4_READ() GPIO_ReadInputDataBit(KEY4_PORT, KEY4_PIN)

void    KEY_Init(void);
uint8_t KEY_Scan(void);   // 短按返回1~4，无按键返回0

#endif
