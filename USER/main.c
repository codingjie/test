#include "stm32f10x.h"
#include "delay.h"
#include "oled.h"
#include "adc.h"
#include "motor.h"
#include "beep.h"
#include "infra.h"
#include "dht11.h"
#include "usart.h"
#include "key.h"

// 阈值定义
#define TEMP_HIGH       35      // 温度超过此值报警（℃）
#define HUMI_LOW        35      // 湿度超过此值报警（%）
#define LIGHT_HIGH      80      // 光照强度阈值

// 当前采集值
DHT11_Sensor_Data_TypeDef dht11Data;
uint8_t temp_value  = 0;    // 温度（℃）
uint8_t humi_value  = 0;    // 湿度（%）
uint8_t light_value = 0;    // 光照强度 Lux
uint8_t infra_flag  = 0;    // 红外标志
char buf[64];

int main(void) {
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
    ADC1_GPIO_Config();
    ADC1_Config();
    delay_init();
    OLED_Init();
    MOTOR_GPIO_Config();
    BEEP_GPIO_Config();
    INFRA_GPIO_Config();
    DHT11_Sensor_GPIO_Configuration();
    USART_Config(9600);

    OLED_Clear();

    // 第0行：温度
    OLED_ShowString(0,  0, (uint8_t *)"Temp:");
    // 第2行：土壤湿度
    OLED_ShowString(0, 2, (uint8_t *)"Humi:");
    // 第4行：光照强度
    OLED_ShowString(0,  4, (uint8_t *)"Light:");

    while (1) {  
        if(DHT11_ReadSensorData(&dht11Data) == 1) {
            temp_value = dht11Data.temperature_integer;  // 温度整数
        }

        ADC1_ReadAll();
        // 映射到 0~100 %
        humi_value  = (uint16_t)(ADCValues[0] * 100.0f / 4096);
        // 映射到 0~100 Lux
        light_value   = (uint16_t)(ADCValues[1] * 100.0f / 4096);

        // 刷新温度
        sprintf(buf, "%-2dC  ", temp_value); // %-2d 左对齐占2位，多余空格清除旧数据
        OLED_ShowString(48, 0, (uint8_t *)buf);

        // 刷新湿度
        sprintf(buf, "%-3d %% ", humi_value); // %% 转义显示百分号
        OLED_ShowString(48, 2, (uint8_t *)buf);

        // 刷新光照
        sprintf(buf, "%-3dLux", light_value);
        OLED_ShowString(48, 4, (uint8_t *)buf);

        // 报警判断
        if (temp_value  > TEMP_HIGH  || humi_value  < HUMI_LOW || light_value > LIGHT_HIGH || infra_flag) {
            BEEP_ON();
            OLED_ShowString(0, 6, (uint8_t *)("ALARM "));
        }
        else {
            BEEP_OFF();
            OLED_ShowString(0, 6, (uint8_t *)("NORMAL"));
        }

        // 水泵判断
        if (humi_value < HUMI_LOW) {
            MOTOR_PUMP_ON();
        }
        else {
            MOTOR_PUMP_OFF();
        }

        // 风扇判断
        if (temp_value > TEMP_HIGH) {
            MOTOR_FAN_ON();
        }
        else {
            MOTOR_FAN_OFF();
        }

        // 舵机判断
        if (light_value > LIGHT_HIGH) {
            MOTOR_SERVO_ON();
        }
        else {
            MOTOR_SERVO_OFF();
        }

        /* 串口发送 */
        sprintf(buf, "Temp:%dC Humi:%d%% Light:%d Lux\r\n", temp_value, humi_value, light_value);
        USART_SendString(buf);

        delay_ms(500);
    }
}
