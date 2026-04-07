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
#include "sd.h"

// ================================================================
// 系统模式
// ================================================================
typedef enum { MODE_IDLE = 0, MODE_WORK, MODE_TEST } SysMode_t;
static SysMode_t sys_mode = MODE_IDLE;

// ================================================================
// 工作模式运行时状态
// ================================================================
static uint8_t  wm_started    = 0;  // IR信号已收到，序列运行中
static uint32_t wm_t0         = 0;  // 收到IR时的 g_tick_ms
static uint8_t  wm_relay      = 0;  // 继电器已触发
static uint8_t  wm_servo_cmd  = 0;  // 舵机PWM命令已发出
static uint32_t wm_servo_t0   = 0;  // 发出舵机命令时的 g_tick_ms
static uint8_t  wm_servo_type = 0;  // 1=z≥5m触发  2=8.5s强制触发
static uint8_t  wm_led_set    = 0;  // 舵机完成后LED已配置
static uint8_t  wm_done       = 0;  // 40s完成

// ================================================================
// 积分状态（每20ms一次，50Hz）
// ================================================================
static float    int_vx = 0, int_vy = 0, int_vz = 0;  // 速度 m/s
static float    int_px = 0, int_py = 0, int_pz = 0;  // 位移 m
static uint32_t int_tick  = 0;   // 上次积分的 g_tick_ms
static uint32_t log_tick  = 0;   // 上次写SD卡的 g_tick_ms

// ================================================================
// 1Hz 黄灯+蜂鸣器闪烁
// ================================================================
static uint8_t  blink_en   = 0;
static uint32_t blink_tick = 0;

// ================================================================
// OLED 辅助（避免不必要的刷新）
// ================================================================
static uint8_t  oled_cd_last = 0xFF;  // 上次显示的倒计时值
static uint32_t oled_tick    = 0;     // 上次刷新位移显示的 g_tick_ms

// ================================================================
// PA6 重配为 GPIO 输出（舵机完成后驱动蓝灯）
// 调用后 TIM3_CH1 PWM 停止，舵机靠摩擦力维持位置
// ================================================================
static void pa6_gpio(uint8_t high)
{
    GPIO_InitTypeDef gi;
    TIM_CCxCmd(TIM3, TIM_Channel_1, TIM_CCx_Disable);
    gi.GPIO_Pin   = GPIO_Pin_6;
    gi.GPIO_Mode  = GPIO_Mode_Out_PP;
    gi.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gi);
    GPIO_WriteBit(GPIOA, GPIO_Pin_6, high ? Bit_SET : Bit_RESET);
}

// ================================================================
// 重置工作模式所有状态
// ================================================================
static void wm_reset(void)
{
    if (wm_started) SD_Close();
    wm_started   = 0;
    wm_relay     = 0;
    wm_servo_cmd = 0;
    wm_servo_type = 0;
    wm_led_set   = 0;
    wm_done      = 0;
    blink_en     = 0;
    int_vx = int_vy = int_vz = 0.0f;
    int_px = int_py = int_pz = 0.0f;
    oled_cd_last = 0xFF;
    RELAY1_OFF();
    LED_YELLOW_OFF();
    BEEP_OFF();
    LED_RED1_OFF();
    SG90_Init();    // 同时将 PA6 重配为 TIM3_CH1 复用推挽
}

// ================================================================
// OLED 固定界面
// ================================================================
static void oled_idle(void)
{
    OLED_Clear();
    OLED_ShowString(16, 0, (uint8_t *)"System  Ready");
    OLED_ShowString(0,  2, (uint8_t *)"KEY1: Work Mode");
    OLED_ShowString(0,  4, (uint8_t *)"KEY2: Test Mode");
    OLED_ShowString(0,  6, (uint8_t *)"KEY3: Reset");
}

static void oled_wait_ir(void)
{
    OLED_Clear();
    OLED_ShowString(20, 0, (uint8_t *)"Work Mode");
    OLED_ShowString(0,  3, (uint8_t *)"Wait IR signal..");
}

