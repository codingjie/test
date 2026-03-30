#ifndef __OLED_H
#define __OLED_H

#include "stm32f10x.h"

#define GPIO_PORT_I2C	GPIOB
#define RCC_I2C_PORT 	RCC_APB2Periph_GPIOB
#define I2C_SCL_PIN		GPIO_Pin_6
#define I2C_SDA_PIN		GPIO_Pin_7

#define OLED_W_SCL(x) GPIO_WriteBit(GPIO_PORT_I2C, I2C_SCL_PIN, (BitAction)(x))
#define OLED_W_SDA(x) GPIO_WriteBit(GPIO_PORT_I2C, I2C_SDA_PIN, (BitAction)(x))

void OLED_Init(void);
void OLED_Set_Pos(uint8_t x, uint8_t y);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr);
void OLED_ShowString(uint8_t x, uint8_t y, uint8_t* str);
void OLED_ShowNum(uint8_t x, uint8_t y, int32_t num, uint8_t len);
void OLED_ShowFloat(uint8_t x, uint8_t y, float num, uint8_t k);
void OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t index);

#endif
