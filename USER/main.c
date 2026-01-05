#include "led.h"
#include "stm32f10x.h"
#include "tim.h"
#include "delay.h"

// 状态定义
typedef enum {
    SN_GREEN_STATE,  // 南北绿，东西红
    SN_YELLOW_STATE, // 南北黄闪，东西红
    EW_GREEN_STATE,  // 东西绿，南北红
    EW_YELLOW_STATE  // 东西黄闪，南北红
} TrafficState;

TrafficState current_state = SN_GREEN_STATE;

// 声明外部中断变量
extern volatile uint32_t ms_tick;
extern volatile uint16_t seconds_left;

void All_Off(void) {
    SNGREEN_OFF;
    SNRED_OFF;
    SNYELLOW_OFF;
    EWGREEN_OFF;
    EWRED_OFF;
    EWYELLOW_OFF;
}

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
    // RCC_Configuration(); // 时钟初始化

    delay_init();
    LED_Init();
    TIM2_Int_Init(); // 内部已设置 seconds_left = 15

    // 初始状态：南北绿(12s)，东西红(12+3=15s)
    current_state = SN_GREEN_STATE;
    seconds_left = 12;

    while (1) {
        // 状态切换逻辑
        if (seconds_left == 0) {
            switch (current_state) {
            case SN_GREEN_STATE:
                current_state = SN_YELLOW_STATE;
                seconds_left = 3;
                break;
            case SN_YELLOW_STATE:
                current_state = EW_GREEN_STATE;
                seconds_left = 12;
                break;
            case EW_GREEN_STATE:
                current_state = EW_YELLOW_STATE;
                seconds_left = 3;
                break;
            case EW_YELLOW_STATE:
                current_state = SN_GREEN_STATE;
                seconds_left = 12;
                break;
            }
        }

        // 3. 各状态灯效控制
        switch (current_state) {
            case SN_GREEN_STATE:
                All_Off();
                SNGREEN_ON;
                EWRED_ON;
                break;

            case SN_YELLOW_STATE:
                All_Off();
                EWRED_ON; // 东西向继续红灯
                // 0.5s 闪烁：1s内的前500ms亮，后500ms灭
                if ((ms_tick % 1000) < 500)
                    SNYELLOW_ON;
                else
                    SNYELLOW_OFF;
                break;

            case EW_GREEN_STATE:
                All_Off();
                EWGREEN_ON;
                SNRED_ON;
                break;

            case EW_YELLOW_STATE:
                All_Off();
                SNRED_ON; // 南北向继续红灯
                if ((ms_tick % 1000) < 500)
                    EWYELLOW_ON;
                else
                    EWYELLOW_OFF;
                break;
        }
    }
}
