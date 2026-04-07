#include "stm32f10x.h"
#include <stdio.h>
#include "delay.h"
#include "oled.h"
#include "ds18b20.h"

// 工作模式
typedef enum {
    MODE_MANUAL = 0,    /* 手动: 按键控制转速/摇头 */
    MODE_AUTO,          /* 自动: 温度自动调速，可手动/语音开摇头 */
    MODE_VOICE,         /* 语音: ASRPRO 控制所有功能 */
    MODE_TRACK          /* 追踪: 雷达+PIR 追踪人体，无人5s后关机 */
} WorkMode_t;

// 系统全局状态
typedef struct {
    WorkMode_t mode;
    uint8_t    power;   /* 0=关机  1=开机 */
    uint8_t    speed;   /* 0~3档  (0=停) */
    uint8_t    swing;   /* 0=固定  1=摇头 */
    float      temp;    /* 当前温度 ℃ */
    uint8_t    pir;     /* SR501 检测状态: 0/1 */
    uint8_t    radar;   /* LD2450 检测状态: 0/1 */
} AppState_t;

static AppState_t g_state;
static uint32_t g_last_person_tick = 0;

static void SetPower(uint8_t on) {
    g_state.power = on ? 1 : 0;
    if (on) {
        if (g_state.speed == 0) { 
            g_state.speed = 1; 
        }
        MOTOR_SetSpeed(g_state.speed);
    } else {
        g_state.speed = 0;
        g_state.swing = 0;  // 关闭风扇时停止摇头
        MOTOR_SetSpeed(0);
        SG90_SetAngle(SERVO_CENTER); // 复位舵机到中心
    }
}

static void SetSpeed(uint8_t speed) {
    if (speed > 3) speed = 3;
    g_state.speed = speed;
    if (speed == 0) {
        g_state.power = 0;
        g_state.swing = 0;  // 关闭风扇时停止摇头
    }
    else {
        g_state.power = 1;
    }
    MOTOR_SetSpeed(speed);
}

static void SetMode(WorkMode_t new_mode) {
    if (new_mode == MODE_MANUAL || new_mode == MODE_VOICE) {
        SetSpeed(0);
        SetPower(0);
    }
    g_state.mode = new_mode;
}

static void KeyProcess(void) {
    uint8_t key = KEY_Scan();
    switch (key) {
        // KEY1 短按: 模式循环
        case 1:
            SetMode((WorkMode_t)((g_state.mode + 1) % 4));
            break;

        // KEY2 短按: 加档
        case 2:
            if (g_state.speed < 3 && (g_state.mode == MODE_MANUAL || g_state.mode == MODE_VOICE)) {
                SetSpeed(g_state.speed + 1);
            }
            break;

        // KEY3 短按: 减档
        case 3:
            if (g_state.speed > 0 && (g_state.mode == MODE_MANUAL || g_state.mode == MODE_VOICE)) {
                SetSpeed(g_state.speed - 1);
            }
            break;

        // KEY4 短按: 摇头开关（追踪模式下和电机关闭时禁用）
        case 4:
            if (g_state.mode != MODE_TRACK && g_state.power)
                g_state.swing = !g_state.swing;
            break;

        default: break;
    }
}

// 语音命令处理
static void VoiceProcess(void) {
    uint8_t cmd = asrpro_rx_cmd;
    if (cmd == 0)
        return;
    asrpro_rx_cmd = 0;

    switch (cmd) {
    case ASRPRO_CMD_FAN_ON:
    case ASRPRO_CMD_FAN_ON2:
        if (g_state.mode == MODE_MANUAL || g_state.mode == MODE_VOICE)
            SetPower(1);
        break;
    case ASRPRO_CMD_FAN_OFF:
        if (g_state.mode == MODE_MANUAL || g_state.mode == MODE_VOICE)
            SetPower(0);
        break;
    case ASRPRO_CMD_SPEED_UP:
        if (g_state.speed < 3 && (g_state.mode == MODE_MANUAL || g_state.mode == MODE_VOICE))
            SetSpeed(g_state.speed + 1);
        break;
    case ASRPRO_CMD_SPEED_DOWN:
        if (g_state.speed > 0 && (g_state.mode == MODE_MANUAL || g_state.mode == MODE_VOICE))
            SetSpeed(g_state.speed - 1);
        break;
    case ASRPRO_CMD_SPEED_1:
        if (g_state.mode == MODE_MANUAL || g_state.mode == MODE_VOICE)
            SetSpeed(1);
        break;
    case ASRPRO_CMD_SPEED_2:
        if (g_state.mode == MODE_MANUAL || g_state.mode == MODE_VOICE)
            SetSpeed(2);
        break;
    case ASRPRO_CMD_SPEED_3:
        if (g_state.mode == MODE_MANUAL || g_state.mode == MODE_VOICE)
            SetSpeed(3);
        break;
    case ASRPRO_CMD_SWING_ON:
        if (g_state.mode != MODE_TRACK && g_state.power)
            g_state.swing = 1;
        break;
    case ASRPRO_CMD_SWING_OFF:
        if (g_state.mode != MODE_TRACK && g_state.power)
            g_state.swing = 0;
        break;
    case ASRPRO_CMD_MODE_MANUAL:
        SetMode(MODE_MANUAL);;
        break;
    case ASRPRO_CMD_MODE_AUTO:
        SetMode(MODE_AUTO);
        break;
    case ASRPRO_CMD_MODE_VOICE:
        SetMode(MODE_VOICE);
        break;
    case ASRPRO_CMD_MODE_TRACK:
        SetMode(MODE_TRACK);
        break;
    default:
        break;
    }
}

