#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "delay.h"
#include "sys.h"
#include "adc.h"
#include "ds18b20.h"
#include "oled.h"
#include "beep.h"
#include "fan.h"
#include "jw01.h"
#include "esp01s.h"

// ============================================================
// 报警阈值定义
// ============================================================

// MQ-2 烟雾（ADC 原始值，12 位 0~4095）
#define SMOKE_LEVEL1_THR    500u
#define SMOKE_LEVEL2_THR    800u

// MQ-7 CO（ppm，由 ADC 线性估算：0~4095 → 0~1000 ppm）
#define CO_LEVEL1_THR       50u
#define CO_LEVEL2_THR       200u

// JW01 VOC（ppb）
#define VOC_LEVEL1_THR      500u
#define VOC_LEVEL2_THR      1000u

// DS18B20 温度（℃）
#define TEMP_LEVEL1_THR     40.0f
#define TEMP_LEVEL2_THR     60.0f

// CO 变化率预警阈值：10 s 内上升超过 50 ppm
#define CO_RATE_WIN         10u     // 采样窗口（每秒 1 次，共 10 个点）
#define CO_RATE_THR         50u     // ppm

// ============================================================
// 滑动平均滤波器（窗口 = 8）
// ============================================================

#define FILTER_SIZE  8u

typedef struct {
    uint16_t buf[FILTER_SIZE];
    uint8_t  idx;
    uint32_t sum;
    uint8_t  count;
} SlidingAvg_t;

static uint16_t SlidingAvg_Update(SlidingAvg_t *f, uint16_t val) {
    f->sum -= f->buf[f->idx];
    f->buf[f->idx] = val;
    f->sum += val;
    f->idx = (f->idx + 1u) % FILTER_SIZE;
    if (f->count < FILTER_SIZE) f->count++;
    return (uint16_t)(f->sum / f->count);
}

// ============================================================
// CO 变化率检测（10 s 滑动窗口）
// ============================================================

typedef struct {
    uint16_t val[CO_RATE_WIN];
    uint8_t  idx;
    uint8_t  count;
} RateWin_t;

static void RateWin_Push(RateWin_t *w, uint16_t v) {
    w->val[w->idx] = v;
    w->idx = (w->idx + 1u) % CO_RATE_WIN;
    if (w->count < CO_RATE_WIN) w->count++;
}

// 返回窗口内最大值与最小值之差
static uint16_t RateWin_Range(const RateWin_t *w) {
    uint8_t  i;
    uint16_t mn, mx;
    if (w->count == 0u) return 0u;
    mn = mx = w->val[0];
    for (i = 1u; i < w->count; i++) {
        if (w->val[i] < mn) mn = w->val[i];
        if (w->val[i] > mx) mx = w->val[i];
    }
    return mx - mn;
}

// ============================================================
// 系统全局状态
// ============================================================

typedef struct {
    // 采集值（滤波后）
    uint16_t smoke;         // MQ-2 ADC 原始值（滤波）
    uint16_t co_ppm;        // CO 浓度 ppm（滤波）
    uint16_t voc_ppb;       // VOC 浓度 ppb（JW01）
    float    temp;          // 温度 ℃

    // 预警状态
    uint8_t  alarm;         // 0=正常  1=一级  2=二级
    uint8_t  rate_alert;    // 1=CO 变化率异常加速

    // 风扇状态
    FanSpeed_t fan_speed;   // FAN_OFF / FAN_LOW / FAN_HIGH
    uint8_t    fan_manual;  // 1=手机端手动控制

    // 通信状态
    uint8_t wifi_ok;
    uint8_t mqtt_ok;
} AppState_t;

static AppState_t       g_state;
static SlidingAvg_t     g_smoke_flt;
static SlidingAvg_t     g_co_flt;
static RateWin_t        g_co_rate_win;

// ============================================================
// CO ppm 换算（MQ-7，粗略线性估算，满量程 1000 ppm）
// ============================================================

static uint16_t ADC_To_CO_ppm(uint16_t adc) {
    // 传感器浓度越高，输出电压越高，ADC 值越大
    // 简单线性映射：0~4095 -> 0~1000 ppm
    return (uint16_t)((uint32_t)adc * 1000u / 4095u);
}

// ============================================================
// 传感器采集（每 100 ms）
// ============================================================

