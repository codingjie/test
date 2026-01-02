#ifndef __APP_TASKS_H
#define __APP_TASKS_H

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

/* Work mode definitions */
#define MODE_MANUAL         0    // Manual mode
#define MODE_ENERGY_SAVE    1    // Energy save mode
#define MODE_AUTO           2    // Auto mode

/* UI mode definitions */
#define UI_MODE_MANUAL      0    // Manual mode UI
#define UI_MODE_ENERGY      1    // Energy save mode UI
#define UI_MODE_AUTO        2    // Auto mode UI
#define UI_MODE_ENV         3    // Environment monitor UI
#define UI_MODE_SETTING     4    // Settings UI
#define UI_MODE_MAX         4    // Max UI mode number

/* Threshold definitions */
#define HUMAN_DETECT_THRESHOLD      100  // Human detection threshold (cm)
#define LIGHT_THRESHOLD_LOW         500  // Low light threshold
#define LIGHT_THRESHOLD_HIGH        2000 // High light threshold

/* Global variables */
extern uint8_t g_current_ui_mode;      // Current UI mode
extern uint8_t g_current_work_mode;    // Current work mode
extern uint16_t g_brightness;          // Current brightness
extern uint8_t g_encoder_pressed;      // Encoder button pressed flag

/* Task function prototypes */
void Task_Manual_Mode(void *pvParameters);      // Manual mode task
void Task_Energy_Save_Mode(void *pvParameters); // Energy save mode task
void Task_Auto_Mode(void *pvParameters);        // Auto mode task
void Task_Sitting_Reminder(void *pvParameters); // Sitting reminder task
void Task_Environment_Monitor(void *pvParameters); // Environment monitor task
void Task_UI_Manager(void *pvParameters);       // UI manager task

/* UI display function prototypes */
void UI_Display_Manual(void);           // Display manual mode UI
void UI_Display_EnergySave(void);       // Display energy save mode UI
void UI_Display_Auto(void);             // Display auto mode UI
void UI_Display_Environment(void);      // Display environment UI
void UI_Display_Setting(void);          // Display settings UI
void UI_Update_Display(void);           // Update display dynamically

#endif /* __APP_TASKS_H */
