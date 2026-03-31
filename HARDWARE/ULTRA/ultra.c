#include "ultra.h"
#include "delay.h"
#include "core_cm3.h"

#define TRIG_PORT   GPIOA
#define TRIG_PIN    GPIO_Pin_8

/* SYSCLK = 72MHz */
#define CYCLES_PER_US   72UL

static const uint16_t echo_pin[4] = {
    GPIO_Pin_6,   // ECHO1 - PB6
    GPIO_Pin_7,   // ECHO2 - PB7
    GPIO_Pin_8,   // ECHO3 - PB8
    GPIO_Pin_9    // ECHO4 - PB9
};

void ULTRA_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

    // TRIG：推挽输出，初始低电平
    GPIO_InitStructure.GPIO_Pin   = TRIG_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TRIG_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(TRIG_PORT, TRIG_PIN);

    // ECHO1~4：浮空输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6 | GPIO_Pin_7
                                 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 使能 DWT 周期计数器，用于精确计时
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
}

// 返回距离（毫米），超时或异常返回 0xFFFF
// 精确计时：使用 DWT->CYCCNT (72MHz)，避免 delay_us 循环误差
uint16_t ULTRA_GetDistance_mm(uint8_t ch)
{
    uint32_t t1, t2, deadline;
    uint16_t pin;

    if (ch < 1 || ch > 4) return 0xFFFF;
    pin = echo_pin[ch - 1];

    // 发送 10us TRIG 触发脉冲
    GPIO_SetBits(TRIG_PORT, TRIG_PIN);
    delay_us(10);
    GPIO_ResetBits(TRIG_PORT, TRIG_PIN);

    // 等待 ECHO 变高（最长 30ms）
    deadline = DWT->CYCCNT + CYCLES_PER_US * 30000UL;
    while (GPIO_ReadInputDataBit(GPIOB, pin) == Bit_RESET) {
        if ((int32_t)(DWT->CYCCNT - deadline) >= 0) return 0xFFFF;
    }

    // 记录 ECHO 上升沿时刻
    t1 = DWT->CYCCNT;

    // 等待 ECHO 变低（最长 30ms）
    deadline = t1 + CYCLES_PER_US * 30000UL;
    while (GPIO_ReadInputDataBit(GPIOB, pin) == Bit_SET) {
        if ((int32_t)(DWT->CYCCNT - deadline) >= 0) return 0xFFFF;
    }

    // 记录 ECHO 下降沿时刻
    t2 = DWT->CYCCNT;

    // 距离(mm) = echo时长(us) / 5.8
    //           = (t2 - t1) / CYCLES_PER_US / 5.8
    //           = (t2 - t1) * 10 / (CYCLES_PER_US * 58)
    return (uint16_t)((t2 - t1) * 10 / (CYCLES_PER_US * 58));
}

// 返回距离（厘米），超时返回 0xFFFF
uint16_t ULTRA_GetDistance_cm(uint8_t ch)
{
    uint16_t d = ULTRA_GetDistance_mm(ch);
    if (d == 0xFFFF) return 0xFFFF;
    return d / 10;
}

uint8_t ULTRA_IsFull(uint8_t ch)
{
    uint16_t d = ULTRA_GetDistance_mm(ch);
    if (d == 0xFFFF) return 0;   // 传感器异常，视为未满
    return (d < ULTRA_FULL_MM) ? 1 : 0;
}
