#ifndef __IR_H
#define __IR_H

#include "stm32f10x.h"
#include <stdint.h>

/*
 * 红外发送  PA1  TIM2_CH2 38kHz PWM（调制载波）
 * 红外接收  PB8  EXTI输入捕获
 *
 * 注：TIM2同时为38kHz载波发生器，不再用于系统tick（已改为TIM4）。
 *
 * 发送接口：
 *   IR_TX_Enable()   / IR_TX_Disable()   开关38kHz载波（调制逻辑"1"）
 *   IR_SendNEC(addr, cmd)                发送NEC格式帧
 *
 * 接收接口：
 *   IR_RX_GetAddr()  / IR_RX_GetCmd()   获取最近解码地址/命令
 *   IR_RX_HasData()                     是否收到新帧（读后自动清零）
 *   EXTI9_5_IRQHandler 中调用 IR_RX_IRQHandler()
 */

void     IR_Init(void);

/* 发送 */
void     IR_TX_Enable(void);
void     IR_TX_Disable(void);
void     IR_SendNEC(uint8_t addr, uint8_t cmd);

/* 接收 */
void     IR_RX_IRQHandler(void);   /* 在 EXTI9_5_IRQHandler 中调用 */
uint8_t  IR_RX_HasData(void);
uint8_t  IR_RX_GetAddr(void);
uint8_t  IR_RX_GetCmd(void);

#endif
