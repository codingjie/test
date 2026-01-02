#include "system_config.h"
#include "bsp_spi_flash.h"
#include <string.h>

/* Global system configuration */
SystemConfig_TypeDef g_system_config;

/**
 * @brief  Initialize system configuration
 * @param  None
 * @retval None
 */
void SystemConfig_Init(void) {
    SystemConfig_Load(&g_system_config);
}

/**
 * @brief  Load configuration from Flash
 * @param  config: Pointer to configuration structure
 * @retval None
 */
void SystemConfig_Load(SystemConfig_TypeDef *config) {
    uint8_t buffer[sizeof(SystemConfig_TypeDef)];

    /* Read configuration from Flash */
    SPI_FLASH_BufferRead(buffer, SYSTEM_CONFIG_FLASH_ADDR, sizeof(SystemConfig_TypeDef));

    /* Copy data to structure */
    memcpy(config, buffer, sizeof(SystemConfig_TypeDef));

    /* Validate magic number, use default if invalid */
    if (config->magic_number != CONFIG_MAGIC_NUMBER) {
        SystemConfig_SetDefault(config);
        SystemConfig_Save(config);
    }
}

/**
 * @brief  Save configuration to Flash
 * @param  config: Pointer to configuration structure
 * @retval None
 */
void SystemConfig_Save(SystemConfig_TypeDef *config) {
    uint8_t buffer[sizeof(SystemConfig_TypeDef)];

    /* Set magic number */
    config->magic_number = CONFIG_MAGIC_NUMBER;

    /* Copy data to buffer */
    memcpy(buffer, config, sizeof(SystemConfig_TypeDef));

    /* Erase sector and write to Flash */
    SPI_FLASH_SectorErase(SYSTEM_CONFIG_FLASH_ADDR);
    SPI_FLASH_BufferWrite(buffer, SYSTEM_CONFIG_FLASH_ADDR, sizeof(SystemConfig_TypeDef));
}

/**
 * @brief  Set default configuration
 * @param  config: Pointer to configuration structure
 * @retval None
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
