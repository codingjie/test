#include "key.h"
#include "delay.h"
#include "sys.h"

void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能 GPIOB 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    // PB12~PB15 内部下拉输入，按键接VCC（高电平有效）
    GPIO_InitStructure.GPIO_Pin   = KEY1_PIN | KEY2_PIN | KEY3_PIN | KEY4_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

// 非阻塞扫描：短按松开时返回1~4（高电平有效），否则返回0
uint8_t KEY_Scan(void)
{
    static uint8_t  last[4]  = {0, 0, 0, 0};
    static uint32_t tick0[4] = {0, 0, 0, 0};
    uint8_t cur[4], i, ret = 0;

    cur[0] = KEY1_READ();
    cur[1] = KEY2_READ();
    cur[2] = KEY3_READ();
    cur[3] = KEY4_READ();

    for (i = 0; i < 4; i++) {
        if (cur[i] == 1 && last[i] == 0) {        // 按下（上升沿）
            tick0[i] = g_tick_ms;
        } else if (cur[i] == 0 && last[i] == 1) { // 松开（下降沿）
            if ((g_tick_ms - tick0[i]) >= 20) ret = i + 1;
        }
        last[i] = cur[i];
    }
    return ret;
}
