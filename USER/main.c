#include "stm32f10x.h"
#include <stdio.h>
#include "delay.h"
#include "sys.h"
#include "key.h"
#include "oled.h"
#include "sg90.h"
#include "beep.h"

int main(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    delay_init();
    App_TickInit();

    // 外设初始化
    KEY_Init();             // PC13/PC14/PC15/PA15 按键（同时禁用JTAG）
    BEEP_GPIO_Config();     // PB12 蜂鸣器
    OLED_Init();            // PB10/PB11 软件I2C OLED
    SG90_Init();            // TIM3 CH1-4：PA6/PA7/PB0/PB1 舵机
	
    // 开机欢迎界面，同时等待红外传感器稳定
    OLED_Clear();
    OLED_ShowString(4, 4, (uint8_t *)"Initializing...");
    delay_ms(1500);
    OLED_Clear();

    while (1) {

    }
}
