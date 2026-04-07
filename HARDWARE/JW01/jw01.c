#include "jw01.h"

// 帧接收缓冲区
static uint8_t  s_buf[JW01_FRAME_LEN];
static uint8_t  s_idx     = 0;        // 当前写入位置
static uint8_t  s_in_frame = 0;       // 是否正在接收帧

// 最新解析结果
static uint16_t s_voc_ppb  = 0;
static uint8_t  s_new_data = 0;       // 有新数据标志

// 初始化 USART1（仅开 RX，PA10）
void JW01_Init(void) {
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;

    // 使能时钟
    RCC_APB2PeriphClockCmd(JW01_GPIO_CLK | JW01_USART_CLK, ENABLE);

    // PA10 浮空输入（USART1 RX）
    GPIO_InitStructure.GPIO_Pin  = JW01_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(JW01_RX_PORT, &GPIO_InitStructure);

    // USART1：9600 8N1，仅使能接收
    USART_InitStructure.USART_BaudRate            = JW01_BAUD;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx;
    USART_Init(JW01_USART, &USART_InitStructure);

    // 开启接收中断
    USART_ITConfig(JW01_USART, USART_IT_RXNE, ENABLE);
    USART_Cmd(JW01_USART, ENABLE);

    // NVIC
    NVIC_InitStructure.NVIC_IRQChannel                   = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

// 校验和验证：sum([1]..[7]) + [8] == 0xFF+1 = 256 (取低 8 位为 0)
static uint8_t JW01_CheckSum(void) {
    uint8_t sum = 0;
    uint8_t i;
    for (i = 1; i < 8; i++) {
        sum += s_buf[i];
    }
    // 校验：(~sum + 1) 应等于 s_buf[8]
    return ((uint8_t)(~sum + 1) == s_buf[8]);
}

// 在 USART1_IRQHandler 中调用
void JW01_IRQHandler(void) {
    if (USART_GetITStatus(JW01_USART, USART_IT_RXNE) == RESET) {
        return;
    }

    uint8_t byte = (uint8_t)USART_ReceiveData(JW01_USART);

    if (!s_in_frame) {
        // 等待帧头第一字节 0xFF
        if (byte == JW01_HEADER1) {
            s_buf[0]   = byte;
            s_idx      = 1;
            s_in_frame = 1;
        }
    } else {
        s_buf[s_idx++] = byte;

        // 帧头第二字节不是 0x86 则重置
        if (s_idx == 2 && byte != JW01_HEADER2) {
            s_in_frame = 0;
            s_idx      = 0;
            return;
        }

        // 接收完整帧
        if (s_idx >= JW01_FRAME_LEN) {
            s_in_frame = 0;
            s_idx      = 0;

            if (JW01_CheckSum()) {
                s_voc_ppb  = ((uint16_t)s_buf[2] << 8) | s_buf[3];
                s_new_data = 1;
            }
        }
    }
}

// 获取最新 VOC 值（有新数据返回 1，否则返回 0）
uint8_t JW01_GetData(uint16_t *voc_ppb) {
    if (!s_new_data) {
        return 0;
    }
    *voc_ppb   = s_voc_ppb;
    s_new_data = 0;
    return 1;
}
