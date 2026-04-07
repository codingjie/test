#include "stm32f10x_it.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"
#include "sys.h"
#include "jw01.h"
#include "esp01s.h"

// ---- 内核异常 -------------------------------------------------------
void NMI_Handler(void)        {}
void HardFault_Handler(void)  { while (1) {} }
void MemManage_Handler(void)  { while (1) {} }
void BusFault_Handler(void)   { while (1) {} }
void UsageFault_Handler(void) { while (1) {} }
void SVC_Handler(void)        {}
void DebugMon_Handler(void)   {}
void PendSV_Handler(void)     {}
void SysTick_Handler(void)    {}

// ---- TIM2：1ms 系统节拍 -------------------------------------------
void TIM2_IRQHandler(void) {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        g_tick_ms++;
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}

// ---- USART1：JW01 VOC 传感器接收 ----------------------------------
void USART1_IRQHandler(void) {
    JW01_IRQHandler();
}

// ---- USART2：ESP01S Wi-Fi 模块接收 --------------------------------
void USART2_IRQHandler(void) {
    ESP01S_IRQHandler();
}
