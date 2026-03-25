#ifndef _LCD_H
#define _LCD_H

#include "stm32f10x.h"
#include "stm32f10x_spi.h"
#include <stdio.h>
#include <stdarg.h>

#define LCD_WIDTH   128
#define LCD_HEIGHT  160

/* --- PA 组: SCK(PA5), SDA(PA7), CS(PA4), BLK(PA1) --- */
#define LCD_RCC_GPA       RCC_APB2Periph_GPIOA
#define LCD_GPA_PORT      GPIOA

#define LCD_SCK_PIN       GPIO_Pin_5    /* PA5 SPI1_SCK 复用推挽 */
#define LCD_SDA_PIN       GPIO_Pin_7    /* PA7 SPI1_MOSI 复用推挽 */
#define LCD_CS_PIN        GPIO_Pin_4    /* PA4 推挽输出 */
#define LCD_BLK_PIN       GPIO_Pin_1    /* PA1 推挽输出 */

/* --- PB 组: RES(PB1), DC(PB0) --- */
#define LCD_RCC_GPB       RCC_APB2Periph_GPIOB
#define LCD_GPB_PORT      GPIOB

#define LCD_RES_PIN       GPIO_Pin_1    /* PB1 推挽输出 */
#define LCD_DC_PIN        GPIO_Pin_0    /* PB0 推挽输出 */

#define LCD_SPI           SPI1
#define LCD_RCC_SPI       RCC_APB2Periph_SPI1

/* 引脚快速操作 */
#define LCD_CS_H    (LCD_GPA_PORT->BSRR = LCD_CS_PIN)
#define LCD_CS_L    (LCD_GPA_PORT->BRR  = LCD_CS_PIN)

#define LCD_DC_H    (LCD_GPB_PORT->BSRR = LCD_DC_PIN)
#define LCD_DC_L    (LCD_GPB_PORT->BRR  = LCD_DC_PIN)

#define LCD_RES_H   (LCD_GPB_PORT->BSRR = LCD_RES_PIN)
#define LCD_RES_L   (LCD_GPB_PORT->BRR  = LCD_RES_PIN)

#define LCD_BLK_H   (LCD_GPA_PORT->BSRR = LCD_BLK_PIN)
#define LCD_BLK_L   (LCD_GPA_PORT->BRR  = LCD_BLK_PIN)

/* ==================== 常用颜色 (RGB565) ==================== */
#define RED         0xF800
#define GREEN       0x07E0
#define BLUE        0x001F
#define BLUE2       0x1C9F
#define PINK        0xD8A7
#define ORANGE      0xFA20
#define WHITE       0xFFFF
#define BLACK       0x0000
#define YELLOW      0xFFE0
#define CYAN        0x07FF
#define PURPLE      0xF81F
#define PURPLE2     0xDB92
#define PURPLE3     0x8811
#define GRAY0       0xEF7D
#define GRAY1       0x8410
#define GRAY2       0x4208

/* 底层 SPI / GPIO */
void LCD_GPIO_Init(void);
void LCD_SPI_Init(void);
void LCD_SPI_SendByte(uint8_t data);
void LCD_SendCmd(uint8_t cmd);
void LCD_SendData(uint8_t data);
void LCD_Send16Bit(uint16_t data);
void LCD_SendCmdData(uint8_t cmd, uint8_t data);

/* 初始化与控制 */
void LCD_Init(void);
void LCD_Reset(void);
void LCD_BackLight(uint8_t on);
void LCD_SetRegion(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end);
void LCD_SetCursor(uint16_t x, uint16_t y);
void LCD_Clear(uint16_t color);
void LCD_Fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

/* 基本函数 */
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);
void LCD_ShowImage(uint16_t x, uint16_t y, uint16_t length, uint16_t width, const unsigned char *p);
void LCD_ShowChar(uint8_t x, uint8_t y, uint16_t fc, uint16_t bc, char c);
void LCD_ShowString(uint8_t x, uint8_t y, uint16_t fc, uint16_t bc, char *c);
void LCD_ShowNumber(uint8_t x, uint8_t y, uint16_t fc, uint16_t bc, long long num);
// void LCD_ShowChinese(uint8_t x, uint8_t y, uint16_t fc, uint16_t bc, char *c);

void LCD_Printf(const char *format, ...);

#endif
