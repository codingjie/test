#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"
#include <stdint.h>

// 4个按键，高电平有效（内部下拉，按键接VCC）
// KEY1(工作模式): PB12
// KEY2(测试模式): PB13
// KEY3(重置):     PB14
// KEY4(启动积分): PB15
#define KEY1_PORT   GPIOB
#define KEY1_PIN    GPIO_Pin_12
#define KEY2_PORT   GPIOB
#define KEY2_PIN    GPIO_Pin_13
#define KEY3_PORT   GPIOB
#define KEY3_PIN    GPIO_Pin_14
#define KEY4_PORT   GPIOB
#define KEY4_PIN    GPIO_Pin_15

#define KEY1_READ() GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN)
#define KEY2_READ() GPIO_ReadInputDataBit(KEY2_PORT, KEY2_PIN)
#define KEY3_READ() GPIO_ReadInputDataBit(KEY3_PORT, KEY3_PIN)
#define KEY4_READ() GPIO_ReadInputDataBit(KEY4_PORT, KEY4_PIN)

void    KEY_Init(void);
uint8_t KEY_Scan(void);   // 短按返回1~4，无按键返回0

#endif