// ================================================================
// main
// ================================================================
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    delay_init();
    App_TickInit();          // TIM4 1ms 节拍

    LED_Init();              // PA0 常亮
    BEEP_GPIO_Config();      // PA5
    KEY_Init();              // PB12~PB15 高电平有效
    RELAY_Init();            // PC4 PC5
    IR_Init();               // PA1(TX) PB8(RX)
    USART1_Init(115200);     // PA9/PA10 调试串口，printf 已重定向
    JY61P_Init();            // PB10/PB11 USART3
    OLED_Init();             // PB6/PB7 软件I2C
    SG90_Init();             // PA6 TIM3_CH1
    SD_Init();               // SDIO + FatFS 挂载

    OLED_Clear();
    OLED_ShowString(4, 4, (uint8_t *)"Initializing...");
    printf("System started\r\n");
    delay_ms(1500);
    oled_idle();

    while (1) {
        uint8_t key = KEY_Scan();

        // KEY1: 进入/重启工作模式
        if (key == 1) {
            sys_mode = MODE_WORK;
            wm_reset();
            oled_wait_ir();
        }

        // KEY2: 测试模式 → PA3 红灯亮
        if (key == 2) {
            sys_mode = MODE_TEST;
            wm_reset();
            LED_RED1_ON();
            OLED_Clear();
            OLED_ShowString(20, 0, (uint8_t *)"Test Mode");
            OLED_ShowString(0,  3, (uint8_t *)"Red LED: ON");
        }

        // KEY3: 重置 → 空闲
        if (key == 3) {
            sys_mode = MODE_IDLE;
            wm_reset();
            LED_RED1_OFF();
            oled_idle();
        }

        // ============================================================
        // 工作模式逻辑
        // ============================================================
        if (sys_mode == MODE_WORK) {

            // ---- 触发：收到IR信号 ----
            if (!wm_started && IR_RX_HasData()) {
                wm_started   = 1;
                wm_t0        = g_tick_ms;
                blink_en     = 1;
                blink_tick   = g_tick_ms;
                int_tick     = g_tick_ms;
                log_tick     = g_tick_ms;
                oled_cd_last = 0xFF;
                int_vx = int_vy = int_vz = 0.0f;
                int_px = int_py = int_pz = 0.0f;
                if (SD_Open("DATA.CSV"))
                    SD_WriteHeader();
                printf("IR received: sequence started\r\n");
            }

            if (wm_started && !wm_done) {
                uint32_t elapsed = g_tick_ms - wm_t0;

                // ---- 加速度积分 @ 50Hz（每20ms）----
                // JY61P az 包含重力(静止时az≈1g)，减去1g得运动加速度
                if ((g_tick_ms - int_tick) >= 20) {
                    float dt = (float)(g_tick_ms - int_tick) * 0.001f;
                    int_tick = g_tick_ms;

                    float ax = jy61p_data.ax * 9.81f;
                    float ay = jy61p_data.ay * 9.81f;
                    float az = (jy61p_data.az - 1.0f) * 9.81f;  // 减去重力

                    int_vx += ax * dt;  int_px += int_vx * dt;
                    int_vy += ay * dt;  int_py += int_vy * dt;
                    int_vz += az * dt;  int_pz += int_vz * dt;
                }

                // ---- 写SD卡 @ 20Hz（每50ms）----
                if ((g_tick_ms - log_tick) >= 50) {
                    log_tick = g_tick_ms;
                    SD_Log(elapsed,
                           jy61p_data.ax, jy61p_data.ay, jy61p_data.az,
                           int_vx, int_vy, int_vz,
                           int_px, int_py, int_pz);
                }

                // ---- OLED 倒计时（0~3s，每秒刷新一次）----
                if (elapsed < 3000) {
                    uint8_t cd = (uint8_t)(3 - elapsed / 1000);
                    if (cd != oled_cd_last) {
                        oled_cd_last = cd;
                        OLED_Clear();
                        OLED_ShowString(20, 0, (uint8_t *)"Work Mode");
                        OLED_ShowString(0,  2, (uint8_t *)"Countdown:");
                        OLED_ShowNum(66,  2, (int32_t)cd, 1);
                        OLED_ShowString(0,  4, (uint8_t *)"Integrating...");
                    }
                }

                // ---- 3s：继电器接通，停止黄灯+蜂鸣器 ----
                if (!wm_relay && elapsed >= 3000) {
                    wm_relay = 1;
                    RELAY1_ON();
                    blink_en = 0;
                    LED_YELLOW_OFF();
                    BEEP_OFF();
                    oled_tick = g_tick_ms;
                    OLED_Clear();
                    OLED_ShowString(20, 0, (uint8_t *)"Work Mode");
                    OLED_ShowString(0,  2, (uint8_t *)"Relay: ON");
                    printf("Relay ON (t=3s)\r\n");
                }

                // ---- 继电器已触发后：每500ms刷新位移+时间显示 ----
                if (wm_relay && !wm_servo_cmd &&
                    (g_tick_ms - oled_tick) >= 500) {
                    oled_tick = g_tick_ms;
                    // 只更新第4、6行，不清屏，避免闪烁
                    OLED_ShowString(0, 4, (uint8_t *)"Z:");
                    OLED_ShowFloat(12, 4, int_pz, 2);
                    OLED_ShowString(72, 4, (uint8_t *)"m   ");
                    OLED_ShowString(0, 6, (uint8_t *)"T:");
                    OLED_ShowNum(12, 6, (int32_t)(elapsed / 1000), 2);
                    OLED_ShowString(30, 6, (uint8_t *)"s  ");
                }

                // ---- z轴位移≥5m：舵机旋转90°（继电器触发后，仅触发一次）----
                if (wm_relay && !wm_servo_cmd && int_pz >= 5.0f) {
                    SG90_SetAngle(90);
                    wm_servo_cmd  = 1;
                    wm_servo_t0   = g_tick_ms;
                    wm_servo_type = 1;
                    printf("Servo: z>=5m triggered\r\n");
                }

                // ---- 8.5s强制：若舵机尚未转动，强制转90° ----
                if (wm_relay && !wm_servo_cmd && elapsed >= 8500) {
                    SG90_SetAngle(90);
                    wm_servo_cmd  = 1;
                    wm_servo_t0   = g_tick_ms;
                    wm_servo_type = 2;
                    printf("Servo: forced at 8.5s\r\n");
                }

                // ---- 舵机命令发出500ms后配置LED ----
                // SG90从0°转到90°约需300~500ms
                if (wm_servo_cmd && !wm_led_set &&
                    (g_tick_ms - wm_servo_t0) >= 500) {
                    wm_led_set = 1;
                    if (wm_servo_type == 1) {
                        // z≥5m 触发：PA6 蓝灯亮
                        pa6_gpio(1);
                        OLED_Clear();
                        OLED_ShowString(20, 0, (uint8_t *)"Work Mode");
                        OLED_ShowString(0,  2, (uint8_t *)"Servo: z>=5m");
                        OLED_ShowString(0,  4, (uint8_t *)"Blue LED ON");
                    } else {
                        // 8.5s强制：PA6 蓝灯不亮，PA3 红灯亮
                        pa6_gpio(0);
                        LED_RED1_ON();
                        OLED_Clear();
                        OLED_ShowString(20, 0, (uint8_t *)"Work Mode");
                        OLED_ShowString(0,  2, (uint8_t *)"Servo: Forced");
                        OLED_ShowString(0,  4, (uint8_t *)"Red LED ON");
                    }
                }

                // ---- 40s：停止积分，关闭SD卡，工作结束 ----
                if (elapsed >= 40000) {
                    wm_done = 1;
                    SD_Close();
                    // 此后主循环不再执行积分和日志
                    OLED_Clear();
                    OLED_ShowString(20, 0, (uint8_t *)"Work Mode");
                    OLED_ShowString(0,  2, (uint8_t *)"Done! (40s)");
                    OLED_ShowString(0,  4, (uint8_t *)"Remove SD Card");
                    printf("Work done. Remove SD card.\r\n");
                }
            }
        } // end MODE_WORK

        // ============================================================
        // 1Hz 黄灯 + 蜂鸣器（每500ms切换一次）
        // ============================================================
        if ((g_tick_ms - blink_tick) >= 500) {
            blink_tick = g_tick_ms;
            if (blink_en) {
                LED_YELLOW_TOG();
                // 灯亮时蜂鸣器响，灯灭时蜂鸣器停
                if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_2))
                    BEEP_ON();
                else
                    BEEP_OFF();
            }
        }

    } // end while(1)
}
