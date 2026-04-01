#ifndef __USART_H
#define __USART_H

#include "stdio.h"
#include "sys.h" 

#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收

// 接收状态枚举
typedef enum {
    RX_STATE_IDLE   = 0,  // 空闲，正在接收数据
    RX_STATE_GOT_CR = 1,  // 已收到 \r，等待 \n
    RX_STATE_DONE   = 2   // 接收完成，等待上层处理
} RxState;

extern uint8_t USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern RxState rx_state;
extern uint16_t rx_count;

void USART_Config(uint32_t bound);
void USART_SendString(const char *str);

#endif
