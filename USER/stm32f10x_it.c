#include "stm32f10x_it.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_exti.h"
#include "sys.h"
#include "ir.h"
#include "jy61p.h"

/* ---- 内核异常 ---------------------------------------------------- */
void NMI_Handler(void)        {}
void HardFault_Handler(void)  { while (1) {} }
void MemManage_Handler(void)  { while (1) {} }
void BusFault_Handler(void)   { while (1) {} }
void UsageFault_Handler(void) { while (1) {} }
void SVC_Handler(void)        {}
void DebugMon_Handler(void)   {}
void PendSV_Handler(void)     {}
void SysTick_Handler(void)    {}

/* ---- 外设中断 ----------------------------------------------------- */

/* 1ms 系统节拍 (TIM4，TIM2已用于IR 38kHz) */
void TIM4_IRQHandler(void) {
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) {
        g_tick_ms++;
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    }
}

/* 红外接收 PB8 → EXTI线8 (EXTI9_5) */
void EXTI9_5_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line8) != RESET) {
        IR_RX_IRQHandler();
        EXTI_ClearITPendingBit(EXTI_Line8);
    }
}

/* JY61P 姿态传感器 USART3 接收 */
void USART3_IRQHandler(void) {
    JY61P_IRQHandler();
}
