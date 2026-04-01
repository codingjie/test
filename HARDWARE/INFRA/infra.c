#include "infra.h"

extern uint8_t infra_flag;

void INFRA_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 1. 开启时钟 (GPIOB 和 复用功能 AFIO)
    RCC_APB2PeriphClockCmd(INFRA_GPIO_CLK | RCC_APB2Periph_AFIO, ENABLE);

    // 2. 配置 GPIO 为上拉输入
    GPIO_InitStructure.GPIO_Pin   = INFRA_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_Init(INFRA_GPIO_PORT, &GPIO_InitStructure);

    // 3. 配置中断线路 (PB0 对应 Line0)
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);

    EXTI_InitStructure.EXTI_Line    = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // 4. 配置 NVIC 中断优先级
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  中断服务函数
  */
void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        infra_flag = !infra_flag; // 触发标志位
        EXTI_ClearITPendingBit(EXTI_Line0); // 清除中断标志
    }
}
