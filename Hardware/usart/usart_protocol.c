#include "usart_protocol.h"
#include "bsp_debug_usart.h"
#include "system_config.h"
#include "bsp_led.h"
#include "bsp_dht11.h"
#include "bsp_cs100a.h"
#include "bsp_photoresistor.h"
#include <string.h>
#include <stdio.h>

/* DMA接收缓冲区 */
uint8_t usart_rx_buffer[USART_RX_BUFFER_SIZE];
volatile uint8_t usart_rx_flag = 0;

/**
 * @brief  初始化串口协议（DMA + IDLE中断）
 * @param  无
 * @retval 无
 */
void USART_Protocol_Init(void) {
    USART_DMA_Config();
}

/**
 * @brief  配置串口DMA接收
 * @param  无
 * @retval 无
 */
void USART_DMA_Config(void) {
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 使能DMA2时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    /* 复位DMA */
    DMA_DeInit(DMA2_Stream2);
    while (DMA_GetCmdStatus(DMA2_Stream2) != DISABLE);

    /* 配置DMA */
    DMA_InitStructure.DMA_Channel = DMA_Channel_4;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)usart_rx_buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure.DMA_BufferSize = USART_RX_BUFFER_SIZE;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream2, &DMA_InitStructure);

    /* 配置USART IDLE中断 */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 使能串口空闲中断 */
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

    /* 使能串口DMA接收 */
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);

    /* 使能DMA */
    DMA_Cmd(DMA2_Stream2, ENABLE);
}

/**
 * @brief  CRC16校验计算
 * @param  data: 数据缓冲区指针
 * @param  length: 数据长度
 * @retval CRC16校验值
 */
uint16_t CRC16_Calculate(uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    uint16_t i, j;

    for (i = 0; i < length; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}

/**
 * @brief  协议帧解析
 * @param  data: 数据缓冲区指针
 * @param  length: 数据长度
 * @retval 无
 */
void Protocol_ParseFrame(uint8_t *data, uint16_t length) {
    Protocol_Frame_TypeDef *frame = (Protocol_Frame_TypeDef *)data;
    uint16_t crc_calc;
    uint8_t response_data[32];
    uint8_t response_len = 0;
    uint8_t ack = ACK_OK;

    /* 校验帧头帧尾 */
    if (frame->head != PROTOCOL_HEAD || data[length - 1] != PROTOCOL_TAIL) {
        return;
    }

    /* 校验CRC */
    crc_calc = CRC16_Calculate(data + 1, 2 + frame->length);
    if (crc_calc != frame->crc) {
        Protocol_SendResponse(frame->cmd, ACK_CRC_ERROR, NULL, 0);
        return;
    }

    /* 根据命令码执行 */
    switch (frame->cmd) {
        case CMD_SET_MODE: // 设置工作模式
            if (frame->data[0] <= 2) {
                g_system_config.work_mode = frame->data[0];
                ack = ACK_OK;
            } else {
                ack = ACK_ERROR;
            }
            break;

        case CMD_SET_BRIGHTNESS: // 设置亮度
            if (frame->data[0] <= 99) {
                g_system_config.brightness = frame->data[0];
                LED_SetRGB(frame->data[0], frame->data[0], frame->data[0]);
                ack = ACK_OK;
            } else {
                ack = ACK_ERROR;
            }
            break;

        case CMD_QUERY_STATUS: // 查询状态
            response_data[0] = g_system_config.work_mode;
            response_data[1] = g_system_config.brightness;
            response_len = 2;
            ack = ACK_OK;
            break;

        case CMD_SET_SITTING_CFG: // 设置久坐配置
            g_system_config.sitting_reminder_enable = frame->data[0];
            g_system_config.sitting_time_threshold = (frame->data[1] << 8) | frame->data[2];
            g_system_config.sitting_distance = (frame->data[3] << 8) | frame->data[4];
            ack = ACK_OK;
            break;

        case CMD_SAVE_CONFIG: // 保存配置
            SystemConfig_Save(&g_system_config);
            ack = ACK_OK;
            break;

        case CMD_RESET_CONFIG: // 重置配置
            SystemConfig_SetDefault(&g_system_config);
            SystemConfig_Save(&g_system_config);
            ack = ACK_OK;
            break;

        case CMD_QUERY_ENV: // 查询环境信息
        {
            DHT11_Data_TypeDef dht11_data;
            if (Read_DHT11(&dht11_data) == 0) {
                response_data[0] = dht11_data.temp_int;
                response_data[1] = dht11_data.humi_int;
                response_data[2] = (uint8_t)CS100A_GetDistance();
                response_data[3] = (PhotoResistor_GetValue() >> 8) & 0xFF;
                response_data[4] = PhotoResistor_GetValue() & 0xFF;
                response_len = 5;
                ack = ACK_OK;
            } else {
                ack = ACK_ERROR;
            }
        }
            break;

        default:
            ack = ACK_INVALID_CMD;
            break;
    }

    /* 发送应答 */
    Protocol_SendResponse(frame->cmd, ack, response_data, response_len);
}

/**
 * @brief  发送应答帧
 * @param  cmd: 命令码
 * @param  ack: 应答码
 * @param  data: 数据缓冲区指针
 * @param  length: 数据长度
 * @retval 无
 */
void Protocol_SendResponse(uint8_t cmd, uint8_t ack, uint8_t *data, uint8_t length) {
    uint8_t send_buf[64];
    uint16_t crc;
    uint8_t index = 0;

    /* 组装协议帧 */
    send_buf[index++] = PROTOCOL_HEAD;
    send_buf[index++] = cmd;
    send_buf[index++] = length + 1;  // 长度包含ACK字节
    send_buf[index++] = ack;

    if (data != NULL && length > 0) {
        memcpy(&send_buf[index], data, length);
        index += length;
    }

    /* 计算CRC */
    crc = CRC16_Calculate(&send_buf[1], index - 1);
    send_buf[index++] = (crc >> 8) & 0xFF;
    send_buf[index++] = crc & 0xFF;
    send_buf[index++] = PROTOCOL_TAIL;

    /* 发送协议帧 */
    for (uint8_t i = 0; i < index; i++) {
        USART_SendData(USART1, send_buf[i]);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    }
}
