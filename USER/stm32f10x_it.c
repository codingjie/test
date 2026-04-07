#include "stm32f10x_it.h"
#include "stm32f10x_tim.h"
#include "sys.h"

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
void TIM2_IRQHandler(void) {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        g_tick_ms++;
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}
