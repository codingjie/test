#ifndef __LCD_H
#define __LCD_H
#include "sys.h"

// LCD数据和控制引脚定义
#define D7_Pin GPIO_Pin_13
#define RST_Pin GPIO_Pin_0
#define CS1_Pin GPIO_Pin_1
#define CS2_Pin GPIO_Pin_2
#define EN_Pin GPIO_Pin_3
#define RW_Pin GPIO_Pin_4
#define RS_Pin GPIO_Pin_5
#define D0_Pin GPIO_Pin_6
#define D1_Pin GPIO_Pin_7
#define D2_Pin GPIO_Pin_8
#define D3_Pin GPIO_Pin_9
#define D4_Pin GPIO_Pin_10
#define D5_Pin GPIO_Pin_11
#define D6_Pin GPIO_Pin_12

// LCD控制命令
#define LCDSTARTROW 0xC0
#define LCDPAGE 0xB8
#define LCDLINE 0x40

// 初始化LCD显示
void setupLCDDisplay(void);

// LCD显示更新任务
void displayUpdateTask(void *pdata);

// 数据转换函数
void convertDataToBits(uint16_t data, BitAction *transform, uint16_t length);

// 写数据到LCD
void writeDisplayData(uint16_t ucData);

// 写命令到LCD
void writeDisplayCmd(uint16_t ucCMD);

// LCD硬件初始化
void initDisplayHardware(void);

// 显示一行自定义内容
void displayCustomLine(uint16_t ucPage, uint16_t ucLine, uint16_t ucWidth, unsigned char *ucaRow);

// 全屏清除
void clearDisplayAll(void);

// 打印16点高度内容
void render16PointText(uint16_t startpage, uint16_t startline, uint16_t width, unsigned char *data);

#endif
