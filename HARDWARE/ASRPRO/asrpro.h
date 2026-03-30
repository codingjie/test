#ifndef __ASRPRO_H
#define __ASRPRO_H

#include "stm32f10x.h"
#include <stdint.h>

/* ASRPRO voice recognition module sends a single command byte
 * when a keyword is recognized.
 *
 * Trash bin voice commands:
 *   0x61 -> "可回收垃圾" (recyclable waste)
 *   0x62 -> "有害垃圾"   (hazardous waste)
 *   0x63 -> "厨余垃圾"   (kitchen / food waste)
 *   0x64 -> "其他垃圾"   (other waste)
 *   0x65 -> "全部打开"   (open all lids)
 *   0x66 -> "全部关闭"   (close all lids)
 */
#define ASRPRO_CMD_BIN_RECYCLABLE   0x61
#define ASRPRO_CMD_BIN_HAZARDOUS    0x62
#define ASRPRO_CMD_BIN_KITCHEN      0x63
#define ASRPRO_CMD_BIN_OTHER        0x64
#define ASRPRO_CMD_OPEN_ALL         0x65
#define ASRPRO_CMD_CLOSE_ALL        0x66

#define ASRPRO_USART        USART2
#define ASRPRO_USART_CLK    RCC_APB1Periph_USART2
#define ASRPRO_GPIO_CLK     RCC_APB2Periph_GPIOA
#define ASRPRO_GPIO_PORT    GPIOA
#define ASRPRO_TX_PIN       GPIO_Pin_2
#define ASRPRO_RX_PIN       GPIO_Pin_3
#define ASRPRO_BAUDRATE     9600

/* Most-recently received command byte; cleared by application after use */
extern volatile uint8_t asrpro_rx_cmd;

void ASRPRO_Init(void);
void ASRPRO_SendByte(uint8_t byte);
void ASRPRO_SendString(const char *str);

#endif
