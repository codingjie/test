#include "traffic_light.h"

// 全局变量定义
TrafficLightState current_state = STATE_SN_GREEN_EW_RED;
volatile uint16_t state_timer = GREEN_TIME;
volatile uint8_t yellow_blink_flag = 0;

/**
 * @brief  初始化交通灯系统
 * @param  None
 * @retval None
 */
void TrafficLight_Init(void) {
    current_state = STATE_SN_GREEN_EW_RED;
    state_timer = GREEN_TIME;
    yellow_blink_flag = 0;

    // 初始状态：南北绿灯，东西红灯
    SNGREEN_ON;
    SNRED_OFF;
    SNYELLOW_OFF;

    EWGREEN_OFF;
    EWRED_ON;
    EWYELLOW_OFF;
}

/**
 * @brief  交通灯状态机，处理状态转换
 * @param  None
 * @retval None
 * @note   每秒调用一次
 */
void TrafficLight_StateMachine(void) {
    if (state_timer == 0) {
        // 状态转换
        switch (current_state) {
            case STATE_SN_GREEN_EW_RED:
                // 南北绿灯结束 -> 南北黄灯
                current_state = STATE_SN_YELLOW_EW_RED;
                state_timer = YELLOW_TIME;
                break;

            case STATE_SN_YELLOW_EW_RED:
                // 南北黄灯结束 -> 南北红灯，东西绿灯
                current_state = STATE_SN_RED_EW_GREEN;
                state_timer = GREEN_TIME;
                break;

            case STATE_SN_RED_EW_GREEN:
                // 东西绿灯结束 -> 东西黄灯
                current_state = STATE_SN_RED_EW_YELLOW;
                state_timer = YELLOW_TIME;
                break;

            case STATE_SN_RED_EW_YELLOW:
                // 东西黄灯结束 -> 南北绿灯，东西红灯
                current_state = STATE_SN_GREEN_EW_RED;
                state_timer = GREEN_TIME;
                break;
        }
    }
}

/**
 * @brief  更新交通灯显示
 * @param  blink_phase: 闪烁相位（0或1，用于黄灯闪烁）
 * @retval None
 * @note   根据当前状态和闪烁相位控制LED
 */
void TrafficLight_UpdateLights(uint8_t blink_phase) {
    switch (current_state) {
        case STATE_SN_GREEN_EW_RED:
            // 南北：绿灯亮
            SNGREEN_ON;
            SNRED_OFF;
            SNYELLOW_OFF;

            // 东西：红灯亮
            EWGREEN_OFF;
            EWRED_ON;
            EWYELLOW_OFF;
            break;

        case STATE_SN_YELLOW_EW_RED:
            // 南北：黄灯闪烁
            SNGREEN_OFF;
            SNRED_OFF;
            if (blink_phase) {
                SNYELLOW_ON;
            } else {
                SNYELLOW_OFF;
            }

            // 东西：红灯亮
            EWGREEN_OFF;
            EWRED_ON;
            EWYELLOW_OFF;
            break;

        case STATE_SN_RED_EW_GREEN:
            // 南北：红灯亮
            SNGREEN_OFF;
            SNRED_ON;
            SNYELLOW_OFF;

            // 东西：绿灯亮
            EWGREEN_ON;
            EWRED_OFF;
            EWYELLOW_OFF;
            break;

        case STATE_SN_RED_EW_YELLOW:
            // 南北：红灯亮
            SNGREEN_OFF;
            SNRED_ON;
            SNYELLOW_OFF;

            // 东西：黄灯闪烁
            EWGREEN_OFF;
            EWRED_OFF;
            if (blink_phase) {
                EWYELLOW_ON;
            } else {
                EWYELLOW_OFF;
            }
            break;
    }
}
