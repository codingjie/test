#ifndef __APP_TASKS_H
#define __APP_TASKS_H

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

/* ����ģʽ���� */
#define MODE_MANUAL         0    // �ֶ�ģʽ
#define MODE_ENERGY_SAVE    1    // �ڽ�ģʽ
#define MODE_AUTO           2    // �Զ�ģʽ

/* UI��ģʽ���� */
#define UI_MODE_MANUAL      0    // �ֶ�ģʽ����
#define UI_MODE_ENERGY      1    // �ڽ�ģʽ����
#define UI_MODE_AUTO        2    // �Զ�ģʽ����
#define UI_MODE_ENV         3    // ��������ʾ����
#define UI_MODE_SETTING     4    // ��������
#define UI_MODE_MAX         4    // ���UI����

/* �����ֵ */
#define HUMAN_DETECT_THRESHOLD      100  // �˴���ֵⷧ(cm)
#define LIGHT_THRESHOLD_LOW         500  // ��������ֵ
#define LIGHT_THRESHOLD_HIGH        2000 // ��������ֵ

/* ȫ�ֱ��� */
extern uint8_t g_current_ui_mode;      // ��ǰUI����
extern uint8_t g_current_work_mode;    // ��ǰ����ģʽ
extern uint16_t g_brightness;          // ��ǰ����
extern uint8_t g_encoder_pressed;      // ��������Ƿ����

/* �������� */
void Task_Manual_Mode(void *pvParameters);      // �ֶ�ģʽ����
void Task_Energy_Save_Mode(void *pvParameters); // �ڽ�ģʽ����
void Task_Auto_Mode(void *pvParameters);        // �Զ�ģʽ����
void Task_Sitting_Reminder(void *pvParameters); // �ò���ʾ����
void Task_Environment_Monitor(void *pvParameters); // �������ⷴ����
void Task_UI_Manager(void *pvParameters);       // UI��������

/* ���������� */
void UI_Display_Manual(void);           // ��ʾ�ֶ�ģʽ����
void UI_Display_EnergySave(void);       // ��ʾ�ڽ�ģʽ����
void UI_Display_Auto(void);             // ��ʾ�Զ�ģʽ����
void UI_Display_Environment(void);      // ��ʾ��������
void UI_Display_Setting(void);          // ��ʾ��������
void UI_Update_Display(void);           // ��̬����ʾ

#endif /* __APP_TASKS_H */