static void SensorProcess(void) {
    static uint32_t last_t = 0u;
    uint16_t raw_smoke, raw_co, co_ppm;

    if (g_tick_ms - last_t < 100u) return;
    last_t = g_tick_ms;

    // MQ-2 烟雾
    raw_smoke = ADC1_GetValue(ADC_CH_SMOKE);
    g_state.smoke = SlidingAvg_Update(&g_smoke_flt, raw_smoke);

    // MQ-7 CO
    raw_co = ADC1_GetValue(ADC_CH_CO);
    co_ppm = ADC_To_CO_ppm(raw_co);
    g_state.co_ppm = SlidingAvg_Update(&g_co_flt, co_ppm);

    // JW01 VOC（中断驱动，有新数据才更新）
    uint16_t voc_tmp;
    if (JW01_GetData(&voc_tmp)) {
        g_state.voc_ppb = voc_tmp;
    }
}

// ============================================================
// 温度采集（非阻塞，每 2 s 读一次）
// ============================================================

static void TempProcess(void) {
    static uint8_t  converting = 0u;
    static uint32_t conv_tick  = 0u;
    static uint32_t next_tick  = 0u;

    if (!converting && g_tick_ms >= next_tick) {
        DS18B20_StartConvert();
        converting = 1u;
        conv_tick  = g_tick_ms;
        return;
    }
    if (converting && (g_tick_ms - conv_tick) >= 750u) {
        float t = DS18B20_GetTemp();
        if (t > -100.0f) g_state.temp = t;
        converting = 0u;
        next_tick  = g_tick_ms + 1250u;   // 约 2 s 读一次
    }
}

// ============================================================
// CO 变化率检测（每 1 s 推入一个采样点）
// ============================================================

static void RateProcess(void) {
    static uint32_t last_t = 0u;

    if (g_tick_ms - last_t < 1000u) return;
    last_t = g_tick_ms;

    RateWin_Push(&g_co_rate_win, g_state.co_ppm);

    // 窗口内上升幅度超过阈值，即使未达到绝对阈值也预警
    if (RateWin_Range(&g_co_rate_win) >= CO_RATE_THR) {
        g_state.rate_alert = 1u;
    } else {
        g_state.rate_alert = 0u;
    }
}

// ============================================================
// 双级预警逻辑
// 二级 > 一级；任意一个传感器达到对应阈值即触发
// ============================================================

static void AlarmProcess(void) {
    uint8_t lv2 = 0u, lv1 = 0u;

    // 二级判断
    if (g_state.smoke  >= SMOKE_LEVEL2_THR) lv2 = 1u;
    if (g_state.co_ppm >= CO_LEVEL2_THR)    lv2 = 1u;
    if (g_state.voc_ppb >= VOC_LEVEL2_THR)  lv2 = 1u;
    if (g_state.temp >= TEMP_LEVEL2_THR)    lv2 = 1u;

    // 一级判断（未达二级）
    if (!lv2) {
        if (g_state.smoke  >= SMOKE_LEVEL1_THR) lv1 = 1u;
        if (g_state.co_ppm >= CO_LEVEL1_THR)    lv1 = 1u;
        if (g_state.voc_ppb >= VOC_LEVEL1_THR)  lv1 = 1u;
        if (g_state.temp >= TEMP_LEVEL1_THR)    lv1 = 1u;
        // 变化率异常加速也触发一级
        if (g_state.rate_alert)                 lv1 = 1u;
    }

    g_state.alarm = lv2 ? 2u : (lv1 ? 1u : 0u);
}

// ============================================================
// 风扇与蜂鸣器执行
// ============================================================

static void ActuatorProcess(void) {
    // 手动模式下不覆盖风扇状态（由 MQTT 远程指令控制）
    if (!g_state.fan_manual) {
        FanSpeed_t target;
        switch (g_state.alarm) {
            case 2u:  target = FAN_HIGH; break;
            case 1u:  target = FAN_LOW;  break;
            default:  target = FAN_OFF;  break;
        }
        if (target != g_state.fan_speed) {
            g_state.fan_speed = target;
            FAN_SetSpeed(target);
        }
    }

    // 蜂鸣器：二级报警 500ms 闪鸣；其余静音
    if (g_state.alarm == 2u) {
        // 每 500ms 翻转一次蜂鸣器
        static uint32_t beep_t = 0u;
        static uint8_t  beep_s = 0u;
        if (g_tick_ms - beep_t >= 500u) {
            beep_t = g_tick_ms;
            beep_s = !beep_s;
            if (beep_s) BEEP_ON(); else BEEP_OFF();
        }
    } else {
        BEEP_OFF();
    }
}

