#include "ultra.h"
#include "delay.h"

#define TRIG_PORT   GPIOA
#define TRIG_PIN    GPIO_Pin_8

static const uint16_t echo_pin[4] = {
    GPIO_Pin_6,   /* ECHO1 - PB6 */
    GPIO_Pin_7,   /* ECHO2 - PB7 */
    GPIO_Pin_8,   /* ECHO3 - PB8 */
    GPIO_Pin_9    /* ECHO4 - PB9 */
};

void ULTRA_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

    /* TRIG: output push-pull, start LOW */
    GPIO_InitStructure.GPIO_Pin   = TRIG_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TRIG_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(TRIG_PORT, TRIG_PIN);

    /* ECHO1~4: floating input */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6 | GPIO_Pin_7
                                 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/* Returns distance in cm, or 0xFFFF on timeout */
uint16_t ULTRA_GetDistance_cm(uint8_t ch)
{
    uint32_t t;
    uint16_t pin;

    if (ch < 1 || ch > 4) return 0xFFFF;
    pin = echo_pin[ch - 1];

    /* Send 10us TRIG pulse */
    GPIO_SetBits(TRIG_PORT, TRIG_PIN);
    delay_us(10);
    GPIO_ResetBits(TRIG_PORT, TRIG_PIN);

    /* Wait for ECHO to go HIGH (max 30ms) */
    t = 0;
    while (GPIO_ReadInputDataBit(GPIOB, pin) == Bit_RESET) {
        delay_us(1);
        if (++t > 30000) return 0xFFFF;
    }

    /* Measure ECHO HIGH duration (max 30ms) */
    t = 0;
    while (GPIO_ReadInputDataBit(GPIOB, pin) == Bit_SET) {
        delay_us(1);
        if (++t > 30000) return 0xFFFF;
    }

    /* distance_cm = t / 58 */
    return (uint16_t)(t / 58);
}

uint8_t ULTRA_IsFull(uint8_t ch)
{
    uint16_t d = ULTRA_GetDistance_cm(ch);
    if (d == 0xFFFF) return 0;   /* sensor error -> assume not full */
    return (d < ULTRA_FULL_CM) ? 1 : 0;
}
