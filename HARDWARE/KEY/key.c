#include "key.h"
#include "delay.h"
#include "sys.h"

/**
 * @brief 初始化4个按键引脚为上拉输入
 *        PA15 默认为 JTDI，需先关闭 JTAG 才能作普通 GPIO 使用
 */
void KEY_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 使能 AFIO 时钟，禁用 JTAG（保留 SWD），释放 PA15 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    RCC_APB2PeriphClockCmd(KEY1_CLK, ENABLE);   /* GPIOA */
    RCC_APB2PeriphClockCmd(KEY4_CLK, ENABLE);   /* GPIOB */

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    /* PA5, PA8, PA15 上拉输入 */
    GPIO_InitStructure.GPIO_Pin = KEY1_PIN | KEY2_PIN | KEY3_PIN;
    GPIO_Init(KEY1_PORT, &GPIO_InitStructure);

    /* PB0 上拉输入 */
    GPIO_InitStructure.GPIO_Pin = KEY4_PIN;
    GPIO_Init(KEY4_PORT, &GPIO_InitStructure);
}

// 非阻塞按键检测
uint8_t KEY_Scan(void) {
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
            uint32_t dur = g_tick_ms - tick0[i];
            if (dur >= 20) ret = i + 1;   // 仅短按
        }
        last[i] = cur[i];
    }
    return ret;
}
