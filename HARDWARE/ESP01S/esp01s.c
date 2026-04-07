#include "esp01s.h"
#include "delay.h"
#include <string.h>
#include <stdio.h>

// 接收环形缓冲区
static uint8_t  s_rx_buf[ESP_RX_BUF_SIZE];
static uint16_t s_rx_head = 0;   // 写指针（中断更新）
static uint16_t s_rx_tail = 0;   // 读指针（主循环更新）

// 连接状态标志
static uint8_t s_wifi_ok = 0;
static uint8_t s_mqtt_ok = 0;

// -----------------------------------------------------------------------
// 底层 USART 发送
// -----------------------------------------------------------------------

static void ESP_SendByte(uint8_t byte) {
    while (USART_GetFlagStatus(ESP_USART, USART_FLAG_TXE) == RESET);
    USART_SendData(ESP_USART, byte);
}

static void ESP_SendStr(const char *str) {
    while (*str) {
        ESP_SendByte((uint8_t)*str++);
    }
}

// 清空接收缓冲区
static void ESP_FlushRx(void) {
    s_rx_head = 0;
    s_rx_tail = 0;
}

// 从缓冲区读取一行（以 \n 结尾），返回实际字节数（不含 \n）
static uint16_t ESP_ReadLine(char *out, uint16_t max_len, uint32_t timeout_ms) {
    extern volatile uint32_t g_tick_ms;
    uint32_t t0 = g_tick_ms;
    uint16_t n  = 0;

    while (1) {
        if (s_rx_tail != s_rx_head) {
            uint8_t byte = s_rx_buf[s_rx_tail];
            s_rx_tail = (s_rx_tail + 1) % ESP_RX_BUF_SIZE;

            if (byte == '\n') {
                break;
            }
            if (byte != '\r' && n < max_len - 1) {
                out[n++] = (char)byte;
            }
        }

        if ((g_tick_ms - t0) > timeout_ms) {
            break;
        }
    }

    out[n] = '\0';
    return n;
}

// 等待缓冲区中出现期望字符串（返回 1 找到，0 超时）
static uint8_t ESP_WaitFor(const char *expect, uint32_t timeout_ms) {
    extern volatile uint32_t g_tick_ms;
    char line[128];
    uint32_t t0 = g_tick_ms;

    while ((g_tick_ms - t0) < timeout_ms) {
        if (ESP_ReadLine(line, sizeof(line), 200) > 0) {
            if (strstr(line, expect) != NULL) {
                return 1;
            }
            // 检测断线事件
            if (strstr(line, "MQTTDISCONN") != NULL) {
                s_mqtt_ok = 0;
            }
        }
    }
    return 0;
}

// 发送 AT 命令并等待期望响应
static uint8_t ESP_SendCmd(const char *cmd, const char *expect, uint32_t timeout_ms) {
    ESP_FlushRx();
    ESP_SendStr(cmd);
    ESP_SendStr("\r\n");
    return ESP_WaitFor(expect, timeout_ms);
}

// -----------------------------------------------------------------------
// 初始化
// -----------------------------------------------------------------------