// 自动模式（温控调速）
static void AutoLogic(void) {
    uint8_t target;
    if (g_state.temp < 26.0f)
        target = 0;
    else if (g_state.temp < 29.0f)
        target = 1;
    else if (g_state.temp < 32.0f)
        target = 2;
    else
        target = 3;

    if (target != g_state.speed)
        SetSpeed(target);
}

// 追踪模式
static void TrackLogic(void) {
    uint8_t person = 0;

    // 只有PIR检测到人才判定有人
    if (g_state.pir) {
        person = 1;
        g_last_person_tick = g_tick_ms;
    }

    // PIR有人时雷达才追踪方向
    if (g_state.pir && g_state.radar) {
        if (!g_state.swing) {
            // 找X最接近中心的目标
            int32_t best_x = 0x7FFFFFFF;
            uint8_t i;
            for (i = 0; i < ld2450_frame.count; i++) {
                int32_t x = ld2450_frame.target[i].x;
                if (x < -500 || x > 500) continue;
                if (best_x == 0x7FFFFFFF ||
                    (x < 0 ? -x : x) < (best_x < 0 ? -best_x : best_x)) {
                    best_x = x;
                }
            }

            if (best_x != 0x7FFFFFFF) {
                int16_t deg = SERVO_CENTER - (int16_t)(best_x * 60 / 500);
                if (deg < SERVO_MIN) deg = SERVO_MIN;
                if (deg > SERVO_MAX) deg = SERVO_MAX;
                SG90_SetAngle((uint8_t)deg);
            }
        }
    }

    // 无人 5s → 关机
    if (!person && (g_tick_ms - g_last_person_tick) >= 5000) {
        if (g_state.power) SetPower(0);
    } else if (person && !g_state.power) {
        // 人重新出现 → 开启
        g_state.speed = 2;
        SetPower(1);
    }

    // PIR有人时按Y轴距离自动调速（200近~1000远，越远越大）
    if (person && g_state.radar) {
        int32_t y = ld2450_frame.target[0].y;
        uint8_t target;
        if (y < 400)       target = 1;  // 近 → 小风
        else if (y < 700)  target = 2;  // 中 → 中风
        else               target = 3;  // 远 → 大风
        if (target != g_state.speed)
            SetSpeed(target);
    }
}

// 摇头（自动往复 40°~160°）
static void SwingProcess(void) {
    static uint8_t angle = SERVO_CENTER;
    static int8_t dir = 1;
    static uint32_t last_t = 0;

    if (!g_state.swing)
        return;

    if (g_tick_ms - last_t >= 50) {
        last_t = g_tick_ms;
        if (dir > 0) {
            angle++;
            if (angle >= SERVO_MAX) {
                angle = SERVO_MAX;
                dir = -1;
            }
        } else {
            angle--;
            if (angle <= SERVO_MIN) {
                angle = SERVO_MIN;
                dir = 1;
            }
        }
        SG90_SetAngle(angle);
    }
}

