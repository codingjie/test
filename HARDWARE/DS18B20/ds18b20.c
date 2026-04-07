#include "ds18b20.h"
#include "delay.h"

static void DS18B20_SetOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = DS18B20_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DS18B20_GPIO_PORT, &GPIO_InitStructure);
}

static void DS18B20_SetInput(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin  = DS18B20_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(DS18B20_GPIO_PORT, &GPIO_InitStructure);
}

void DS18B20_Init(void)
{
    RCC_APB2PeriphClockCmd(DS18B20_GPIO_CLK, ENABLE);
    DS18B20_SetOutput();
    DS18B20_OUT_HIGH();
}

uint8_t DS18B20_Reset(void)
{
    uint8_t presence;
    DS18B20_SetOutput();
    DS18B20_OUT_LOW();
    delay_us(480);
    DS18B20_OUT_HIGH();
    delay_us(60);
    DS18B20_SetInput();
    presence = DS18B20_READ_BIT();
    delay_us(420);
    DS18B20_SetOutput();
    DS18B20_OUT_HIGH();
    return presence;   /* 0: 在线 */
}

static void DS18B20_WriteBit(uint8_t bit)
{
    DS18B20_SetOutput();
    DS18B20_OUT_LOW();
    delay_us(2);
    if (bit) DS18B20_OUT_HIGH();
    delay_us(60);
    DS18B20_OUT_HIGH();
    delay_us(2);
}

static uint8_t DS18B20_ReadBit(void)
{
    uint8_t bit;
    DS18B20_SetOutput();
    DS18B20_OUT_LOW();
    delay_us(2);
    DS18B20_OUT_HIGH();
    delay_us(10);
    DS18B20_SetInput();
    bit = DS18B20_READ_BIT();
    delay_us(50);
    return bit;
}

static void DS18B20_WriteByte(uint8_t byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++) { DS18B20_WriteBit(byte & 0x01); byte >>= 1; }
}

static uint8_t DS18B20_ReadByte(void)
{
    uint8_t i, byte = 0;
    for (i = 0; i < 8; i++) { byte >>= 1; if (DS18B20_ReadBit()) byte |= 0x80; }
    return byte;
}

/* 阻塞式：发出转换命令 + 等待 750ms + 读取结果 */
float DS18B20_ReadTemp(void)
{
    uint8_t low, high;
    if (DS18B20_Reset() != 0) return -999.0f;
    DS18B20_WriteByte(0xCC);
    DS18B20_WriteByte(0x44);
    delay_ms(750);
    if (DS18B20_Reset() != 0) return -999.0f;
    DS18B20_WriteByte(0xCC);
    DS18B20_WriteByte(0xBE);
    low  = DS18B20_ReadByte();
    high = DS18B20_ReadByte();
    return (float)(int16_t)((high << 8) | low) * 0.0625f;
}

/* 非阻塞：仅发出转换命令，750ms 后再调用 DS18B20_GetTemp() */
void DS18B20_StartConvert(void)
{
    if (DS18B20_Reset() != 0) return;
    DS18B20_WriteByte(0xCC);
    DS18B20_WriteByte(0x44);
}

/* 非阻塞：读取上次转换的温度结果 */
float DS18B20_GetTemp(void)
{
    uint8_t low, high;
    if (DS18B20_Reset() != 0) return -999.0f;
    DS18B20_WriteByte(0xCC);
    DS18B20_WriteByte(0xBE);
    low  = DS18B20_ReadByte();
    high = DS18B20_ReadByte();
    return (float)(int16_t)((high << 8) | low) * 0.0625f;
}
