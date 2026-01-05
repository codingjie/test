#include "led.h"
#include "stm32f10x.h"
#include "tim.h"
#include "delay.h"
#include "traffic_light.h"

void RCC_Configuration(void) {
    RCC_DeInit();

    RCC_HSEConfig(RCC_HSE_ON); // 开启外部晶振
    while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);

    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div2);
    RCC_PCLK2Config(RCC_HCLK_Div1);

    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); // 8MHz * 9 = 72MHz
    RCC_PLLCmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    while (RCC_GetSYSCLKSource() != 0x08);
}

int main(void) {
    RCC_Configuration(); // 时钟初始化

    delay_init();
    LED_Init();
    TrafficLight_Init(); // 初始化交通灯系统
    TIM2_Int_Init(); // 启动定时器中断

    while (1) {
        // 主循环中无需操作，所有控制逻辑在定时器中断中完成
    }
}
