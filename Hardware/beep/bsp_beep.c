#include "bsp_beep.h"

/**
 * @brief  初始化控制蜂鸣器的IO
 * @param  无
 * @retval 无
 */
void BEEP_GPIO_Config(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 开启GPIO端口时钟 - F4使用AHB1总线 */
    RCC_AHB1PeriphClockCmd(BEEP_GPIO_CLK, ENABLE);

    /* 选择要控制蜂鸣器的GPIO */
    GPIO_InitStructure.GPIO_Pin = BEEP_GPIO_PIN;

    /* 设置GPIO模式为推挽输出 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;

    /* 设置输出类型为推挽 - F4新增 */
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;

    /* 设置上下拉 - F4新增 */
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

    /* 设置GPIO速率 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    /* 初始化GPIO */
    GPIO_Init(BEEP_GPIO_PORT, &GPIO_InitStructure);

    /* 关闭蜂鸣器 */
    GPIO_ResetBits(BEEP_GPIO_PORT, BEEP_GPIO_PIN);
}
