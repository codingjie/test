#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"

/* KEY GPIO 定义 (GPIOB) */
#define KEY1_GPIO_PIN   GPIO_Pin_2   /* SET     */
#define KEY2_GPIO_PIN   GPIO_Pin_3   /* INCREASE */
#define KEY3_GPIO_PIN   GPIO_Pin_4   /* DECREASE */
#define KEY4_GPIO_PIN   GPIO_Pin_5   /* CONFIRM  */
#define KEY_GPIO_PORT   GPIOB
#define KEY_GPIO_CLK    RCC_APB2Periph_GPIOB

/* 按键事件返回值 */
#define KEY_NONE        0
#define KEY1_SHORT      1   /* SET     短按：循环选中参数 */
#define KEY1_LONG       2   /* SET     长按：切换自动/手动 */
#define KEY2_SHORT      3   /* INCREASE 短按 */
#define KEY3_SHORT      4   /* DECREASE 短按 */
#define KEY3_LONG       5   /* DECREASE 长按：切换报警开/关 */
#define KEY4_SHORT      6   /* CONFIRM  短按 */

/* 长按判定：函数每 10 ms 调用一次，100次 = 1000 ms */
#define LONG_PRESS_TICKS  100

void KEY_GPIO_Config(void);
uint8_t KEY_Scan(void);

#endif
