/**
 ******************************************************************************
 * @file    stm32f10x_it.c
 * @brief   中断服务函数
 ******************************************************************************
 */

#include "stm32f10x_it.h"
#include "tim.h"

void NMI_Handler(void) {}

void HardFault_Handler(void) { while (1) {} }

void MemManage_Handler(void) { while (1) {} }

void BusFault_Handler(void) { while (1) {} }

void UsageFault_Handler(void) { while (1) {} }

void SVC_Handler(void) {}

void DebugMon_Handler(void) {}

void PendSV_Handler(void) {}

void SysTick_Handler(void) {}

/******************************************************************************/

/**
 * @brief TIM2 全局中断：PWM 输入捕获（周期 + 占空比）
 *        CC1=上升沿(周期), CC2=下降沿(高电平时间), Update=溢出
 */
void TIM2_IRQHandler(void) {
    PWM_TIM2_IRQHandler();
}
