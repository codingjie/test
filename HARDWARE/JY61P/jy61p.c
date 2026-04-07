#include "jy61p.h"

// JY61P 姿态传感器 USART3
// PB10: USART3_TX → JY61P RX
// PB11: USART3_RX → JY61P TX
// 9600-8N1
//
// 解析帧:
//   0x55 0x51 → 加速度
//   0x55 0x52 → 角速度
//   0x55 0x53 → 角度（欧拉角）

volatile JY61P_Data_t jy61p_data;

static uint8_t rx_buf[11];
static uint8_t rx_idx = 0;

static void parse_frame(void)
{
    int16_t d0, d1, d2;

    // 校验和
    uint8_t sum = 0;
    uint8_t i;
    for (i = 0; i < 10; i++) sum += rx_buf[i];
    if (sum != rx_buf[10]) return;

    d0 = (int16_t)((rx_buf[3] << 8) | rx_buf[2]);
    d1 = (int16_t)((rx_buf[5] << 8) | rx_buf[4]);
    d2 = (int16_t)((rx_buf[7] << 8) | rx_buf[6]);

    switch (rx_buf[1]) {
    case 0x51:   // 加速度 ±16g
        jy61p_data.ax = d0 / 32768.0f * 16.0f;
        jy61p_data.ay = d1 / 32768.0f * 16.0f;
        jy61p_data.az = d2 / 32768.0f * 16.0f;
        break;
    case 0x52:   // 角速度 ±2000°/s
        jy61p_data.wx = d0 / 32768.0f * 2000.0f;
        jy61p_data.wy = d1 / 32768.0f * 2000.0f;
        jy61p_data.wz = d2 / 32768.0f * 2000.0f;
        break;
    case 0x53:   // 角度 ±180°
        jy61p_data.roll  = d0 / 32768.0f * 180.0f;
        jy61p_data.pitch = d1 / 32768.0f * 180.0f;
        jy61p_data.yaw   = d2 / 32768.0f * 180.0f;
        break;
    default:
        break;
    }
}

void JY61P_IRQHandler(void)
{
    uint8_t byte;

    if (USART_GetITStatus(USART3, USART_IT_RXNE) == RESET) return;
    byte = (uint8_t)USART_ReceiveData(USART3);

    if (rx_idx == 0) {
        if (byte != 0x55) return;   // 等帧头
    }
    rx_buf[rx_idx++] = byte;

    if (rx_idx == 11) {
        parse_frame();
        rx_idx = 0;
    }
}

void JY61P_Init(void)
{
    GPIO_InitTypeDef  gi;
    USART_InitTypeDef ui;
    NVIC_InitTypeDef  ni;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    // PB10: USART3_TX 复用推挽
    gi.GPIO_Pin   = GPIO_Pin_10;
    gi.GPIO_Mode  = GPIO_Mode_AF_PP;
    gi.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gi);

    // PB11: USART3_RX 浮空输入
    gi.GPIO_Pin  = GPIO_Pin_11;
    gi.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &gi);

    ui.USART_BaudRate            = 9600;
    ui.USART_WordLength          = USART_WordLength_8b;
    ui.USART_StopBits            = USART_StopBits_1;
    ui.USART_Parity              = USART_Parity_No;
    ui.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    ui.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART3, &ui);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    ni.NVIC_IRQChannel                   = USART3_IRQn;
    ni.NVIC_IRQChannelPreemptionPriority = 3;
    ni.NVIC_IRQChannelSubPriority        = 0;
    ni.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&ni);

    USART_Cmd(USART3, ENABLE);
}
