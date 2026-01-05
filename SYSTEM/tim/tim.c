#include "tim.h"

// 全局变量用于计时
volatile uint32_t ms_tick = 0;    // 毫秒计数值
volatile uint16_t seconds_left = 15; // 倒计时秒数

void TIM2_Int_Init(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // 72MHz / 7200 = 10,000Hz (0.1ms计一次数)
    // 计 10 个数即为 1ms (1000Hz)
    TIM_TimeBaseStructure.TIM_Period = 10 - 1;          
    TIM_TimeBaseStructure.TIM_Prescaler = 7200 - 1;       
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
 
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM2, ENABLE);
}

void TIM2_IRQHandler(void) { // 1ms
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        ms_tick++; 
        
        // 每 1000ms 倒计时减 1
        if (ms_tick % 1000 == 0)
        {
            if (seconds_left > 0) seconds_left--;
        }
        
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}
