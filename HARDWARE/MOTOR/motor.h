#ifndef __MOTOR_H
#define __MOTOR_H
#include "sys.h"

// 初始化电机PWM和GPIO
void setupBrushlessMotor(void);

// 电机控制任务
void brushlessMotorTask(void *pdata);

// 初始化ADC采集电路
void setupMotorADC(void);

#endif
