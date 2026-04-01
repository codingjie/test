#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f10x.h"

#define MOTOR_GPIO_PORT      GPIOB
#define MOTOR_GPIO_CLK       RCC_APB2Periph_GPIOB

#define MOTOR_SERVO_PIN      GPIO_Pin_13
#define MOTOR_FAN_PIN        GPIO_Pin_14
#define MOTOR_PUMP_PIN       GPIO_Pin_15

/* 低电平触发(Reset)或高电平触发(Set)视具体驱动模块而定，此处沿用原逻辑 */
#define MOTOR_SERVO_ON()     GPIO_ResetBits(MOTOR_GPIO_PORT, MOTOR_SERVO_PIN)
#define MOTOR_SERVO_OFF()    GPIO_SetBits(MOTOR_GPIO_PORT,   MOTOR_SERVO_PIN)

#define MOTOR_FAN_ON()       GPIO_ResetBits(MOTOR_GPIO_PORT, MOTOR_FAN_PIN)
#define MOTOR_FAN_OFF()      GPIO_SetBits(MOTOR_GPIO_PORT,   MOTOR_FAN_PIN)

#define MOTOR_PUMP_ON()      GPIO_ResetBits(MOTOR_GPIO_PORT, MOTOR_PUMP_PIN)
#define MOTOR_PUMP_OFF()     GPIO_SetBits(MOTOR_GPIO_PORT,   MOTOR_PUMP_PIN)

void MOTOR_GPIO_Config(void);

#endif
