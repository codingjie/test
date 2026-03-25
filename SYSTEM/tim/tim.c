#include "tim.h"

/**
 * 功能：初始化定时器2，配置为1ms中断一次
 * 解释：72MHz / 7200 = 10,000Hz (0.1ms计一次数)
 * 计10个数即为1ms
 */
void Timer2_Init(void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 开启TIM2时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // 定时器基本配置
    // 计数频率 = 72MHz / (PSC+1) = 72MHz / 7200 = 10kHz
    TIM_TimeBaseStructure.TIM_Period = 10 - 1; // ARR: 自动重装载值 (计10个数)
    TIM_TimeBaseStructure.TIM_Prescaler = 7200 - 1; // PSC: 预分频系数
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    // 开启定时器更新中断
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    // 清除更新标志位，防止初始化完立即进入一次中断
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);

    // 配置NVIC中断优先级
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 建议在main中配置一次即可

    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        // 响应优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 使能定时器
    TIM_Cmd(TIM2, ENABLE);
}
