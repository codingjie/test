#include "key.h"

void KEY_GPIO_Config(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(KEY_GPIO_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin  = KEY1_GPIO_PIN | KEY2_GPIO_PIN |
                                   KEY3_GPIO_PIN | KEY4_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; /* 上拉输入，低电平有效 */
    GPIO_Init(KEY_GPIO_PORT, &GPIO_InitStructure);
}

/*
 * KEY_Scan - 轮询式按键扫描，约每 10 ms 调用一次。
 * 返回当前发生的按键事件（KEY_NONE 表示无事件）。
 * 同一次按压只返回一次事件：长按在达到阈值时立即返回；
 * 短按在按键释放时返回。
 */
uint8_t KEY_Scan(void) {
    /* 当前按下的键编号（1~4），0=无按键 */
    static uint8_t held_key  = 0;
    static uint16_t hold_cnt = 0;
    static uint8_t long_fired = 0; /* 该次长按是否已触发过 */

    uint8_t cur = 0;

    if      (!GPIO_ReadInputDataBit(KEY_GPIO_PORT, KEY1_GPIO_PIN)) cur = 1;
    else if (!GPIO_ReadInputDataBit(KEY_GPIO_PORT, KEY2_GPIO_PIN)) cur = 2;
    else if (!GPIO_ReadInputDataBit(KEY_GPIO_PORT, KEY3_GPIO_PIN)) cur = 3;
    else if (!GPIO_ReadInputDataBit(KEY_GPIO_PORT, KEY4_GPIO_PIN)) cur = 4;

    if (cur != 0) {
        if (cur != held_key) {
            /* 新按键按下 */
            held_key   = cur;
            hold_cnt   = 1;
            long_fired = 0;
        } else {
            hold_cnt++;
            /* 长按事件：KEY1 或 KEY3，且尚未触发过 */
            if (!long_fired && hold_cnt >= LONG_PRESS_TICKS) {
                long_fired = 1;
                if (cur == 1) return KEY1_LONG;
                if (cur == 3) return KEY3_LONG;
            }
        }
    } else {
        /* 按键已释放 */
        if (held_key != 0 && !long_fired && hold_cnt >= 2) {
            /* 短按：未触发过长按且持续超过消抖（≥2次扫描≈20ms） */
            uint8_t k = held_key;
            held_key   = 0;
            hold_cnt   = 0;
            long_fired = 0;
            if (k == 1) return KEY1_SHORT;
            if (k == 2) return KEY2_SHORT;
            if (k == 3) return KEY3_SHORT;
            if (k == 4) return KEY4_SHORT;
        }
        held_key   = 0;
        hold_cnt   = 0;
        long_fired = 0;
    }

    return KEY_NONE;
}
