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

/* ȫ�ֱ��� */
uint8_t g_current_ui_mode = UI_MODE_MANUAL;
uint8_t g_current_work_mode = MODE_MANUAL;
uint16_t g_brightness = 20;
uint8_t g_encoder_pressed = 0;

/* �ڲ����� */
static uint32_t sitting_timer = 0;          // �ò�ʱ�ƼƲ���

/**
 * @brief  �ֶ�ģʽ����
 * @param  pvParameters: �������
 * @retval ��
 */
void Task_Manual_Mode(void *pvParameters) {
    float cycle_count = 0;

    while(1) {
        if (g_current_work_mode == MODE_MANUAL) {
            /* ��ȡ������ֵ */
            Encoder_Get_Val(&cycle_count);

            /* ��������ת���������� */
            if(dirction_flag == POSITIVE_DIRECTION) {
                g_brightness++;
                if (g_brightness > 99) g_brightness = 99;
            } else if(dirction_flag == REVERSE_DIRECTION) {
                if (g_brightness > 0) g_brightness--;
            }

            /* ���������� */
            LED_SetRGB(g_brightness, g_brightness, g_brightness);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/**
 * @brief  �ڽ�ģʽ���񣨳�����������˵ƶ
 * @param  pvParameters: �������
 * @retval ��
 */
void Task_Energy_Save_Mode(void *pvParameters) {
    float distance = 0;
    uint8_t human_detected = 0;

    while(1) {
        if (g_current_work_mode == MODE_ENERGY_SAVE) {
            /* ��ȡ���� */
            CS100A_TRIG();
            vTaskDelay(pdMS_TO_TICKS(60));  // �ȴ�����
            distance = CS100A_GetDistance();

            /* �жϴˮ��ˣ����������Χ�ڣ� */
            if (distance > 5 && distance < g_system_config.sitting_distance) {
                human_detected = 1;
                g_brightness = 50;  // �˵ƶ�������50
            } else {
                human_detected = 0;
                g_brightness = 0;   // �˵�
            }

            /* ���������� */
            LED_SetRGB(g_brightness, g_brightness, g_brightness);
        }

        vTaskDelay(pdMS_TO_TICKS(500));  // ÿ500ms���һ��
    }
}

/**
 * @brief  �Զ�ģʽ���񣨸�ݹ⽳�Զ���������
 * @param  pvParameters: �������
 * @retval ��
 */
void Task_Auto_Mode(void *pvParameters) {
    uint16_t light_value = 0;
    uint16_t target_brightness = 0;

    while(1) {
        if (g_current_work_mode == MODE_AUTO) {
            /* ��ȡ����ֵ */
            light_value = PhotoResistor_GetValue();

            /* ���ݹ���ֵ����Ŀ����ȣ����ԽϰߣƷȽϰ� */
            if (light_value < LIGHT_THRESHOLD_LOW) {
                /* ������ */
                target_brightness = 80;
            } else if (light_value > LIGHT_THRESHOLD_HIGH) {
                /* ��������ȴ� */
                target_brightness = 20;
            } else {
                /* �н� */
                target_brightness = 50;
            }

            /* ƽ������ȱ仯 */
            if (g_brightness < target_brightness) {
                g_brightness++;
            } else if (g_brightness > target_brightness) {
                g_brightness--;
            }

            /* ���������� */
            LED_SetRGB(g_brightness, g_brightness, g_brightness);
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/**
 * @brief  �ò���ʾ����
 * @param  pvParameters: �������
 * @retval ��
 */
void Task_Sitting_Reminder(void *pvParameters) {
    float distance = 0;
    uint8_t is_sitting = 0;
    uint8_t beep_count = 0;

    sitting_timer = 0;

    while(1) {
        /* ֻ��ʹ��ʱ�Ż� */
        if (g_system_config.sitting_reminder_enable) {
            /* ��ȡ���� */
            CS100A_TRIG();
            vTaskDelay(pdMS_TO_TICKS(60));
            distance = CS100A_GetDistance();

            /* �жϴǲ��Ǿò� */
            if (distance > 5 && distance < g_system_config.sitting_distance) {
                /* �˴� */
                is_sitting = 1;
                sitting_timer++;

                /* �������ʱ���ޣ�����ʾ */
                if (sitting_timer >= g_system_config.sitting_time_threshold) {
                    /* ������3�� */
                    for (beep_count = 0; beep_count < 3; beep_count++) {
                        BEEP_ON();
                        vTaskDelay(pdMS_TO_TICKS(200));
                        BEEP_OFF();
                        vTaskDelay(pdMS_TO_TICKS(200));
                    }

                    /* �ؼ�ʱ */
                    sitting_timer = 0;
                }
            } else {
                /* ����� */
                is_sitting = 0;
                sitting_timer = 0;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));  // ÿ�����һ��
    }
}

/**
 * @brief  �������ⷴ����
 * @param  pvParameters: �������
 * @retval ��
 */
void Task_Environment_Monitor(void *pvParameters) {
    DHT11_Data_TypeDef dht11_data;

    while(1) {
        /* ��ȡ�¶�ʪ������ */
        Read_DHT11(&dht11_data);

        /* �������¶�ʪ�ȣ������Ҫ��ģ���е��� */

        vTaskDelay(pdMS_TO_TICKS(2000));  // ÿ2�����һ��
    }
}

/**
 * @brief  UI��������
 * @param  pvParameters: �������
 * @retval ��
 */
void Task_UI_Manager(void *pvParameters) {
    static uint8_t last_ui_mode = 0xFF;
    float cycle_count = 0;

    while(1) {
        /* ��ȡ������ֵ������ѡ�� */
        Encoder_Get_Val(&cycle_count);

        if(dirction_flag == POSITIVE_DIRECTION) {
            /* ��ת��ģ��+1 */
            g_current_ui_mode++;
            if (g_current_ui_mode > UI_MODE_MAX) {
                g_current_ui_mode = 0;
            }
        } else if(dirction_flag == REVERSE_DIRECTION) {
            /* ��ת��ģ��-1 */
            if (g_current_ui_mode == 0) {
                g_current_ui_mode = UI_MODE_MAX;
            } else {
                g_current_ui_mode--;
            }
        }

        /* ��⡱������������˵�ǰ�����ģʽ */
        if (g_encoder_pressed) {
            g_encoder_pressed = 0;

            /* ����UI�����л����ģʽ */
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

        /* UI���������ʾ��Ӧ���� */
        if (last_ui_mode != g_current_ui_mode) {
            last_ui_mode = g_current_ui_mode;
            OLED_CLS();  // ����

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

        /* ��̬����ʾ */
        UI_Update_Display();

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief  ��ʾ�ֶ�ģʽ����
 * @param  ��
 * @retval ��
 */
void UI_Display_Manual(void) {
    OLED_ShowStr(0, 0, (unsigned char *)"Mode:Manual", 2);
    OLED_ShowStr(0, 2, (unsigned char *)"Brightness:", 2);
}

/**
 * @brief  ��ʾ�ڽ�ģʽ����
 * @param  ��
 * @retval ��
 */
void UI_Display_EnergySave(void) {
    OLED_ShowStr(0, 0, (unsigned char *)"Mode:Energy", 2);
    OLED_ShowStr(0, 2, (unsigned char *)"Distance:", 2);
}

/**
 * @brief  ��ʾ�Զ�ģʽ����
 * @param  ��
 * @retval ��
 */
void UI_Display_Auto(void) {
    OLED_ShowStr(0, 0, (unsigned char *)"Mode:Auto", 2);
    OLED_ShowStr(0, 2, (unsigned char *)"Light:", 2);
}

/**
 * @brief  ��ʾ��������
 * @param  ��
 * @retval ��
 */
void UI_Display_Environment(void) {
    OLED_ShowStr(0, 0, (unsigned char *)"Environment", 2);
    OLED_ShowStr(0, 2, (unsigned char *)"Temp:", 2);
    OLED_ShowStr(0, 4, (unsigned char *)"Humi:", 2);
}

/**
 * @brief  ��ʾ��������
 * @param  ��
 * @retval ��
 */
void UI_Display_Setting(void) {
    OLED_ShowStr(0, 0, (unsigned char *)"Settings", 2);
    OLED_ShowStr(0, 2, (unsigned char *)"Sitting:", 2);
}

/**
 * @brief  ��̬����ʾ
 * @param  ��
 * @retval ��
 */
void UI_Update_Display(void) {
    char str_buf[16];
    DHT11_Data_TypeDef dht11_data;
    float distance = 0;
    uint16_t light_value = 0;

    switch(g_current_ui_mode) {
        case UI_MODE_MANUAL:
            /* ��ʾ���� */
            sprintf(str_buf, "%3d  ", g_brightness);
            OLED_ShowStr(0, 4, (unsigned char *)str_buf, 2);
            break;

        case UI_MODE_ENERGY:
            /* ��ʾ���� */
            distance = CS100A_GetDistance();
            sprintf(str_buf, "%3.1fcm ", distance);
            OLED_ShowStr(0, 4, (unsigned char *)str_buf, 2);
            break;

        case UI_MODE_AUTO:
            /* ��ʾ����ֵ */
            light_value = PhotoResistor_GetValue();
            sprintf(str_buf, "%4d  ", light_value);
            OLED_ShowStr(0, 4, (unsigned char *)str_buf, 2);
            break;

        case UI_MODE_ENV:
            /* ��ʾ�¶�ʪ�� */
            if (Read_DHT11(&dht11_data) == 0) {
                sprintf(str_buf, "%2dC  ", dht11_data.temp_int);
                OLED_ShowStr(60, 2, (unsigned char *)str_buf, 2);
                sprintf(str_buf, "%2d%%  ", dht11_data.humi_int);
                OLED_ShowStr(60, 4, (unsigned char *)str_buf, 2);
            }
            break;

        case UI_MODE_SETTING:
            /* ��ʾ�ò�����״̬ */
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
