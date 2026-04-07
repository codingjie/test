#include "led.h"

/*
 * LED_Init: 配置所有LED引脚为推挽输出。
 * 初始状态: 绿色LED1常亮，其余全灭。
 * 注意: PA6(蓝色LED1)与TIM3_CH1共用，调用SG90_Init()后
 *       该引脚将被重配为复用推挽，LED宏失效。
 */
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

    /* PA0(G1) PA2(Y) PA3(R1) PA4(R2) PA6(B1) */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3
                                  | GPIO_Pin_4 | GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* PB0(G2) PB1(B2) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* 初始状态 */
    LED_GREEN1_ON();   /* PA0: 上电常亮 */
    LED_GREEN2_OFF();
    LED_BLUE1_OFF();
    LED_BLUE2_OFF();
    LED_YELLOW_OFF();
    LED_RED1_OFF();
    LED_RED2_OFF();
}