// OLED 刷新
static void OledUpdate(void) {
    static uint32_t last_t = 0;
    char buf[17];

    if (g_tick_ms - last_t < 300) return;
    last_t = g_tick_ms;

    // 行0: 模式
    static const char * const mode_name[] = {"Manual", "Auto  ", "Voice ", "Track "};
    snprintf(buf, sizeof(buf), "Mode:%-6s", mode_name[g_state.mode]);
    OLED_ShowString(0, 0, (uint8_t *)buf);

    // 行2: 风扇状态
    if (g_state.power) {
        snprintf(buf, sizeof(buf), "Fan:ON   Spd:%d   ", g_state.speed);
    } else {
        snprintf(buf, sizeof(buf), "Fan:OFF  Spd:%d   ", g_state.speed);
    }
    OLED_ShowString(0, 2, (uint8_t *)buf);

    // 行4: 温度 + 摇头
    if (g_state.temp > -100.0f) {
        int16_t ti = (int16_t)g_state.temp;
        int16_t td = (int16_t)((g_state.temp - ti) * 10);
        if (td < 0) td = -td;
        snprintf(buf, sizeof(buf), "T:%d.%dC  Swg:%s", ti, td,
                 g_state.swing ? "ON " : "OFF");
    } else {
        snprintf(buf, sizeof(buf), "T:---    Swg:%s  ",
                 g_state.swing ? "ON " : "OFF");
    }
    OLED_ShowString(0, 4, (uint8_t *)buf);

    // 行6: 雷达坐标 + PIR
    if (g_state.radar) {
        int32_t rx = ld2450_frame.target[0].x;
        int32_t ry = ld2450_frame.target[0].y;
        snprintf(buf, sizeof(buf), "(%ld,%ld)        ", rx, ry);
    } else {
        snprintf(buf, sizeof(buf), "(---,---)       ");
    }
    OLED_ShowString(0, 6, (uint8_t *)buf);

    // PIR + 倒计时固定显示在右侧
    if (g_state.mode == MODE_TRACK) {
        uint32_t elapsed = g_tick_ms - g_last_person_tick;
        uint8_t cnt;
        if (g_state.pir) {
            cnt = 5;
        } else if (elapsed >= 5000) {
            cnt = 0;
        } else {
            cnt = (uint8_t)(5 - elapsed / 1000);
        }
        snprintf(buf, sizeof(buf), "P:%c %d", g_state.pir ? 'Y' : 'N', cnt);
    } else {
        snprintf(buf, sizeof(buf), "P:%c  ", g_state.pir ? 'Y' : 'N');
    }
    OLED_ShowString(88, 6, (uint8_t *)buf);
}

// 温度采集（每2s读一次，避免阻塞主循环）
static void TempProcess(void) {
    static uint8_t  converting  = 0;
    static uint32_t conv_tick   = 0;
    static uint32_t next_tick   = 0;

    if (!converting && g_tick_ms >= next_tick) {
        DS18B20_StartConvert();
        converting = 1;
        conv_tick  = g_tick_ms;
        return;
    }
    if (converting && (g_tick_ms - conv_tick) >= 750) {
        float t = DS18B20_GetTemp();
        if (t > -100.0f) g_state.temp = t;
        converting = 0;
        next_tick  = g_tick_ms + 250;   // 约1s读一次
    }
}

int main(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    delay_init();
    App_TickInit();

    KEY_Init();         // PA5/PA8/PA15/PB0 按键（含JTAG禁用释放PA15）
    OLED_Init();        // PB6/PB7 I2C OLED
    DS18B20_Init();     // PA1 温度传感器
    SR501_Init();       // PA4 PIR 传感器
    ASRPRO_Init();      // USART2 PA2/PA3 语音模块
    LD2450_Init();      // USART3 PB10/PB11 毫米波雷达
    SG90_Init();        // TIM3_CH1 PA6 舵机
    MOTOR_PWM_Init();   // TIM3_CH2 PA7 风扇（共用TIM3时基）

    g_state.mode  = MODE_MANUAL;
    g_state.power = 0;
    g_state.speed = 0;
    g_state.swing = 0;
    g_state.temp  = -999.0f;
    g_state.pir   = 0;
    g_state.radar = 0;

    OLED_Clear();
    OLED_ShowString(20, 2, (uint8_t *)"Smart Fan");
    OLED_ShowString(20, 4, (uint8_t *)"Starting...");
    delay_ms(1500);
    OLED_Clear();

    while (1) {
        KeyProcess();
        VoiceProcess();
        TempProcess();

        g_state.pir   = SR501_Detected();
        g_state.radar = (ld2450_frame.count > 0) ? 1 : 0;

        switch (g_state.mode) {
            case MODE_AUTO:  AutoLogic();  break;
            case MODE_TRACK: TrackLogic(); break;
            default: break;
        }

        SwingProcess();
        OledUpdate();
    }
}
