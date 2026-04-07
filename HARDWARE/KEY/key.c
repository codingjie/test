#include "key.h"
#include "delay.h"
#include "sys.h"

void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能 AFIO + GPIOA + GPIOC 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO
                           | RCC_APB2Periph_GPIOA
                           | RCC_APB2Periph_GPIOC, ENABLE);

    // 禁用JTAG（保留SWD）：释放PA15、PB3、PB4用作GPIO
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    // PC13、PC14、PC15
    GPIO_InitStructure.GPIO_Pin = KEY1_PIN | KEY2_PIN | KEY3_PIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // PA15
    GPIO_InitStructure.GPIO_Pin = KEY4_PIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

// 非阻塞扫描：短按松开时返回1~4，否则返回0
uint8_t KEY_Scan(void)
{
    static uint8_t  last[4]  = {1, 1, 1, 1};
    static uint32_t tick0[4] = {0, 0, 0, 0};
    uint8_t cur[4], i, ret = 0;

    cur[0] = KEY1_READ();
    cur[1] = KEY2_READ();
    cur[2] = KEY3_READ();
    cur[3] = KEY4_READ();

    for (i = 0; i < 4; i++) {
        if (cur[i] == 0 && last[i] == 1) {
            tick0[i] = g_tick_ms;
        } else if (cur[i] == 1 && last[i] == 0) {
            if ((g_tick_ms - tick0[i]) >= 20) ret = i + 1;
        }
        last[i] = cur[i];
    }
    return ret;
}
