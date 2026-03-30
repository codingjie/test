#include "infrared.h"

/* All 4 IR ECHO pins are on GPIOA */
void IR_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4 | GPIO_Pin_5
                                  | GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/* Returns 1 when obstacle (person) is detected (pin LOW), 0 otherwise */
uint8_t IR_Detected(uint8_t ch)
{
    switch (ch) {
        case 1: return (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4)  == Bit_RESET) ? 1 : 0;
        case 2: return (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5)  == Bit_RESET) ? 1 : 0;
        case 3: return (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_11) == Bit_RESET) ? 1 : 0;
        case 4: return (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12) == Bit_RESET) ? 1 : 0;
        default: return 0;
    }
}
