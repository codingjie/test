#ifndef __ASRPRO_H
#define __ASRPRO_H

#include "stm32f10x.h"
#include <stdint.h>

// 识别到语音后发送单字节命令码：
#define ASRPRO_CMD_FAN_ON       0x61    /* 开启风扇 (ID0, 'a') */
#define ASRPRO_CMD_FAN_ON2      0x62    /* 打开风扇 (ID1, 'b') */
#define ASRPRO_CMD_FAN_OFF      0x63    /* 关闭风扇 (ID2, 'c') */
#define ASRPRO_CMD_SPEED_UP     0x64    /* 增加档位 (ID3, 'd') */
#define ASRPRO_CMD_SPEED_DOWN   0x65    /* 减小档位 (ID4, 'e') */
#define ASRPRO_CMD_SPEED_1      0x66    /* 一档风   (ID5, 'f') */
#define ASRPRO_CMD_SPEED_2      0x67    /* 二档风   (ID6, 'g') */
#define ASRPRO_CMD_SPEED_3      0x68    /* 三档风   (ID7, 'h') */
#define ASRPRO_CMD_SWING_ON     0x69    /* 打开摇头 (ID8, 'i') */
#define ASRPRO_CMD_SWING_OFF    0x6A    /* 关闭摇头 (ID9, 'j') */
#define ASRPRO_CMD_MODE_MANUAL  0x6B    /* 切换手动模式 (ID10, 'k') */
#define ASRPRO_CMD_MODE_AUTO    0x6C    /* 切换自动模式 (ID11, 'l') */
#define ASRPRO_CMD_MODE_VOICE   0x6D    /* 切换语音模式 (ID12, 'm') */
#define ASRPRO_CMD_MODE_TRACK   0x6E    /* 切换追踪模式 (ID13, 'n') */

#define ASRPRO_USART        USART2
#define ASRPRO_USART_CLK    RCC_APB1Periph_USART2
#define ASRPRO_GPIO_CLK     RCC_APB2Periph_GPIOA
#define ASRPRO_GPIO_PORT    GPIOA
#define ASRPRO_TX_PIN       GPIO_Pin_2
#define ASRPRO_RX_PIN       GPIO_Pin_3
#define ASRPRO_BAUDRATE     9600

/* 最近一次接收到的命令，0 表示无新命令，读取后由应用层清零 */
extern volatile uint8_t asrpro_rx_cmd;

void ASRPRO_Init(void);
void ASRPRO_SendByte(uint8_t byte);
void ASRPRO_SendString(const char *str);

#endif
