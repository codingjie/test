#ifndef __APP_TASKS_H
#define __APP_TASKS_H

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

/* 工作模式定义 */
#define MODE_MANUAL         0    // 手动模式
#define MODE_ENERGY_SAVE    1    // 节能模式
#define MODE_AUTO           2    // 自动模式

/* UI界面模式定义 */
#define UI_MODE_MANUAL      0    // 手动模式界面
#define UI_MODE_ENERGY      1    // 节能模式界面
#define UI_MODE_AUTO        2    // 自动模式界面
#define UI_MODE_ENV         3    // 环境监测显示界面
#define UI_MODE_SETTING     4    // 设置界面
#define UI_MODE_MAX         4    // 最大UI界面数

/* 阈值定义 */
#define HUMAN_DETECT_THRESHOLD      100  // 人体检测阈值距离(cm)
#define LIGHT_THRESHOLD_LOW         500  // 低光照阈值
#define LIGHT_THRESHOLD_HIGH        2000 // 高光照阈值

/* 全局变量 */
extern uint8_t g_current_ui_mode;      // 当前UI界面
extern uint8_t g_current_work_mode;    // 当前工作模式
extern uint16_t g_brightness;          // 当前亮度
extern uint8_t g_encoder_pressed;      // 编码器按键是否按下

/* 任务函数声明 */
void Task_Manual_Mode(void *pvParameters);      // 手动模式任务
void Task_Energy_Save_Mode(void *pvParameters); // 节能模式任务
void Task_Auto_Mode(void *pvParameters);        // 自动模式任务
void Task_Sitting_Reminder(void *pvParameters); // 久坐提醒任务
void Task_Environment_Monitor(void *pvParameters); // 环境监测任务
void Task_UI_Manager(void *pvParameters);       // UI管理任务

/* 界面显示函数声明 */
void UI_Display_Manual(void);           // 显示手动模式界面
void UI_Display_EnergySave(void);       // 显示节能模式界面
void UI_Display_Auto(void);             // 显示自动模式界面
void UI_Display_Environment(void);      // 显示环境界面
void UI_Display_Setting(void);          // 显示设置界面
void UI_Update_Display(void);           // 动态更新显示

#endif /* __APP_TASKS_H */
