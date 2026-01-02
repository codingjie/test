#ifndef __USART_PROTOCOL_H
#define __USART_PROTOCOL_H

#include "stm32f4xx.h"

/* ͨ��Э�鶨�� */
#define PROTOCOL_HEAD           0xAA    // ֡ͷ
#define PROTOCOL_TAIL           0x55    // ֡β

/* ���������� */
#define CMD_SET_MODE            0x01    // ����ģʽ
#define CMD_SET_BRIGHTNESS      0x02    // ���������
#define CMD_QUERY_STATUS        0x03    // ��ѯ״̬
#define CMD_SET_SITTING_CFG     0x04    // �����ò�������
#define CMD_SAVE_CONFIG         0x05    // ���������
#define CMD_RESET_CONFIG        0x06    // ��������
#define CMD_QUERY_ENV           0x07    // ��ѯ�����Ϣ

/* Ӧ������ */
#define ACK_OK                  0x00    // ִ�гɹ�
#define ACK_ERROR               0x01    // ִ��ʧ��
#define ACK_INVALID_CMD         0x02    // ��Ч����
#define ACK_CRC_ERROR           0x03    // CRCУ��ʧ��

/* ͨ�������ݽṹ */
#pragma pack(1)
typedef struct {
    uint8_t head;           // ֡ͷ 0xAA
    uint8_t cmd;            // ��������
    uint8_t length;         // �����ݳ���
    uint8_t data[32];       // ��������
    uint16_t crc;           // CRCУ��ֵ
    uint8_t tail;           // ֡β 0x55
} Protocol_Frame_TypeDef;
#pragma pack()

/* DMA���ջ����� */
#define USART_RX_BUFFER_SIZE    128
extern uint8_t usart_rx_buffer[USART_RX_BUFFER_SIZE];
extern volatile uint8_t usart_rx_flag;

/* ��������� */
void USART_Protocol_Init(void);
void USART_DMA_Config(void);
uint16_t CRC16_Calculate(uint8_t *data, uint16_t length);
void Protocol_ParseFrame(uint8_t *data, uint16_t length);
void Protocol_SendResponse(uint8_t cmd, uint8_t ack, uint8_t *data, uint8_t length);

#endif /* __USART_PROTOCOL_H */
