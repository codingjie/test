#ifndef __SYSTEM_CONFIG_H
#define __SYSTEM_CONFIG_H

#include "stm32f4xx.h"

/* System configuration structure */
typedef struct {
    uint8_t work_mode;              // Work mode: 0-manual, 1-energy save, 2-auto
    uint8_t brightness;             // Brightness: 0-99
    uint8_t sitting_reminder_enable; // Sitting reminder enable
    uint16_t sitting_time_threshold; // Sitting time threshold (seconds)
    uint16_t sitting_distance;       // Sitting distance detection (cm)
    uint8_t auto_mode_sensitivity;   // Auto mode sensitivity
    uint32_t magic_number;           // Magic number for validation
} SystemConfig_TypeDef;

/* Flash address definition */
#define SYSTEM_CONFIG_FLASH_ADDR    0x000000  // System config address
#define CONFIG_MAGIC_NUMBER         0x12345678 // Validation magic number

/* Default configuration */
#define DEFAULT_WORK_MODE           0    // Default manual mode
#define DEFAULT_BRIGHTNESS          20   // Default brightness 20
#define DEFAULT_SITTING_ENABLE      1    // Default sitting reminder on
#define DEFAULT_SITTING_TIME        1800 // Default sitting time 30 minutes
#define DEFAULT_SITTING_DISTANCE    50   // Default distance 50cm
#define DEFAULT_AUTO_SENSITIVITY    50   // Default sensitivity 50

/* Function prototypes */
void SystemConfig_Init(void);
void SystemConfig_Save(SystemConfig_TypeDef *config);
void SystemConfig_Load(SystemConfig_TypeDef *config);
void SystemConfig_SetDefault(SystemConfig_TypeDef *config);

extern SystemConfig_TypeDef g_system_config;

#endif /* __SYSTEM_CONFIG_H */
