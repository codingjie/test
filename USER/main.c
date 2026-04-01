#include "stm32f10x.h"
#include "delay.h"
#include "oled.h"
#include "adc.h"
#include "motor.h"
#include "beep.h"
#include "infra.h"
#include "dht11.h"
#include "usart.h"
#include "key.h"

/* ------------------------------------------------------------------ */
/* 可调阈值（初始默认值）                                              */
/* ------------------------------------------------------------------ */
static int16_t thresh_temp  = 35;   /* 温度上限  ℃  */
static int16_t thresh_humi  = 35;   /* 土壤湿度下限 % */
static int16_t thresh_light = 80;   /* 光照强度上限   */

/* ------------------------------------------------------------------ */
/* 系统状态                                                            */
/* ------------------------------------------------------------------ */
static uint8_t mode           = 0;  /* 0=自动  1=手动 */
static uint8_t selected_param = 0;  /* 0=无  1=温度  2=土壤  3=光敏 */
static uint8_t alarm_enabled  = 1;  /* 0=关报警  1=开报警 */

/* 手动模式设备状态 */
static uint8_t fan_on   = 0;
static uint8_t pump_on  = 0;
static uint8_t motor_on = 0;

/* 报警节拍计数 */
static uint8_t  alarm_active = 0;   /* 当前是否处于报警序列中 */
static uint8_t  alarm_beats  = 0;   /* 已完成的半周期数（最多 16 = 8 次鸣叫） */
static uint16_t alarm_tick   = 0;   /* 500 ms 节拍计数器（单位 10 ms）*/

/* 传感器读值 */
static DHT11_Sensor_Data_TypeDef dht11Data;
static uint8_t temp_value  = 0;
static uint8_t humi_value  = 0;
static uint8_t light_value = 0;

static char buf[32];

/* ------------------------------------------------------------------ */
/* 辅助：刷新 OLED 显示                                               */
/* ------------------------------------------------------------------ */
static void Display_Update(void) {
    /* 行 0：温度  （'>' 表示当前选中参数）*/
    sprintf(buf, "%cTemp:%-2dC T:%-3d",
            (selected_param == 1) ? '>' : ' ',
            temp_value, (int)thresh_temp);
    OLED_ShowString(0, 0, (uint8_t *)buf);

    /* 行 2：土壤湿度 */
    sprintf(buf, "%cHumi:%-2d%% T:%-3d",
            (selected_param == 2) ? '>' : ' ',
            humi_value, (int)thresh_humi);
    OLED_ShowString(0, 2, (uint8_t *)buf);

    /* 行 4：光照强度 */
    sprintf(buf, "%cLght:%-2d  T:%-3d",
            (selected_param == 3) ? '>' : ' ',
            light_value, (int)thresh_light);
    OLED_ShowString(0, 4, (uint8_t *)buf);

    /* 行 6：模式 + 报警 / 手动设备状态 */
    if (mode == 0) {
        /* 自动模式 */
        sprintf(buf, "%s ALM:%s      ",
                "AUTO",
                alarm_enabled ? "ON " : "OFF");
    } else {
        /* 手动模式：显示各设备开关 */
        sprintf(buf, "MANU M:%d F:%d P:%d   ",
                motor_on, fan_on, pump_on);
    }
    OLED_ShowString(0, 6, (uint8_t *)buf);
}

/* ------------------------------------------------------------------ */
/* 辅助：处理报警节拍（每 10 ms 调用一次）                            */
/* ------------------------------------------------------------------ */
static void Alarm_Tick(uint8_t condition_met) {
    /* 触发新的报警序列 */
    if (alarm_enabled && condition_met && !alarm_active) {
        alarm_active = 1;
        alarm_beats  = 0;
        alarm_tick   = 0;
        BEEP_ON();
    }

    /* 关闭报警开关时立即停止 */
    if (!alarm_enabled) {
        alarm_active = 0;
        alarm_beats  = 0;
        alarm_tick   = 0;
        BEEP_OFF();
        return;
    }

    if (!alarm_active) return;

    alarm_tick++;
    if (alarm_tick >= 50) {        /* 50 × 10 ms = 500 ms */
        alarm_tick = 0;
        alarm_beats++;
        if (alarm_beats >= 16) {   /* 16 个半周期 = 8 次鸣叫 */
            alarm_active = 0;
            alarm_beats  = 0;
            BEEP_OFF();
        } else {
            if (alarm_beats % 2 == 0) BEEP_ON();
            else                      BEEP_OFF();
        }
    }
}

