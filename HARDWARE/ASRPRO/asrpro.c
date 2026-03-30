#include "asrpro.h"

volatile uint8_t asrpro_rx_cmd = 0;

void ASRPRO_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(ASRPRO_GPIO_CLK | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(ASRPRO_USART_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = ASRPRO_TX_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ASRPRO_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin  = ASRPRO_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(ASRPRO_GPIO_PORT, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate            = ASRPRO_BAUDRATE;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(ASRPRO_USART, &USART_InitStructure);

    USART_ITConfig(ASRPRO_USART, USART_IT_RXNE, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel                   = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(ASRPRO_USART, ENABLE);
}

void ASRPRO_SendByte(uint8_t byte)
{
    while (USART_GetFlagStatus(ASRPRO_USART, USART_FLAG_TXE) == RESET);
    USART_SendData(ASRPRO_USART, byte);
    while (USART_GetFlagStatus(ASRPRO_USART, USART_FLAG_TC)  == RESET);
}

void ASRPRO_SendString(const char *str)
{
    while (*str) ASRPRO_SendByte((uint8_t)(*str++));
}

// 接收中断：将语音识别命令字节存入 asrpro_rx_cmd
void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(ASRPRO_USART, USART_IT_RXNE) != RESET) {
        asrpro_rx_cmd = (uint8_t)USART_ReceiveData(ASRPRO_USART);
        USART_ClearITPendingBit(ASRPRO_USART, USART_IT_RXNE);
    }
}
