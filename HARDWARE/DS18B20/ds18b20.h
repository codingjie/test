#ifndef __DS18B20_H
#define __DS18B20_H

#include "stm32f10x.h"
#include <stdint.h>

/* DS18B20 单总线温度传感器
 * PA1: 单总线数据，需外接 4.7kΩ 上拉至 VCC
 */
#define DS18B20_GPIO_PORT   GPIOA
#define DS18B20_GPIO_CLK    RCC_APB2Periph_GPIOA
#define DS18B20_GPIO_PIN    GPIO_Pin_1

#define DS18B20_OUT_HIGH()  GPIO_SetBits(DS18B20_GPIO_PORT,   DS18B20_GPIO_PIN)
#define DS18B20_OUT_LOW()   GPIO_ResetBits(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN)
#define DS18B20_READ_BIT()  GPIO_ReadInputDataBit(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN)

void    DS18B20_Init(void);
uint8_t DS18B20_Reset(void);           /* 返回 0: 在线, 非0: 不在线 */
float   DS18B20_ReadTemp(void);        /* 阻塞式读取，内部等待 750ms 转换 */
void    DS18B20_StartConvert(void);    /* 发出转换命令（非阻塞） */
float   DS18B20_GetTemp(void);         /* 读取上次转换结果（需等待 750ms 后调用） */

#endif
