#ifndef __SYSTEM_CONFIG_H
#define __SYSTEM_CONFIG_H

#include "stm32f4xx.h"

/* ϵͳ���ýṹ�� */
typedef struct {
    uint8_t work_mode;              // ����ģʽ��0-�ֶ���1-�ڽڡ�2-�Զ���
    uint8_t brightness;             // ���ȣ�0-99
    uint8_t sitting_reminder_enable; // �ò���ʾʹ��
    uint16_t sitting_time_threshold; // �ò�ʱ���ֵ������λ�룩
    uint16_t sitting_distance;       // �ò��������־ࣨ��λcm��
    uint8_t auto_mode_sensitivity;   // �Զ�ģʽ���м�
    uint32_t magic_number;           // ħ�������ڼ�����Ч��
} SystemConfig_TypeDef;

/* Flash��ַ���� */
#define SYSTEM_CONFIG_FLASH_ADDR    0x000000  // ϵͳ���õ�ַ
#define CONFIG_MAGIC_NUMBER         0x12345678 // ��֤ħ����

/* Ĭ������ */
#define DEFAULT_WORK_MODE           0    // Ĭ���ֶ�ģʽ
#define DEFAULT_BRIGHTNESS          20   // Ĭ������20
#define DEFAULT_SITTING_ENABLE      1    // Ĭ�Ͽ��ò���ʾ
#define DEFAULT_SITTING_TIME        1800 // Ĭ�ϳ���30����
#define DEFAULT_SITTING_DISTANCE    50   // Ĭ�Ͼ���50cm
#define DEFAULT_AUTO_SENSITIVITY    50   // Ĭ�����м�50

/* ��������� */
void SystemConfig_Init(void);
void SystemConfig_Save(SystemConfig_TypeDef *config);
void SystemConfig_Load(SystemConfig_TypeDef *config);
void SystemConfig_SetDefault(SystemConfig_TypeDef *config);

extern SystemConfig_TypeDef g_system_config;

#endif /* __SYSTEM_CONFIG_H */
