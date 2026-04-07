#include "stm32f10x.h"
#include <stdio.h>
#include "delay.h"
#include "sys.h"
#include "led.h"
#include "beep.h"
#include "key.h"
#include "relay.h"
#include "ir.h"
#include "usart.h"
#include "jy61p.h"
#include "oled.h"
#include "sg90.h"

int main(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    delay_init();
    App_TickInit();          // TIM4 1ms 节拍

    // 外设初始化
    LED_Init();              // PA0(G1常亮) PA2(Y) PA3(R1) PA4(R2) PA6(B1) PB0(G2) PB1(B2)
    BEEP_GPIO_Config();      // PA5 蜂鸣器
    KEY_Init();              // PB12~PB15 按键（高电平有效）
    RELAY_Init();            // PC4(继电器1) PC5(继电器2)
    IR_Init();               // PA1(TIM2_CH2 38kHz发送) PB8(EXTI接收)
    USART1_Init(115200);     // PA9/PA10 调试串口
    JY61P_Init();            // PB10/PB11 USART3 姿态传感器
    OLED_Init();             // PB6(SCL) PB7(SDA) 软件I2C
    SG90_Init();             // PA6 TIM3_CH1 舵机（初始化后PA6切为复用推挽）

    // 开机欢迎
    OLED_Clear();
    OLED_ShowString(4, 4, (uint8_t *)"Initializing...");
    printf("System started\r\n");
    delay_ms(1500);
    OLED_Clear();

    while (1) {
        uint8_t key = KEY_Scan();

        // KEY1: 工作模式
        if (key == 1) {
        }

        // KEY2: 测试模式 → 红色LED1亮
        if (key == 2) {
            LED_RED1_ON();
        }

        // KEY3: 重置
        if (key == 3) {
            LED_RED1_OFF();
            RELAY1_OFF();
            SG90_Close();
        }

        // KEY4: 启动积分
        if (key == 4) {
        }

        // 1Hz 黄灯闪烁 + 蜂鸣器
        {
            static uint32_t last_toggle = 0;
            if ((g_tick_ms - last_toggle) >= 500) {
                last_toggle = g_tick_ms;
                LED_YELLOW_TOG();
                if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_2))
                    BEEP_ON();
                else
                    BEEP_OFF();
            }
        }
    }
}
