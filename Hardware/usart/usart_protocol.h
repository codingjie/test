#ifndef __USART_PROTOCOL_H
#define __USART_PROTOCOL_H

#include "stm32f4xx.h"

/* 通信协议定义 */
#define PROTOCOL_HEAD           0xAA    // 帧头
#define PROTOCOL_TAIL           0x55    // 帧尾

/* 命令码定义 */
#define CMD_SET_MODE            0x01    // 设置模式
#define CMD_SET_BRIGHTNESS      0x02    // 设置亮度
#define CMD_QUERY_STATUS        0x03    // 查询状态
#define CMD_SET_SITTING_CFG     0x04    // 设置久坐配置
#define CMD_SAVE_CONFIG         0x05    // 保存配置
#define CMD_RESET_CONFIG        0x06    // 重置配置
#define CMD_QUERY_ENV           0x07    // 查询环境信息

/* 应答码定义 */
#define ACK_OK                  0x00    // 执行成功
#define ACK_ERROR               0x01    // 执行失败
#define ACK_INVALID_CMD         0x02    // 无效命令
#define ACK_CRC_ERROR           0x03    // CRC校验失败

/* 通信协议数据结构 */
#pragma pack(1)
typedef struct {
    uint8_t head;           // 帧头 0xAA
    uint8_t cmd;            // 命令码
    uint8_t length;         // 数据长度
    uint8_t data[32];       // 数据载荷
    uint16_t crc;           // CRC校验值
    uint8_t tail;           // 帧尾 0x55
} Protocol_Frame_TypeDef;
#pragma pack()

/* DMA接收缓冲区大小 */
#define USART_RX_BUFFER_SIZE    128
extern uint8_t usart_rx_buffer[USART_RX_BUFFER_SIZE];
extern volatile uint8_t usart_rx_flag;

/* 函数声明 */
void USART_Protocol_Init(void);
void USART_DMA_Config(void);
uint16_t CRC16_Calculate(uint8_t *data, uint16_t length);
void Protocol_ParseFrame(uint8_t *data, uint16_t length);
void Protocol_SendResponse(uint8_t cmd, uint8_t ack, uint8_t *data, uint8_t length);

#endif /* __USART_PROTOCOL_H */