// ============================================================
// OLED 刷新（每 300 ms）
// 显示布局（128×64，每字符 8×16，行间隔 2 页）：
//   行 0: 报警等级 + 变化率标志
//   行 2: 烟雾 + CO 浓度
//   行 4: VOC + 温度
//   行 6: 风扇状态 + 网络状态
// ============================================================

static void OledUpdate(void) {
    static uint32_t last_t = 0u;
    char buf[17];

    if (g_tick_ms - last_t < 300u) return;
    last_t = g_tick_ms;

    // 行 0：报警等级
    if (g_state.alarm == 2u) {
        OLED_ShowString(0, 0, (uint8_t *)"[!!ALARM LV2!!] ");
    } else if (g_state.alarm == 1u) {
        if (g_state.rate_alert) {
            OLED_ShowString(0, 0, (uint8_t *)"[WARN LV1+RATE] ");
        } else {
            OLED_ShowString(0, 0, (uint8_t *)"[WARN LV1     ] ");
        }
    } else {
        if (g_state.rate_alert) {
            OLED_ShowString(0, 0, (uint8_t *)"[RATE ALERT   ] ");
        } else {
            OLED_ShowString(0, 0, (uint8_t *)"[NORMAL       ] ");
        }
    }

    // 行 2：烟雾 + CO
    snprintf(buf, sizeof(buf), "Smk:%-4u CO:%-4u", g_state.smoke, g_state.co_ppm);
    OLED_ShowString(0, 2, (uint8_t *)buf);

    // 行 4：VOC + 温度
    {
        int16_t ti = (int16_t)g_state.temp;
        int16_t td = (int16_t)((g_state.temp - (float)ti) * 10.0f);
        if (td < 0) td = -td;
        snprintf(buf, sizeof(buf), "VOC:%-5uT:%d.%dC", g_state.voc_ppb, ti, td);
    }
    OLED_ShowString(0, 4, (uint8_t *)buf);

    // 行 6：风扇档位 + 网络状态
    {
        const char *fan_str;
        switch (g_state.fan_speed) {
            case FAN_HIGH: fan_str = "Hi "; break;
            case FAN_LOW:  fan_str = "Lo "; break;
            default:       fan_str = "Off"; break;
        }
        snprintf(buf, sizeof(buf), "Fan:%s%s W:%c M:%c ",
                 fan_str,
                 g_state.fan_manual ? "(M)" : "   ",
                 g_state.wifi_ok ? 'Y' : 'N',
                 g_state.mqtt_ok ? 'Y' : 'N');
    }
    OLED_ShowString(0, 6, (uint8_t *)buf);
}

// ============================================================
// MQTT 数据上报（每 5 s）
// JSON 格式：{"smoke":N,"co":N,"voc":N,"temp":N.N,"alarm":N,"fan":N}
// ============================================================

static void MqttPublishProcess(void) {
    static uint32_t last_t = 0u;
    char payload[128];

    if (g_tick_ms - last_t < 5000u) return;
    last_t = g_tick_ms;

    if (!g_state.mqtt_ok) return;

    // 温度格式化（避免 %f 体积过大，手动分解整数和小数部分）
    int16_t ti = (int16_t)g_state.temp;
    int16_t td = (int16_t)((g_state.temp - (float)ti) * 10.0f);
    if (td < 0) td = -td;

    snprintf(payload, sizeof(payload),
             "{\\\"smoke\\\":%u,\\\"co\\\":%u,\\\"voc\\\":%u,"
             "\\\"temp\\\":%d.%d,\\\"alarm\\\":%u,\\\"fan\\\":%u}",
             g_state.smoke,
             g_state.co_ppm,
             g_state.voc_ppb,
             ti, td,
             (unsigned)g_state.alarm,
             (unsigned)g_state.fan_speed);

    ESP01S_MQTTPublish(payload);
}

// ============================================================
// 远程控制处理（来自微信小程序 → MQTT → ESP01S）
// 支持指令：{"fan":0/1/2}
// fan=0: 关闭  fan=1: 低速手动  fan=2: 高速手动
// 手动控制在二级报警解除后自动恢复自动模式
// ============================================================

static void RemoteControlProcess(void) {
    uint8_t fan_cmd = 0xFFu;

    if (!ESP01S_Process(&fan_cmd)) {
        return;
    }

    if (fan_cmd <= 2u) {
        g_state.fan_manual = 1u;
        g_state.fan_speed  = (FanSpeed_t)fan_cmd;
        FAN_SetSpeed(g_state.fan_speed);
    }
}

