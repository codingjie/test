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

/* 全局变量 */
uint8_t g_current_ui_mode = UI_MODE_MANUAL;
uint8_t g_current_work_mode = MODE_MANUAL;
uint16_t g_brightness = 20;
uint8_t g_encoder_pressed = 0;

/* 内部变量 */
static uint32_t sitting_timer = 0;          // 久坐计时器计数（秒）

/**
 * @brief  手动模式任务
 * @param  pvParameters: 任务参数
 * @retval 无
 */
void Task_Manual_Mode(void *pvParameters) {
    float cycle_count = 0;

    while(1) {
        if (g_current_work_mode == MODE_MANUAL) {
            /* 获取编码器值 */
            Encoder_Get_Val(&cycle_count);

            /* 根据编码器旋转调整亮度 */
            if(dirction_flag == POSITIVE_DIRECTION) {
                g_brightness++;
                if (g_brightness > 99) g_brightness = 99;
            } else if(dirction_flag == REVERSE_DIRECTION) {
                if (g_brightness > 0) g_brightness--;
            }

            /* 设置LED亮度 */
            LED_SetRGB(g_brightness, g_brightness, g_brightness);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/**
 * @brief  节能模式任务（超声波人体检测）
 * @param  pvParameters: 任务参数
 * @retval 无
 */
void Task_Energy_Save_Mode(void *pvParameters) {
    float distance = 0;
    uint8_t human_detected = 0;

    while(1) {
        if (g_current_work_mode == MODE_ENERGY_SAVE) {
            /* 触发超声波传感器 */
            CS100A_TRIG();
            vTaskDelay(pdMS_TO_TICKS(60));  // 等待测量
            distance = CS100A_GetDistance();

            /* 判断是否检测到人（在有效范围内） */
            if (distance > 5 && distance < g_system_config.sitting_distance) {
                human_detected = 1;
                g_brightness = 50;  // 人在灯亮，亮度设为50
            } else {
                human_detected = 0;
                g_brightness = 0;   // 人走灯灭
            }

            /* 设置LED亮度 */
            LED_SetRGB(g_brightness, g_brightness, g_brightness);
        }

        vTaskDelay(pdMS_TO_TICKS(500));  // 每500ms检测一次
    }
}

/**
 * @brief  自动模式任务（光敏传感器自动调光）
 * @param  pvParameters: 任务参数
 * @retval 无
 */
void Task_Auto_Mode(void *pvParameters) {
    uint16_t light_value = 0;
    uint16_t target_brightness = 0;

    while(1) {
        if (g_current_work_mode == MODE_AUTO) {
            /* 获取光照值 */
            light_value = PhotoResistor_GetValue();

            /* 根据光照值计算目标亮度（光照越暗，灯越亮） */
            if (light_value < LIGHT_THRESHOLD_LOW) {
                /* 环境较暗 */
                target_brightness = 80;
            } else if (light_value > LIGHT_THRESHOLD_HIGH) {
                /* 环境明亮度大 */
                target_brightness = 20;
            } else {
                /* 中等亮度 */
                target_brightness = 50;
            }

            /* 平滑过渡亮度变化 */
            if (g_brightness < target_brightness) {
                g_brightness++;
            } else if (g_brightness > target_brightness) {
                g_brightness--;
            }

            /* 设置LED亮度 */
            LED_SetRGB(g_brightness, g_brightness, g_brightness);
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/**
 * @brief  久坐提醒任务
 * @param  pvParameters: 任务参数
 * @retval 无
 */
void Task_Sitting_Reminder(void *pvParameters) {
    float distance = 0;
    uint8_t is_sitting = 0;
    uint8_t beep_count = 0;

    sitting_timer = 0;

    while(1) {
        /* 只在使能时才运行 */
        if (g_system_config.sitting_reminder_enable) {
            /* 获取距离 */
            CS100A_TRIG();
            vTaskDelay(pdMS_TO_TICKS(60));
            distance = CS100A_GetDistance();

            /* 判断是否久坐 */
            if (distance > 5 && distance < g_system_config.sitting_distance) {
                /* 人在此处 */
                is_sitting = 1;
                sitting_timer++;

                /* 检测是否超过时间阈值，进行提醒 */
                if (sitting_timer >= g_system_config.sitting_time_threshold) {
                    /* 蜂鸣器响3次 */
                    for (beep_count = 0; beep_count < 3; beep_count++) {
                        BEEP_ON();
                        vTaskDelay(pdMS_TO_TICKS(200));
                        BEEP_OFF();
                        vTaskDelay(pdMS_TO_TICKS(200));
                    }

                    /* 重置计时器 */
                    sitting_timer = 0;
                }
            } else {
                /* 无人在此 */
                is_sitting = 0;
                sitting_timer = 0;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));  // 每秒检测一次
    }
}

/**
 * @brief  环境监测任务
 * @param  pvParameters: 任务参数
 * @retval 无
 */
void Task_Environment_Monitor(void *pvParameters) {
    DHT11_Data_TypeDef dht11_data;

    while(1) {
        /* 读取温度湿度数据 */
        Read_DHT11(&dht11_data);

        /* 存储环境数据供UI模块显示使用 */

        vTaskDelay(pdMS_TO_TICKS(2000));  // 每2秒读取一次
    }
}

/**
 * @brief  UI管理任务
 * @param  pvParameters: 任务参数
 * @retval 无
 */
void Task_UI_Manager(void *pvParameters) {
    static uint8_t last_ui_mode = 0xFF;
    float cycle_count = 0;

    while(1) {
        /* 获取编码器值用于界面选择 */
        Encoder_Get_Val(&cycle_count);

        if(dirction_flag == POSITIVE_DIRECTION) {
            /* 右转：界面模式+1 */
            g_current_ui_mode++;
            if (g_current_ui_mode > UI_MODE_MAX) {
                g_current_ui_mode = 0;
            }
        } else if(dirction_flag == REVERSE_DIRECTION) {
            /* 左转：界面模式-1 */
            if (g_current_ui_mode == 0) {
                g_current_ui_mode = UI_MODE_MAX;
            } else {
                g_current_ui_mode--;
            }
        }

        /* 检测编码器按键按下，进入当前界面的工作模式 */
        if (g_encoder_pressed) {
            g_encoder_pressed = 0;

            /* 根据UI界面切换工作模式 */
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

        /* UI界面改变时显示对应界面 */
        if (last_ui_mode != g_current_ui_mode) {
            last_ui_mode = g_current_ui_mode;
            OLED_CLS();  // 清屏

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

        /* 动态更新显示 */
        UI_Update_Display();

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief  显示手动模式界面
 * @param  无
 * @retval 无
 */
void UI_Display_Manual(void) {
    OLED_ShowStr(0, 0, (unsigned char *)"Mode:Manual", 2);
    OLED_ShowStr(0, 2, (unsigned char *)"Brightness:", 2);
}

/**
 * @brief  显示节能模式界面
 * @param  无
 * @retval 无
 */
void UI_Display_EnergySave(void) {
    OLED_ShowStr(0, 0, (unsigned char *)"Mode:Energy", 2);
    OLED_ShowStr(0, 2, (unsigned char *)"Distance:", 2);
}

/**
 * @brief  显示自动模式界面
 * @param  无
 * @retval 无
 */
void UI_Display_Auto(void) {
    OLED_ShowStr(0, 0, (unsigned char *)"Mode:Auto", 2);
    OLED_ShowStr(0, 2, (unsigned char *)"Light:", 2);
}

/**
 * @brief  显示环境界面
 * @param  无
 * @retval 无
 */
void UI_Display_Environment(void) {
    OLED_ShowStr(0, 0, (unsigned char *)"Environment", 2);
    OLED_ShowStr(0, 2, (unsigned char *)"Temp:", 2);
    OLED_ShowStr(0, 4, (unsigned char *)"Humi:", 2);
}

/**
 * @brief  显示设置界面
 * @param  无
 * @retval 无
 */
void UI_Display_Setting(void) {
    OLED_ShowStr(0, 0, (unsigned char *)"Settings", 2);
    OLED_ShowStr(0, 2, (unsigned char *)"Sitting:", 2);
}

/**
 * @brief  动态更新显示
 * @param  无
 * @retval 无
 */
void UI_Update_Display(void) {
    char str_buf[16];
    DHT11_Data_TypeDef dht11_data;
    float distance = 0;
    uint16_t light_value = 0;

    switch(g_current_ui_mode) {
        case UI_MODE_MANUAL:
            /* 显示亮度 */
            sprintf(str_buf, "%3d  ", g_brightness);
            OLED_ShowStr(0, 4, (unsigned char *)str_buf, 2);
            break;

        case UI_MODE_ENERGY:
            /* 显示距离 */
            distance = CS100A_GetDistance();
            sprintf(str_buf, "%3.1fcm ", distance);
            OLED_ShowStr(0, 4, (unsigned char *)str_buf, 2);
            break;

        case UI_MODE_AUTO:
            /* 显示光照值 */
            light_value = PhotoResistor_GetValue();
            sprintf(str_buf, "%4d  ", light_value);
            OLED_ShowStr(0, 4, (unsigned char *)str_buf, 2);
            break;

        case UI_MODE_ENV:
            /* 显示温度湿度 */
            if (Read_DHT11(&dht11_data) == 0) {
                sprintf(str_buf, "%2dC  ", dht11_data.temp_int);
                OLED_ShowStr(60, 2, (unsigned char *)str_buf, 2);
                sprintf(str_buf, "%2d%%  ", dht11_data.humi_int);
                OLED_ShowStr(60, 4, (unsigned char *)str_buf, 2);
            }
            break;

        case UI_MODE_SETTING:
            /* 显示久坐提醒状态 */
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