/* ------------------------------------------------------------------ */
/* 辅助：自动模式设备控制                                             */
/* ------------------------------------------------------------------ */
static void Auto_Control(void) {
    /* 风扇：温度超上限时开 */
    if (temp_value > (uint8_t)thresh_temp)   MOTOR_FAN_ON();
    else                                      MOTOR_FAN_OFF();

    /* 水泵：土壤湿度低于下限时开 */
    if (humi_value < (uint8_t)thresh_humi)   MOTOR_PUMP_ON();
    else                                      MOTOR_PUMP_OFF();

    /* 马达/舵机：光照超上限时开 */
    if (light_value > (uint8_t)thresh_light) MOTOR_SERVO_ON();
    else                                      MOTOR_SERVO_OFF();
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */
int main(void) {
    uint16_t sensor_tick = 0;   /* 传感器刷新计数器（50 × 10 ms = 500 ms）*/

    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
    ADC1_GPIO_Config();
    ADC1_Config();
    delay_init();
    OLED_Init();
    MOTOR_GPIO_Config();
    BEEP_GPIO_Config();
    INFRA_GPIO_Config();
    DHT11_Sensor_GPIO_Configuration();
    USART_Config(9600);
    KEY_GPIO_Config();

    OLED_Clear();
    Display_Update();

    while (1) {
        /* ---- 按键扫描（每 10 ms）---- */
        uint8_t key = KEY_Scan();

        switch (key) {
            /* SET 短按：循环选中参数 (0→1→2→3→0) */
            case KEY1_SHORT:
                selected_param = (selected_param + 1) % 4;
                break;

            /* SET 长按：切换自动 / 手动模式 */
            case KEY1_LONG:
                mode = !mode;
                selected_param = 0;
                if (mode == 0) {
                    /* 回到自动模式，立即执行一次自动控制 */
                    Auto_Control();
                } else {
                    /* 进入手动模式，同步当前设备状态 */
                    fan_on   = 0;
                    pump_on  = 0;
                    motor_on = 0;
                    MOTOR_FAN_OFF();
                    MOTOR_PUMP_OFF();
                    MOTOR_SERVO_OFF();
                }
                break;

            /* INCREASE 短按 */
            case KEY2_SHORT:
                if (mode == 1) {
                    /* 手动模式：切换马达 */
                    motor_on = !motor_on;
                    if (motor_on) MOTOR_SERVO_ON(); else MOTOR_SERVO_OFF();
                } else {
                    /* 自动模式：调高选中参数阈值 */
                    if      (selected_param == 1) thresh_temp  += 10;
                    else if (selected_param == 2) thresh_humi  += 10;
                    else if (selected_param == 3) thresh_light += 10;
                }
                break;

            /* DECREASE 短按 */
            case KEY3_SHORT:
                if (mode == 1) {
                    /* 手动模式：切换风扇 */
                    fan_on = !fan_on;
                    if (fan_on) MOTOR_FAN_ON(); else MOTOR_FAN_OFF();
                } else {
                    /* 自动模式：调低选中参数阈值 */
                    if      (selected_param == 1) thresh_temp  -= 10;
                    else if (selected_param == 2) thresh_humi  -= 10;
                    else if (selected_param == 3) thresh_light -= 10;
                }
                break;

            /* DECREASE 长按：切换报警开 / 关 */
            case KEY3_LONG:
                alarm_enabled = !alarm_enabled;
                break;

            /* CONFIRM 短按：手动模式切换水泵 */
            case KEY4_SHORT:
                if (mode == 1) {
                    pump_on = !pump_on;
                    if (pump_on) MOTOR_PUMP_ON(); else MOTOR_PUMP_OFF();
                }
                break;

            default:
                break;
        }

        /* ---- 传感器读取与自动控制（每 500 ms）---- */
        sensor_tick++;
        if (sensor_tick >= 50) {
            sensor_tick = 0;

            /* 读 DHT11 温度 */
            if (DHT11_ReadSensorData(&dht11Data) == 1)
                temp_value = dht11Data.temperature_integer;

            /* 读 ADC */
            ADC1_ReadAll();
            humi_value  = (uint8_t)(ADCValues[0] * 100.0f / 4096);
            light_value = (uint8_t)(ADCValues[1] * 100.0f / 4096);

            /* 自动模式控制 */
            if (mode == 0) Auto_Control();

            /* 刷新 OLED */
            Display_Update();

            /* 串口上报 */
            sprintf(buf, "T:%dC H:%d%% L:%d [%s]\r\n",
                    temp_value, humi_value, light_value,
                    mode ? "MANU" : "AUTO");
            USART_SendString(buf);
        }

        /* ---- 报警节拍（每 10 ms 推进一次）---- */
        uint8_t alarm_cond = (temp_value  > (uint8_t)thresh_temp)  ||
                             (humi_value  < (uint8_t)thresh_humi)  ||
                             (light_value > (uint8_t)thresh_light);
        Alarm_Tick(alarm_cond);

        delay_ms(10);
    }
}