// ============================================================
// 网络重连机制（Wi-Fi 或 MQTT 断线时自动重连）
// ============================================================

static void NetProcess(void) {
    static uint32_t last_check = 0u;

    if (g_tick_ms - last_check < 30000u) return;  // 每 30 s 检查一次
    last_check = g_tick_ms;

    g_state.wifi_ok = ESP01S_IsWiFiConnected();
    g_state.mqtt_ok = ESP01S_IsMQTTConnected();

    if (!g_state.wifi_ok) {
        g_state.mqtt_ok = 0u;
        if (ESP01S_ConnectWiFi()) {
            g_state.wifi_ok = 1u;
        }
    }

    if (g_state.wifi_ok && !g_state.mqtt_ok) {
        if (ESP01S_MQTTConnect()) {
            g_state.mqtt_ok = 1u;
            ESP01S_MQTTSubscribe();
        }
    }
}

// ============================================================
// 手动模式自动解除
// 若系统恢复正常（alarm=0）且已手动控制风扇，60 s 后自动切回自动模式
// ============================================================

static void ManualAutoRelease(void) {
    static uint32_t manual_start = 0u;
    static uint8_t  counting     = 0u;

    if (!g_state.fan_manual) {
        counting = 0u;
        return;
    }

    if (!counting) {
        manual_start = g_tick_ms;
        counting = 1u;
    }

    if ((g_tick_ms - manual_start) >= 60000u) {
        g_state.fan_manual = 0u;
        counting = 0u;
    }
}

// ============================================================
// main
// ============================================================

int main(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    delay_init();
    App_TickInit();

    // 外设初始化
    ADC1_GPIO_Config();     // PA0 MQ-2, PA1 MQ-7
    ADC1_Config();
    DS18B20_Init();         // PB0 单总线温度
    JW01_Init();            // USART1 PA10 VOC
    FAN_Init();             // TIM3_CH1 PA6 PWM 风扇
    BEEP_GPIO_Config();     // PB8 蜂鸣器
    OLED_Init();            // PB6/PB7 I2C OLED

    // 初始化状态
    memset(&g_state, 0, sizeof(g_state));
    memset(&g_smoke_flt, 0, sizeof(g_smoke_flt));
    memset(&g_co_flt, 0, sizeof(g_co_flt));
    memset(&g_co_rate_win, 0, sizeof(g_co_rate_win));
    g_state.temp       = 0.0f;
    g_state.fan_speed  = FAN_OFF;
    g_state.fan_manual = 0u;

    // OLED 欢迎画面
    OLED_Clear();
    OLED_ShowString(8,  2, (uint8_t *)"Battery Guard");
    OLED_ShowString(16, 4, (uint8_t *)"Starting...");
    delay_ms(1000);
    OLED_Clear();

    // Wi-Fi & MQTT 初始化（等待网络连接）
    ESP01S_Init();           // USART2 PA2/PA3
    OLED_ShowString(0, 0, (uint8_t *)"Connecting WiFi ");
    if (ESP01S_ConnectWiFi()) {
        g_state.wifi_ok = 1u;
        OLED_ShowString(0, 2, (uint8_t *)"WiFi OK         ");
        if (ESP01S_MQTTConnect()) {
            g_state.mqtt_ok = 1u;
            ESP01S_MQTTSubscribe();
            OLED_ShowString(0, 4, (uint8_t *)"MQTT OK         ");
        } else {
            OLED_ShowString(0, 4, (uint8_t *)"MQTT Fail       ");
        }
    } else {
        OLED_ShowString(0, 2, (uint8_t *)"WiFi Fail       ");
    }
    delay_ms(1500);
    OLED_Clear();

    // 主循环
    while (1) {
        SensorProcess();          // ADC 采集 + JW01 读取（100ms 节拍）
        TempProcess();            // DS18B20 非阻塞读取
        RateProcess();            // CO 变化率检测（1s 节拍）
        AlarmProcess();           // 双级预警判断
        ActuatorProcess();        // 风扇 PWM + 蜂鸣器
        RemoteControlProcess();   // 处理 MQTT 下行控制指令
        ManualAutoRelease();      // 手动模式 60 s 后自动解除
        NetProcess();             // 网络重连（30s 节拍）
        MqttPublishProcess();     // MQTT 上报（5s 节拍）
        OledUpdate();             // OLED 刷新（300ms 节拍）
    }
}
