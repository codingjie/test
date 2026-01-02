#ifndef __USART_PROTOCOL_H
#define __USART_PROTOCOL_H

#include "stm32f4xx.h"

/* Protocol definition */
#define PROTOCOL_HEAD           0xAA    // Frame header
#define PROTOCOL_TAIL           0x55    // Frame tail

/* Command codes */
#define CMD_SET_MODE            0x01    // Set work mode
#define CMD_SET_BRIGHTNESS      0x02    // Set brightness
#define CMD_QUERY_STATUS        0x03    // Query status
#define CMD_SET_SITTING_CFG     0x04    // Set sitting reminder config
#define CMD_SAVE_CONFIG         0x05    // Save configuration
#define CMD_RESET_CONFIG        0x06    // Reset configuration
#define CMD_QUERY_ENV           0x07    // Query environment data

/* Response codes */
#define ACK_OK                  0x00    // Execute success
#define ACK_ERROR               0x01    // Execute failed
#define ACK_INVALID_CMD         0x02    // Invalid command
#define ACK_CRC_ERROR           0x03    // CRC check failed

/* Protocol frame structure */
#pragma pack(1)
typedef struct {
    uint8_t head;           // Frame header 0xAA
    uint8_t cmd;            // Command code
    uint8_t length;         // Data length
    uint8_t data[32];       // Data payload
    uint16_t crc;           // CRC16 checksum
    uint8_t tail;           // Frame tail 0x55
} Protocol_Frame_TypeDef;
#pragma pack()

/* DMA receive buffer size */
#define USART_RX_BUFFER_SIZE    128
extern uint8_t usart_rx_buffer[USART_RX_BUFFER_SIZE];
extern volatile uint8_t usart_rx_flag;

/* Function prototypes */
void USART_Protocol_Init(void);
void USART_DMA_Config(void);
uint16_t CRC16_Calculate(uint8_t *data, uint16_t length);
void Protocol_ParseFrame(uint8_t *data, uint16_t length);
void Protocol_SendResponse(uint8_t cmd, uint8_t ack, uint8_t *data, uint8_t length);

#endif /* __USART_PROTOCOL_H */
