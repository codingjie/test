#include "rgb.h"

static GPIO_TypeDef * const led_port[8] = {
    GPIOA, GPIOA,   // LED1 PA0, LED2 PA1
    GPIOB, GPIOB,   // LED3 PB13, LED4 PB14
    GPIOB, GPIOB,   // LED5 PB15, LED6 PB3
    GPIOB, GPIOB    // LED7 PB4,  LED8 PB5
};

static const uint16_t led_pin[8] = {
    GPIO_Pin_0,   // LED1 PA0
    GPIO_Pin_1,   // LED2 PA1
    GPIO_Pin_13,  // LED3 PB13
    GPIO_Pin_14,  // LED4 PB14
    GPIO_Pin_15,  // LED5 PB15
    GPIO_Pin_3,   // LED6 PB3
    GPIO_Pin_4,   // LED7 PB4
    GPIO_Pin_5    // LED8 PB5
};

void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB
                           | RCC_APB2Periph_AFIO, ENABLE);
    // 释放PB3、PB4（PA15由KEY使用）
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    // PA0、PA1
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOA, GPIO_Pin_0 | GPIO_Pin_1);

    // PB3、PB4、PB5、PB13、PB14、PB15
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3  | GPIO_Pin_4  | GPIO_Pin_5
                                | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_3  | GPIO_Pin_4  | GPIO_Pin_5
                        | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
}

void LED_Set(uint8_t led, uint8_t on)
{
    if (led < 1 || led > 8) return;
    if (on)
        GPIO_SetBits(led_port[led - 1], led_pin[led - 1]);
    else
        GPIO_ResetBits(led_port[led - 1], led_pin[led - 1]);
}

// 根据满仓状态设置对应垃圾桶的红/绿LED
void LED_SetBinStatus(uint8_t bin, uint8_t full)
{
    uint8_t red_led   = (bin - 1) * 2 + 1;  // 1, 3, 5, 7
    uint8_t green_led = (bin - 1) * 2 + 2;  // 2, 4, 6, 8
    if (bin < 1 || bin > 4) return;
    LED_Set(red_led,   full ? 1 : 0);
    LED_Set(green_led, full ? 0 : 1);
}
