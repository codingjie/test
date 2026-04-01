#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f10x.h"                  // Device header
#include "delay.h"
 
//修改下面三个参数来进行引脚的修改（更加方便进行移植）
#define DHT11_PIN              GPIO_Pin_10
#define DHT11_RCC              RCC_APB2Periph_GPIOB
#define DHT11_GPIO_PORT        GPIOB
 
#define DHT11_LOGIC_HIGH       1
#define DHT11_LOGIC_LOW        0
 
#define DHT11_DATA_OUTPUT(a)  if (a) \
                                GPIO_SetBits(DHT11_GPIO_PORT, DHT11_PIN);\
                                else \
                                GPIO_ResetBits(DHT11_GPIO_PORT, DHT11_PIN)
 
#define DHT11_DATA_INPUT()     GPIO_ReadInputDataBit(DHT11_GPIO_PORT, DHT11_PIN)

typedef struct
{
    uint8_t humidity_integer;    //湿度的整数部分
    uint8_t humidity_decimal;    //湿度的小数部分
    uint8_t temperature_integer; //温度的整数部分
    uint8_t temperature_decimal; //温度的小数部分
    uint8_t checksum;            
} DHT11_Sensor_Data_TypeDef;
 
void DHT11_Sensor_GPIO_Configuration(void);
static void DHT11_Sensor_SetInputPullUpMode(void);
static void DHT11_Sensor_SetOutputPushPullMode(void);
uint8_t DHT11_ReadSensorData(DHT11_Sensor_Data_TypeDef *sensorData);
static uint8_t DHT11_ReadDataByte(void);
 
#endif
