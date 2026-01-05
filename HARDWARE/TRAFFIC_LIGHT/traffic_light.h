#ifndef __TRAFFIC_LIGHT_H
#define __TRAFFIC_LIGHT_H

#include "stm32f10x.h"
#include "led.h"

// 交通灯状态定义
typedef enum {
    STATE_SN_GREEN_EW_RED,      // 南北绿灯，东西红灯
    STATE_SN_YELLOW_EW_RED,     // 南北黄灯，东西红灯
    STATE_SN_RED_EW_GREEN,      // 南北红灯，东西绿灯
    STATE_SN_RED_EW_YELLOW      // 南北红灯，东西黄灯
} TrafficLightState;

// 时间定义（秒）
#define GREEN_TIME   12
#define YELLOW_TIME  3
#define RED_TIME     15

// 黄灯闪烁周期（毫秒）
#define YELLOW_BLINK_PERIOD 500

// 全局变量声明
extern TrafficLightState current_state;
extern volatile uint16_t state_timer;      // 当前状态剩余时间（秒）
extern volatile uint8_t yellow_blink_flag; // 黄灯闪烁标志

// 函数声明
void TrafficLight_Init(void);
void TrafficLight_StateMachine(void);
void TrafficLight_UpdateLights(uint8_t blink_phase);

#endif
