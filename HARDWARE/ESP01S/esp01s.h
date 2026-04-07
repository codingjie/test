#ifndef __ESP01S_H
#define __ESP01S_H

#include "stm32f10x.h"
#include "stm32f10x_usart.h"

// ESP01S Wi-Fi 模块，接 USART2（PA2=TX，PA3=RX，3.3V 电平直连）
// 使用 ESP-AT 固件，波特率 115200
// 功能：连接 Wi-Fi → MQTT 连接 → 上报传感器数据 → 接收远程控制指令

#define ESP_USART          USART2
#define ESP_USART_CLK      RCC_APB1Periph_USART2
#define ESP_GPIO_CLK       RCC_APB2Periph_GPIOA
#define ESP_TX_PIN         GPIO_Pin_2     // PA2 -> USART2_TX -> ESP01S RX
#define ESP_RX_PIN         GPIO_Pin_3     // PA3 -> USART2_RX -> ESP01S TX
#define ESP_GPIO_PORT      GPIOA
#define ESP_BAUD           115200

// 接收环形缓冲区大小
#define ESP_RX_BUF_SIZE    512

// Wi-Fi 和 MQTT 配置（根据实际环境修改）
#define ESP_WIFI_SSID      "YourWiFiSSID"
#define ESP_WIFI_PWD       "YourWiFiPwd"
#define MQTT_BROKER        "broker.emqx.io"
#define MQTT_PORT          1883
#define MQTT_CLIENT_ID     "charger_stm32_01"
#define MQTT_USERNAME      ""
#define MQTT_PASSWORD      ""
#define MQTT_TOPIC_PUB     "battery/charger01/data"
#define MQTT_TOPIC_SUB     "battery/charger01/control"

// 初始化 USART2
void ESP01S_Init(void);

// USART2 中断中调用（缓存接收字节）
void ESP01S_IRQHandler(void);

// 连接 Wi-Fi（阻塞，超时返回 0）
uint8_t ESP01S_ConnectWiFi(void);

// 建立 MQTT 连接（阻塞，超时返回 0）
uint8_t ESP01S_MQTTConnect(void);

// MQTT 订阅控制主题
uint8_t ESP01S_MQTTSubscribe(void);

// 发布 MQTT 消息
uint8_t ESP01S_MQTTPublish(const char *payload);

// 主循环中调用：处理接收缓冲区中的 MQTT 下行消息
// 若收到风扇控制指令则通过 fan_cmd 返回 0/1/2，无指令返回 0xFF
uint8_t ESP01S_Process(uint8_t *fan_cmd);

// 查询连接状态
uint8_t ESP01S_IsWiFiConnected(void);
uint8_t ESP01S_IsMQTTConnected(void);

#endif
