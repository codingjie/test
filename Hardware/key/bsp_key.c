#include "bsp_key.h"

void Key_EXTI_Config(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 1. 开启 GPIO 和 SYSCFG 时钟（EXTI 映射需要 SYSCFG） */
    RCC_AHB1PeriphClockCmd(KEY1_GPIO_CLK | KEY2_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* 2. 配置 GPIO 为输入模式 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; 
    
    GPIO_InitStructure.GPIO_Pin = KEY1_PIN;
    GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = KEY2_PIN;
    GPIO_Init(KEY2_GPIO_PORT, &GPIO_InitStructure);

    /* 3. 连接 EXTI 线到对应的 GPIO 引脚 */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);  // KEY1 -> PA0 -> EXTI0
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource13); // KEY2 -> PC13 -> EXTI13

    /* 4. 配置 EXTI 中断线 */
    // KEY1
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; // 假设按下为高电平，上升沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // KEY2
    EXTI_InitStructure.EXTI_Line = EXTI_Line13;
    EXTI_Init(&EXTI_InitStructure);
    
    // KEY1 中断通道
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // KEY2 中断通道
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn; // EXTI10-15 共用一个入口
    NVIC_Init(&NVIC_InitStructure);
}
