#ifndef __SYSTEM_CONFIG_H
#define __SYSTEM_CONFIG_H

#include "stm32f4xx.h"

/* 系统配置结构体 */
typedef struct {
    uint8_t work_mode;              // 工作模式：0-手动，1-节能，2-自动
    uint8_t brightness;             // 亮度：0-99
    uint8_t sitting_reminder_enable; // 久坐提醒使能
    uint16_t sitting_time_threshold; // 久坐时间阈值（单位秒）
    uint16_t sitting_distance;       // 久坐检测距离（单位cm）
    uint8_t auto_mode_sensitivity;   // 自动模式灵敏度
    uint32_t magic_number;           // 魔术数，用于校验配置有效性
} SystemConfig_TypeDef;

/* Flash地址定义 */
#define SYSTEM_CONFIG_FLASH_ADDR    0x000000  // 系统配置地址
#define CONFIG_MAGIC_NUMBER         0x12345678 // 校验魔术数

/* 默认配置 */
#define DEFAULT_WORK_MODE           0    // 默认手动模式
#define DEFAULT_BRIGHTNESS          20   // 默认亮度20
#define DEFAULT_SITTING_ENABLE      1    // 默认开启久坐提醒
#define DEFAULT_SITTING_TIME        1800 // 默认超时30分钟
#define DEFAULT_SITTING_DISTANCE    50   // 默认距离50cm
#define DEFAULT_AUTO_SENSITIVITY    50   // 默认灵敏度50

/* 函数声明 */
void SystemConfig_Init(void);
void SystemConfig_Save(SystemConfig_TypeDef *config);
void SystemConfig_Load(SystemConfig_TypeDef *config);
void SystemConfig_SetDefault(SystemConfig_TypeDef *config);

extern SystemConfig_TypeDef g_system_config;

#endif /* __SYSTEM_CONFIG_H */
