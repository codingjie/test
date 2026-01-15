#ifndef __KEY_H
#define __KEY_H
#include "sys.h"

// 按键事件标志定义
#define KEY1_FLAG 0x01  // KEY1按键事件 - 电机正转
#define KEY2_FLAG 0x02  // KEY2按键事件 - 电机反转
#define KEY3_FLAG 0x04  // KEY3按键事件 - 电机停止

// 初始化按键GPIO
void setupKeyboard(void);

#endif
