#include "key.h"

void KEY_EXTI_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 开启 GPIOC 和 AFIO 复用功能时钟 */
    RCC_APB2PeriphClockCmd(KEY_CLK | RCC_APB2Periph_AFIO, ENABLE);

    /* 配置引脚为下拉输入 (IPD)，高电平表示按下 */
    GPIO_InitStructure.GPIO_Pin = KEY1_PIN | KEY2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(KEY_PORT, &GPIO_InitStructure);

    /* 设置引脚与中断线的映射关系 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource1);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource2);

    /* 配置 EXTI Line1 和 Line2，上升沿触发 */
    EXTI_InitStructure.EXTI_Line    = EXTI_Line1 | EXTI_Line2;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* 配置 NVIC — 优先级 0x0F，符合 FreeRTOS MAX_SYSCALL 要求 */
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;

    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 * @brief KEY1 (PC2 / EXTI2) 中断服务函数
 *        功能：光标在 Pump / Fan 行之间切换
 */
void EXTI2_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line2) != RESET) {
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

/**
 * @brief KEY2 (PC1 / EXTI1) 中断服务函数
 *        功能：修改当前选中行的值（Pump 开关 / Fan 转速 +10%）
 */
void EXTI1_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line1) != RESET) {
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}
