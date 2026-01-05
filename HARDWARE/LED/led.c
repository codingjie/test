#include "led.h"

void LED_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // 再配置为推挽输出
    GPIO_InitStructure.GPIO_Pin = SNGREEN_PIN | SNRED_PIN | SNYELLOW_PIN | 
                                 EWGREEN_PIN | EWRED_PIN | EWYELLOW_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 先将引脚电平预设为高电平（灭）
    GPIO_SetBits(GPIOA, SNGREEN_PIN | SNRED_PIN | SNYELLOW_PIN | 
                        EWGREEN_PIN | EWRED_PIN | EWYELLOW_PIN);
}
