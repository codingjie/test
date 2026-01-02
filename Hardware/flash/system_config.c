#include "system_config.h"
#include "bsp_spi_flash.h"
#include <string.h>

/* ȫ��ϵͳ������ */
SystemConfig_TypeDef g_system_config;

/**
 * @brief  ϵͳ���ó�ʼ��
 * @param  ��
 * @retval ��
 */
void SystemConfig_Init(void) {
    SystemConfig_Load(&g_system_config);
}

/**
 * @brief  ���ش�Flash
 * @param  config: ָ����������ָ��
 * @retval ��
 */
void SystemConfig_Load(SystemConfig_TypeDef *config) {
    uint8_t buffer[sizeof(SystemConfig_TypeDef)];

    /* ��Flash�ж�ȡ���� */
    SPI_FLASH_BufferRead(buffer, SYSTEM_CONFIG_FLASH_ADDR, sizeof(SystemConfig_TypeDef));

    /* ������ݵ�����ṹ�� */
    memcpy(config, buffer, sizeof(SystemConfig_TypeDef));

    /* У��ħ����,�����Ч��ʹ��Ĭ������ */
    if (config->magic_number != CONFIG_MAGIC_NUMBER) {
        SystemConfig_SetDefault(config);
        SystemConfig_Save(config);
    }
}

/**
 * @brief  ����������Flash
 * @param  config: ָ����������ָ��
 * @retval ��
 */
void SystemConfig_Save(SystemConfig_TypeDef *config) {
    uint8_t buffer[sizeof(SystemConfig_TypeDef)];

    /* ����ħ���� */
    config->magic_number = CONFIG_MAGIC_NUMBER;

    /* ���������ݸ��Ƶ�����������д��Flash */
    memcpy(buffer, config, sizeof(SystemConfig_TypeDef));

    /* ����Flash������д�� */
    SPI_FLASH_SectorErase(SYSTEM_CONFIG_FLASH_ADDR);
    SPI_FLASH_BufferWrite(buffer, SYSTEM_CONFIG_FLASH_ADDR, sizeof(SystemConfig_TypeDef));
}

/**
 * @brief  ����Ĭ������
 * @param  config: ָ����������ָ��
 * @retval ��
 */
void SystemConfig_SetDefault(SystemConfig_TypeDef *config) {
    config->work_mode = DEFAULT_WORK_MODE;
    config->brightness = DEFAULT_BRIGHTNESS;
    config->sitting_reminder_enable = DEFAULT_SITTING_ENABLE;
    config->sitting_time_threshold = DEFAULT_SITTING_TIME;
    config->sitting_distance = DEFAULT_SITTING_DISTANCE;
    config->auto_mode_sensitivity = DEFAULT_AUTO_SENSITIVITY;
    config->magic_number = CONFIG_MAGIC_NUMBER;
}