void ESP01S_Init(void) {
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(ESP_GPIO_CLK, ENABLE);
    RCC_APB1PeriphClockCmd(ESP_USART_CLK, ENABLE);

    // PA2 复用推挽输出（USART2_TX）
    GPIO_InitStructure.GPIO_Pin   = ESP_TX_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ESP_GPIO_PORT, &GPIO_InitStructure);

    // PA3 浮空输入（USART2_RX）
    GPIO_InitStructure.GPIO_Pin  = ESP_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(ESP_GPIO_PORT, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate            = ESP_BAUD;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(ESP_USART, &USART_InitStructure);

    USART_ITConfig(ESP_USART, USART_IT_RXNE, ENABLE);
    USART_Cmd(ESP_USART, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel                   = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

// USART2 中断服务（在 stm32f10x_it.c 中调用）
void ESP01S_IRQHandler(void) {
    if (USART_GetITStatus(ESP_USART, USART_IT_RXNE) == RESET) {
        return;
    }
    uint8_t byte = (uint8_t)USART_ReceiveData(ESP_USART);
    uint16_t next = (s_rx_head + 1) % ESP_RX_BUF_SIZE;
    if (next != s_rx_tail) {       // 防止溢出
        s_rx_buf[s_rx_head] = byte;
        s_rx_head = next;
    }
}

// -----------------------------------------------------------------------
// Wi-Fi 连接
// -----------------------------------------------------------------------

uint8_t ESP01S_ConnectWiFi(void) {
    char cmd[96];

    // 重置模块
    ESP_SendCmd("AT+RST", "ready", 3000);
    delay_ms(500);

    // 设为 Station 模式
    if (!ESP_SendCmd("AT+CWMODE=1", "OK", 2000)) {
        return 0;
    }

    // 连接 Wi-Fi
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", ESP_WIFI_SSID, ESP_WIFI_PWD);
    if (!ESP_SendCmd(cmd, "WIFI GOT IP", 10000)) {
        return 0;
    }

    s_wifi_ok = 1;
    return 1;
}

// -----------------------------------------------------------------------
// MQTT 连接
// -----------------------------------------------------------------------

uint8_t ESP01S_MQTTConnect(void) {
    char cmd[128];

    // 配置 MQTT 用户信息
    snprintf(cmd, sizeof(cmd),
             "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"",
             MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);
    if (!ESP_SendCmd(cmd, "OK", 3000)) {
        return 0;
    }

    // 连接 MQTT Broker
    snprintf(cmd, sizeof(cmd),
             "AT+MQTTCONN=0,\"%s\",%d,1",
             MQTT_BROKER, MQTT_PORT);
    if (!ESP_SendCmd(cmd, "OK", 8000)) {
        return 0;
    }

    s_mqtt_ok = 1;
    return 1;
}

// -----------------------------------------------------------------------
// MQTT 订阅控制主题
// -----------------------------------------------------------------------

uint8_t ESP01S_MQTTSubscribe(void) {
    char cmd[80];
    snprintf(cmd, sizeof(cmd), "AT+MQTTSUB=0,\"%s\",1", MQTT_TOPIC_SUB);
    return ESP_SendCmd(cmd, "OK", 3000);
}

// -----------------------------------------------------------------------
// MQTT 发布
// -----------------------------------------------------------------------

uint8_t ESP01S_MQTTPublish(const char *payload) {
    char cmd[320];
    if (!s_mqtt_ok) {
        return 0;
    }
    // AT+MQTTPUB=0,"topic","payload",qos,retain
    snprintf(cmd, sizeof(cmd),
             "AT+MQTTPUB=0,\"%s\",\"%s\",1,0",
             MQTT_TOPIC_PUB, payload);
    return ESP_SendCmd(cmd, "OK", 3000);
}

// -----------------------------------------------------------------------
// 主循环处理：解析下行 MQTT 消息，提取风扇控制指令
// 消息格式：+MQTTSUBRECV:0,"topic",len,{"fan":N}
// 返回 0xFF = 无新指令；返回 0/1/2 = 风扇档位指令
// -----------------------------------------------------------------------

uint8_t ESP01S_Process(uint8_t *fan_cmd) {
    char line[128];
    char *p;

    *fan_cmd = 0xFF;    // 默认无指令

    if (ESP_ReadLine(line, sizeof(line), 10) == 0) {
        return 0;
    }

    // 检测 MQTT 断线
    if (strstr(line, "MQTTDISCONN") != NULL) {
        s_mqtt_ok = 0;
        return 0;
    }

    // 解析 MQTT 下行消息
    if (strstr(line, "+MQTTSUBRECV") == NULL) {
        return 0;
    }

    // 在行中查找 "fan": 后面的数字
    p = strstr(line, "\"fan\":");
    if (p == NULL) {
        p = strstr(line, "\"fan\": ");
    }
    if (p != NULL) {
        p += 6;
        while (*p == ' ' || *p == ':') p++;   // 跳过空格/冒号
        if (*p >= '0' && *p <= '2') {
            *fan_cmd = (uint8_t)(*p - '0');
            return 1;
        }
    }
    return 0;
}

// -----------------------------------------------------------------------
// 状态查询
// -----------------------------------------------------------------------

uint8_t ESP01S_IsWiFiConnected(void) {
    return s_wifi_ok;
}

uint8_t ESP01S_IsMQTTConnected(void) {
    return s_mqtt_ok;
}
