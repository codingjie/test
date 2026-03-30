#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"
#include <stdint.h>

/* 4 buttons, active LOW (internal pull-up)
 * KEY1: PC13
 * KEY2: PC14
 * KEY3: PC15
 * KEY4: PA15  (JTAG disabled to use as GPIO)
 */
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
uint8_t KEY_Scan(void);   /* returns 1~4 on short press, 0 if none */

#endif
