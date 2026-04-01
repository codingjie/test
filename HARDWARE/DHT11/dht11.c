#include "dht11.h"
 
void DHT11_Sensor_GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
 
    RCC_APB2PeriphClockCmd(DHT11_RCC, ENABLE); 
    GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);
 
    GPIO_SetBits(DHT11_GPIO_PORT, DHT11_PIN);
}

static void DHT11_Sensor_SetInputPullUpMode(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);
}

static void DHT11_Sensor_SetOutputPushPullMode(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
 
    GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 
    GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);
}
 
static uint8_t DHT11_ReadDataByte(void)
{
    uint8_t i, temp = 0;
 
    for (i = 0; i < 8; i++)
    {
        while (DHT11_DATA_INPUT() == Bit_RESET);
 
        delay_us(35);
 
        if (DHT11_DATA_INPUT() == Bit_SET)
        {
            while (DHT11_DATA_INPUT() == Bit_SET);
 
            temp |= (uint8_t)(0x01 << (7 - i));
        }
        else
        {
            temp &= (uint8_t) ~(0x01 << (7 - i));
        }
    }
    return temp;
}
 
uint8_t DHT11_ReadSensorData(DHT11_Sensor_Data_TypeDef *sensorData)
{
    DHT11_Sensor_SetOutputPushPullMode();
    DHT11_DATA_OUTPUT(DHT11_LOGIC_LOW);
    delay_ms(18);
 
    DHT11_DATA_OUTPUT(DHT11_LOGIC_HIGH);
 
    delay_us(30);
 
    DHT11_Sensor_SetInputPullUpMode();
 
    if (DHT11_DATA_INPUT() == Bit_RESET)
    {
        while (DHT11_DATA_INPUT() == Bit_RESET);
 
        while (DHT11_DATA_INPUT() == Bit_SET);
 
        sensorData->humidity_integer = DHT11_ReadDataByte();
        sensorData->humidity_decimal = DHT11_ReadDataByte();
        sensorData->temperature_integer = DHT11_ReadDataByte();
        sensorData->temperature_decimal = DHT11_ReadDataByte();
        sensorData->checksum = DHT11_ReadDataByte();
 
        DHT11_Sensor_SetOutputPushPullMode();
        DHT11_DATA_OUTPUT(DHT11_LOGIC_HIGH);
 
        if (sensorData->checksum == sensorData->humidity_integer + sensorData->humidity_decimal + sensorData->temperature_integer + sensorData->temperature_decimal)
            return SUCCESS;
        else
            return SUCCESS;
    }
    else
    {
        return ERROR;
    }
}

