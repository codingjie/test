#include "app_tasks.h"
#include "system_config.h"
#include "bsp_led.h"
#include "bsp_oled_debug.h"
#include "bsp_cs100a.h"
#include "bsp_photoresistor.h"
#include "bsp_dht11.h"
#include "bsp_beep.h"
#include "bsp_timer_encoder.h"
#include <stdio.h>

/* Global variables */
uint8_t g_current_ui_mode = UI_MODE_MANUAL;
uint8_t g_current_work_mode = MODE_MANUAL;
uint16_t g_brightness = 20;
uint8_t g_encoder_pressed = 0;

/* Internal variables */
static uint32_t sitting_timer = 0;          // Sitting time counter (seconds)

/**
 * @brief  Manual mode task
 * @param  pvParameters: Task parameters
 * @retval None
 */
void Task_Manual_Mode(void *pvParameters) {
    float cycle_count = 0;

    while(1) {
        if (g_current_work_mode == MODE_MANUAL) {
            /* Get encoder value */
            Encoder_Get_Val(&cycle_count);

            /* Adjust brightness based on encoder rotation */
            if(dirction_flag == POSITIVE_DIRECTION) {
                g_brightness++;
                if (g_brightness > 99) g_brightness = 99;
            } else if(dirction_flag == REVERSE_DIRECTION) {
                if (g_brightness > 0) g_brightness--;
            }

            /* Set LED brightness */
            LED_SetRGB(g_brightness, g_brightness, g_brightness);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/**
 * @brief  Energy save mode task (ultrasonic detection)
 * @param  pvParameters: Task parameters
 * @retval None
 */
void Task_Energy_Save_Mode(void *pvParameters) {
    float distance = 0;
    uint8_t human_detected = 0;

    while(1) {
        if (g_current_work_mode == MODE_ENERGY_SAVE) {
            /* Trigger ultrasonic sensor */
            CS100A_TRIG();
            vTaskDelay(pdMS_TO_TICKS(60));  // Wait for measurement
            distance = CS100A_GetDistance();

            /* Check if human is detected (within range) */
            if (distance > 5 && distance < g_system_config.sitting_distance) {
                human_detected = 1;
                g_brightness = 50;  // Light on with brightness 50
            } else {
                human_detected = 0;
                g_brightness = 0;   // Light off
            }

            /* Set LED brightness */
            LED_SetRGB(g_brightness, g_brightness, g_brightness);
        }

        vTaskDelay(pdMS_TO_TICKS(500));  // Check every 500ms
    }
}

/**
 * @brief  Auto mode task (light sensor auto adjustment)
 * @param  pvParameters: Task parameters
 * @retval None
 */
void Task_Auto_Mode(void *pvParameters) {
    uint16_t light_value = 0;
    uint16_t target_brightness = 0;

    while(1) {
        if (g_current_work_mode == MODE_AUTO) {
            /* Get light sensor value */
            light_value = PhotoResistor_GetValue();

            /* Calculate target brightness based on ambient light */
            if (light_value < LIGHT_THRESHOLD_LOW) {
                /* Dark environment */
                target_brightness = 80;
            } else if (light_value > LIGHT_THRESHOLD_HIGH) {
                /* Bright environment */
                target_brightness = 20;
            } else {
                /* Medium brightness */
                target_brightness = 50;
            }

            /* Smooth brightness transition */
            if (g_brightness < target_brightness) {
                g_brightness++;
            } else if (g_brightness > target_brightness) {
                g_brightness--;
            }

            /* Set LED brightness */
            LED_SetRGB(g_brightness, g_brightness, g_brightness);
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/**
 * @brief  Sitting reminder task
 * @param  pvParameters: Task parameters
 * @retval None
 */
void Task_Sitting_Reminder(void *pvParameters) {
    float distance = 0;
    uint8_t is_sitting = 0;
    uint8_t beep_count = 0;

    sitting_timer = 0;

    while(1) {
        /* Only run when enabled */
        if (g_system_config.sitting_reminder_enable) {
            /* Get distance */
            CS100A_TRIG();
            vTaskDelay(pdMS_TO_TICKS(60));
            distance = CS100A_GetDistance();

            /* Check if person is sitting */
            if (distance > 5 && distance < g_system_config.sitting_distance) {
                /* Person detected */
                is_sitting = 1;
                sitting_timer++;

                /* Check if sitting time threshold exceeded */
                if (sitting_timer >= g_system_config.sitting_time_threshold) {
                    /* Beep 3 times to remind */
                    for (beep_count = 0; beep_count < 3; beep_count++) {
                        BEEP_ON();
                        vTaskDelay(pdMS_TO_TICKS(200));
                        BEEP_OFF();
                        vTaskDelay(pdMS_TO_TICKS(200));
                    }

                    /* Reset timer */
                    sitting_timer = 0;
                }
            } else {
                /* No person detected */
                is_sitting = 0;
                sitting_timer = 0;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));  // Check every second
    }
}

/**
 * @brief  Environment monitor task
 * @param  pvParameters: Task parameters
 * @retval None
 */
void Task_Environment_Monitor(void *pvParameters) {
    DHT11_Data_TypeDef dht11_data;

    while(1) {
        /* Read temperature and humidity */
        Read_DHT11(&dht11_data);

        /* Store environment data for display in UI */

        vTaskDelay(pdMS_TO_TICKS(2000));  // Read every 2 seconds
    }
}

/**
 * @brief  UI manager task
 * @param  pvParameters: Task parameters
 * @retval None
 */
void Task_UI_Manager(void *pvParameters) {
    static uint8_t last_ui_mode = 0xFF;
    float cycle_count = 0;

    while(1) {
        /* Get encoder value for UI mode selection */
        Encoder_Get_Val(&cycle_count);

        if(dirction_flag == POSITIVE_DIRECTION) {
            /* Rotate right: UI mode + 1 */
            g_current_ui_mode++;
            if (g_current_ui_mode > UI_MODE_MAX) {
                g_current_ui_mode = 0;
            }
        } else if(dirction_flag == REVERSE_DIRECTION) {
            /* Rotate left: UI mode - 1 */
            if (g_current_ui_mode == 0) {
                g_current_ui_mode = UI_MODE_MAX;
            } else {
                g_current_ui_mode--;
            }
        }

        /* Check if encoder button is pressed to enter selected mode */
        if (g_encoder_pressed) {
            g_encoder_pressed = 0;

            /* Switch work mode based on current UI mode */
            switch(g_current_ui_mode) {
                case UI_MODE_MANUAL:
                    g_current_work_mode = MODE_MANUAL;
                    g_system_config.work_mode = MODE_MANUAL;
                    break;

                case UI_MODE_ENERGY:
                    g_current_work_mode = MODE_ENERGY_SAVE;
                    g_system_config.work_mode = MODE_ENERGY_SAVE;
                    break;

                case UI_MODE_AUTO:
                    g_current_work_mode = MODE_AUTO;
                    g_system_config.work_mode = MODE_AUTO;
                    break;

                default:
                    break;
            }
        }

        /* Update UI display when mode changes */
        if (last_ui_mode != g_current_ui_mode) {
            last_ui_mode = g_current_ui_mode;
            OLED_CLS();  // Clear screen

            switch(g_current_ui_mode) {
                case UI_MODE_MANUAL:
                    UI_Display_Manual();
                    break;

                case UI_MODE_ENERGY:
                    UI_Display_EnergySave();
                    break;

                case UI_MODE_AUTO:
                    UI_Display_Auto();
                    break;

                case UI_MODE_ENV:
                    UI_Display_Environment();
                    break;

                case UI_MODE_SETTING:
                    UI_Display_Setting();
                    break;

                default:
                    break;
            }
        }

        /* Dynamic display update */
        UI_Update_Display();

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief  Display manual mode UI
 * @param  None
 * @retval None
 */
void UI_Display_Manual(void) {
    OLED_ShowStr(0, 0, (unsigned char *)"Mode:Manual", 2);
    OLED_ShowStr(0, 2, (unsigned char *)"Brightness:", 2);
}

/**
 * @brief  Display energy save mode UI
 * @param  None
 * @retval None
 */
void UI_Display_EnergySave(void) {
    OLED_ShowStr(0, 0, (unsigned char *)"Mode:Energy", 2);
    OLED_ShowStr(0, 2, (unsigned char *)"Distance:", 2);
}

/**
 * @brief  Display auto mode UI
 * @param  None
 * @retval None
 */
void UI_Display_Auto(void) {
    OLED_ShowStr(0, 0, (unsigned char *)"Mode:Auto", 2);
    OLED_ShowStr(0, 2, (unsigned char *)"Light:", 2);
}

/**
 * @brief  Display environment UI
 * @param  None
 * @retval None
 */
void UI_Display_Environment(void) {
    OLED_ShowStr(0, 0, (unsigned char *)"Environment", 2);
    OLED_ShowStr(0, 2, (unsigned char *)"Temp:", 2);
    OLED_ShowStr(0, 4, (unsigned char *)"Humi:", 2);
}

/**
 * @brief  Display settings UI
 * @param  None
 * @retval None
 */
void UI_Display_Setting(void) {
    OLED_ShowStr(0, 0, (unsigned char *)"Settings", 2);
    OLED_ShowStr(0, 2, (unsigned char *)"Sitting:", 2);
}

/**
 * @brief  Update display dynamically
 * @param  None
 * @retval None
 */
void UI_Update_Display(void) {
    char str_buf[16];
    DHT11_Data_TypeDef dht11_data;
    float distance = 0;
    uint16_t light_value = 0;

    switch(g_current_ui_mode) {
        case UI_MODE_MANUAL:
            /* Display brightness */
            sprintf(str_buf, "%3d  ", g_brightness);
            OLED_ShowStr(0, 4, (unsigned char *)str_buf, 2);
            break;

        case UI_MODE_ENERGY:
            /* Display distance */
            distance = CS100A_GetDistance();
            sprintf(str_buf, "%3.1fcm ", distance);
            OLED_ShowStr(0, 4, (unsigned char *)str_buf, 2);
            break;

        case UI_MODE_AUTO:
            /* Display light value */
            light_value = PhotoResistor_GetValue();
            sprintf(str_buf, "%4d  ", light_value);
            OLED_ShowStr(0, 4, (unsigned char *)str_buf, 2);
            break;

        case UI_MODE_ENV:
            /* Display temperature and humidity */
            if (Read_DHT11(&dht11_data) == 0) {
                sprintf(str_buf, "%2dC  ", dht11_data.temp_int);
                OLED_ShowStr(60, 2, (unsigned char *)str_buf, 2);
                sprintf(str_buf, "%2d%%  ", dht11_data.humi_int);
                OLED_ShowStr(60, 4, (unsigned char *)str_buf, 2);
            }
            break;

        case UI_MODE_SETTING:
            /* Display sitting reminder status */
            if (g_system_config.sitting_reminder_enable) {
                OLED_ShowStr(80, 2, (unsigned char *)"ON ", 2);
            } else {
                OLED_ShowStr(80, 2, (unsigned char *)"OFF", 2);
            }
            break;

        default:
            break;
    }
}
