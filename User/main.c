/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"
/* 开发板硬件bsp头文件 */
#include "stm32f4xx.h"
#include <stdio.h>
#include <stdlib.h>
#include "bsp_beep.h"
#include "bsp_cs100a.h"
#include "bsp_debug_usart.h"
#include "bsp_dht11.h"
#include "bsp_iic_debug.h"
#include "bsp_key.h"
#include "bsp_led.h"
#include "bsp_oled_debug.h"
#include "core_delay.h"
#include "bsp_spi_flash.h"
#include "bsp_photoresistor.h"
#include "bsp_timer_encoder.h"
#include "system_config.h"
#include "usart_protocol.h"
#include "app_tasks.h"

/* 任务句柄 */
static TaskHandle_t AppTaskCreate_Handle = NULL;
static TaskHandle_t Manual_Mode_Task_Handle = NULL;
static TaskHandle_t Energy_Save_Mode_Task_Handle = NULL;
static TaskHandle_t Auto_Mode_Task_Handle = NULL;
static TaskHandle_t Sitting_Reminder_Task_Handle = NULL;
static TaskHandle_t Environment_Monitor_Task_Handle = NULL;
static TaskHandle_t UI_Manager_Task_Handle = NULL;

/* 函数声明 */
static void AppTaskCreate(void);

/* 全局变量 */
int16_t brightness = 20;
uint8_t mode = 0;
uint8_t sw_key_flag = 0;

/**
 * @brief  主函数
 * @param  无
 * @retval int
 */
int main(void) {
    BaseType_t xReturn = pdPASS;

    /* 开发板硬件初始化 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    CPU_TS_TmrInit();
    LED_PWM_Config();
    Debug_USART_Config();
    Key_EXTI_Config();
    IIC_GPIO_Config();
    OLED_Init();
    OLED_CLS();
    BEEP_GPIO_Config();
    CS100A_Init();
    DHT11_GPIO_Config();
    SPI_FLASH_Init();
    PhotoResistor_Init();
    TIMX_Encoder_Init();

    /* 初始化系统配置和串口协议 */
    SystemConfig_Init();
    USART_Protocol_Init();

    /* 加载保存的配置参数 */
    g_current_work_mode = g_system_config.work_mode;
    g_brightness = g_system_config.brightness;

    /* 显示欢迎信息 */
    OLED_ShowStr(0, 0, (unsigned char *)"Smart Light", 2);
    OLED_ShowStr(0, 2, (unsigned char *)"System Init", 2);

    /* 创建AppTaskCreate任务 */
    xReturn = xTaskCreate((TaskFunction_t)AppTaskCreate,
                    (const char *)"AppTaskCreate",
                    (uint16_t)512,
                    (void *)NULL,
                    (UBaseType_t)1,
                    (TaskHandle_t *)&AppTaskCreate_Handle);

    /* 启动调度器 */
    if (pdPASS == xReturn)
        vTaskStartScheduler();
    else
        return -1;

    while (1);
}

/**
 * @brief  创建应用任务
 * @param  无
 * @retval 无
 */
static void AppTaskCreate(void) {
    BaseType_t xReturn = pdPASS;

    taskENTER_CRITICAL();

    /* 创建手动模式任务 */
    xReturn = xTaskCreate((TaskFunction_t)Task_Manual_Mode,
                    (const char *)"Manual_Mode",
                    (uint16_t)512,
                    (void *)NULL,
                    (UBaseType_t)3,
                    (TaskHandle_t *)&Manual_Mode_Task_Handle);
    if (pdPASS == xReturn)
        printf("创建手动模式任务成功!\r\n");

    /* 创建节能模式任务 */
    xReturn = xTaskCreate((TaskFunction_t)Task_Energy_Save_Mode,
                    (const char *)"Energy_Save",
                    (uint16_t)512,
                    (void *)NULL,
                    (UBaseType_t)3,
                    (TaskHandle_t *)&Energy_Save_Mode_Task_Handle);
    if (pdPASS == xReturn)
        printf("创建节能模式任务成功!\r\n");

    /* 创建自动模式任务 */
    xReturn = xTaskCreate((TaskFunction_t)Task_Auto_Mode,
                    (const char *)"Auto_Mode",
                    (uint16_t)512,
                    (void *)NULL,
                    (UBaseType_t)3,
                    (TaskHandle_t *)&Auto_Mode_Task_Handle);
    if (pdPASS == xReturn)
        printf("创建自动模式任务成功!\r\n");

    /* 创建久坐提醒任务 */
    xReturn = xTaskCreate((TaskFunction_t)Task_Sitting_Reminder,
                    (const char *)"Sitting_Reminder",
                    (uint16_t)512,
                    (void *)NULL,
                    (UBaseType_t)2,
                    (TaskHandle_t *)&Sitting_Reminder_Task_Handle);
    if (pdPASS == xReturn)
        printf("创建久坐提醒任务成功!\r\n");

    /* 创建环境监测任务 */
    xReturn = xTaskCreate((TaskFunction_t)Task_Environment_Monitor,
                    (const char *)"Env_Monitor",
                    (uint16_t)512,
                    (void *)NULL,
                    (UBaseType_t)2,
                    (TaskHandle_t *)&Environment_Monitor_Task_Handle);
    if (pdPASS == xReturn)
        printf("创建环境监测任务成功!\r\n");

    /* 创建UI管理任务 */
    xReturn = xTaskCreate((TaskFunction_t)Task_UI_Manager,
                    (const char *)"UI_Manager",
                    (uint16_t)512,
                    (void *)NULL,
                    (UBaseType_t)4,
                    (TaskHandle_t *)&UI_Manager_Task_Handle);
    if (pdPASS == xReturn)
        printf("创建UI管理任务成功!\r\n");

    vTaskDelete(AppTaskCreate_Handle);

    taskEXIT_CRITICAL();
}
