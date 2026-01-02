#include "system_config.h"
#include "bsp_spi_flash.h"
#include <string.h>

/* 全局系统配置变量 */
SystemConfig_TypeDef g_system_config;

/**
 * @brief  初始化系统配置
 * @param  无
 * @retval 无
 */
void SystemConfig_Init(void) {
    SystemConfig_Load(&g_system_config);
}

/**
 * @brief  从Flash加载配置
 * @param  config: 指向配置结构体的指针
 * @retval 无
 */
void SystemConfig_Load(SystemConfig_TypeDef *config) {
    uint8_t buffer[sizeof(SystemConfig_TypeDef)];

    /* 从Flash读取配置 */
    SPI_FLASH_BufferRead(buffer, SYSTEM_CONFIG_FLASH_ADDR, sizeof(SystemConfig_TypeDef));

    /* 复制数据到结构体 */
    memcpy(config, buffer, sizeof(SystemConfig_TypeDef));

    /* 校验魔术数，如果无效则使用默认配置 */
    if (config->magic_number != CONFIG_MAGIC_NUMBER) {
        SystemConfig_SetDefault(config);
        SystemConfig_Save(config);
    }
}

/**
 * @brief  保存配置到Flash
 * @param  config: 指向配置结构体的指针
 * @retval 无
 */
void SystemConfig_Save(SystemConfig_TypeDef *config) {
    uint8_t buffer[sizeof(SystemConfig_TypeDef)];

    /* 设置魔术数 */
    config->magic_number = CONFIG_MAGIC_NUMBER;

    /* 复制数据到缓冲区 */
    memcpy(buffer, config, sizeof(SystemConfig_TypeDef));

    /* 擦除扇区并写入Flash */
    SPI_FLASH_SectorErase(SYSTEM_CONFIG_FLASH_ADDR);
    SPI_FLASH_BufferWrite(buffer, SYSTEM_CONFIG_FLASH_ADDR, sizeof(SystemConfig_TypeDef));
}

/**
 * @brief  设置默认配置
 * @param  config: 指向配置结构体的指针
 * @retval 无
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
