#include "key.h"

/* PB10/PB11 下拉输入，按键按下时引脚拉高 */
void KEY_Init(void) {
    GPIO_InitTypeDef g;
    RCC_APB2PeriphClockCmd(KEY_CLK, ENABLE);
    g.GPIO_Pin   = KEY1_PIN | KEY2_PIN;
    g.GPIO_Mode  = GPIO_Mode_IPD;     /* 内部下拉 */
    g.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEY_PORT, &g);
}
