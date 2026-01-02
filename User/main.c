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
#include "bsp_debug_usart.h"
#include "bsp_esp8266.h"
#include "bsp_esp8266_test.h"

static TaskHandle_t AppTaskCreate_Handle = NULL; /* 创建任务句柄 */
static TaskHandle_t Manual_Mode_Task_Handle = NULL;     /* LED任务句柄 */
static TaskHandle_t KEY_Task_Handle = NULL;      /* KEY任务句柄 */

static void AppTaskCreate(void); /* 用于创建任务 */
static void Manual_Mode_Task(void *pvParameters);
static void KEY_Task(void *pvParameters);  /* KEY_Task任务实现 */

float cycle_count;         // 圈数
int16_t brightness = 20; // led灯的亮度 0~99
uint8_t mode = 0; // 界面选择
uint8_t sw_key_flag = 0; // 编码器按键

int main(void) {
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为pdPASS */

    /* 开发板硬件初始化 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); // 中断优先级分组为4
    CPU_TS_TmrInit();
    LED_PWM_Config();                              // LED引脚初始化
    Debug_USART_Config();                           // 串口初始化
    Key_EXTI_Config();                              // 按键初始化
    IIC_GPIO_Config();                              // IIC引脚初始化
    OLED_Init();                                    // OLED初始化
    OLED_CLS();
    BEEP_GPIO_Config();
    CS100A_Init();
    DHT11_GPIO_Config();
    SPI_FLASH_Init();
    PhotoResistor_Init();
    TIMX_Encoder_Init();
    Debug_USART_Config();
    // ESP8266_Init();
    // ESP8266_StaTcpServer_ConfigTest();

    /* 创建AppTaskCreate任务 */
    xReturn = xTaskCreate((TaskFunction_t)AppTaskCreate, /* 任务入口函数 */
                    (const char *)"AppTaskCreate", /* 任务名字 */
                    (uint16_t)512,                 /* 任务栈大小 */
                    (void *)NULL,                  /* 任务入口函数参数 */
                    (UBaseType_t)1,                /* 任务的优先级 */
                    (TaskHandle_t *)&AppTaskCreate_Handle); /* 任务控制块指针 */
    /* 启动任务调度 */
    if (pdPASS == xReturn)
        vTaskStartScheduler(); /* 启动任务，开启调度 */
    else
        return -1;

    while (1); /* 正常不会执行到这里 */
}

/***********************************************************************
 * @ 函数名  ： AppTaskCreate
 * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
 * @ 参数    ： 无
 * @ 返回值  ： 无
 **********************************************************************/
static void AppTaskCreate(void) {
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为pdPASS */

    taskENTER_CRITICAL(); // 进入临界区

    xReturn = xTaskCreate((TaskFunction_t)Manual_Mode_Task,          /* 任务入口函数 */
                    (const char *)"Manual_Mode_Task",          /* 任务名字 */
                    (uint16_t)512,                      /* 任务栈大小 */
                    (void *)NULL,                       /* 任务入口函数参数 */
                    (UBaseType_t)2,                     /* 任务的优先级 */
                    (TaskHandle_t *)&Manual_Mode_Task_Handle); /* 任务控制块指针 */
    if (pdPASS == xReturn)
        printf("创建Test_Task任务成功!\r\n");
    /* 创建KEY_Task任务 */
    xReturn = xTaskCreate((TaskFunction_t)KEY_Task,          /* 任务入口函数 */
                    (const char *)"KEY_Task",          /* 任务名字 */
                    (uint16_t)512,                     /* 任务栈大小 */
                    (void *)NULL,                      /* 任务入口函数参数 */
                    (UBaseType_t)3,                    /* 任务的优先级 */
                    (TaskHandle_t *)&KEY_Task_Handle); /* 任务控制块指针 */
    if (pdPASS == xReturn)
        printf("创建KEY_Task任务成功!\r\n");

    vTaskDelete(AppTaskCreate_Handle); // 删除AppTaskCreate任务

    taskEXIT_CRITICAL(); // 退出临界区
}

// 手动模式
static void Manual_Mode_Task(void *parameter) { 
    float cycle_count = 0;
    OLED_ShowStr(0, 0, "Mode Manual", 2);
    while(1) {
        Encoder_Get_Val(&cycle_count);
        if (mode == 0 && sw_key_flag) {
            OLED_ShowNum(0, 3, (uint8_t)(cycle_count * 10), 4);
            sw_key_flag = 1;
        }
        else if (mode == 1 && sw_key_flag) {
            LED_SetRGB(brightness, brightness, brightness);
            OLED_ShowNum(0, 3, brightness, 2);

            if(dirction_flag == POSITIVE_DIRECTION) { // 正
                OLED_ShowStr(80, 2, (unsigned char *)"CW ", 2);
                brightness++;
                if (brightness > 99) brightness = 99;
            }
            else if(dirction_flag == REVERSE_DIRECTION) { // 反
                OLED_ShowStr(80, 2, (unsigned char *)"CCW", 2);
                brightness--;
                if (brightness < 0) brightness = 0;
            }
            else {
                OLED_ShowStr(80, 2, (unsigned char *)"-- ", 2);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/**********************************************************************
 * @ 函数名  ： Test_Task
 * @ 功能说明： Test_Task任务主体
 * @ 参数    ：
 * @ 返回值  ： 无
 ********************************************************************/
static void KEY_Task(void *parameter) {
    while(1) {

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
