#include "stm32f10x.h"
#include <stdio.h>
#include "delay.h"
#include "sys.h"
#include "key.h"
#include "oled.h"
#include "sg90.h"
#include "infrared.h"
#include "ultra.h"
#include "beep.h"
#include "rgb.h"
#include "asrpro.h"
#include "esp01s.h"

// 配置参数
#define BIN_COUNT        4       // 垃圾桶数量
#define LID_OPEN_MS   3000       // 开盖后自动关闭延迟（毫秒）
#define ULTRA_CHECK_MS 500       // 满溢检测间隔（毫秒）
#define OLED_UPDATE_MS 300       // OLED刷新间隔（毫秒）
#define BEEP_FULL_MS   200       // 满仓时蜂鸣器鸣响时长（毫秒）

// 垃圾桶状态
typedef struct {
    uint8_t  open;        // 0=关闭, 1=开启
    uint32_t open_tick;   // 最后一次开盖时的 g_tick_ms
    uint8_t  full;        // 0=未满, 1=已满（满溢）
    uint8_t  dist_mm;     // 超声波检测距离（mm），上限99
} BinState_t;

static BinState_t g_bin[BIN_COUNT];

// OLED 显示名称（每个6字符+结束符）
static const char * const bin_name[BIN_COUNT] = {
    "Recycl",   // 1号桶 - 可回收
    "Hazard",   // 2号桶 - 有害垃圾
    "Kitch ",   // 3号桶 - 厨余垃圾
    "Other "    // 4号桶 - 其他垃圾
};

// 开盖/关盖函数
static void OpenBin(uint8_t bin)
{
    if (bin < 1 || bin > BIN_COUNT) return;
    g_bin[bin - 1].open      = 1;
    g_bin[bin - 1].open_tick = g_tick_ms;
    SG90_Open(bin);
}

static void CloseBin(uint8_t bin)
{
    if (bin < 1 || bin > BIN_COUNT) return;
    g_bin[bin - 1].open = 0;
    SG90_Close(bin);
}

// 红外感应检测 -> 自动开盖
static void IRProcess(void)
{
    uint8_t i;
    for (i = 1; i <= BIN_COUNT; i++) {
        if (IR_Detected(i)) {
            if (!g_bin[i - 1].open) {
                OpenBin(i);
            } else {
                // 人还在：刷新自动关盖计时
                g_bin[i - 1].open_tick = g_tick_ms;
            }
        }
    }
}

// 超过 LID_OPEN_MS 后自动关盖
static void AutoCloseProcess(void)
{
    uint8_t i;
    for (i = 1; i <= BIN_COUNT; i++) {
        if (g_bin[i - 1].open) {
            if ((g_tick_ms - g_bin[i - 1].open_tick) >= LID_OPEN_MS) {
                CloseBin(i);
                return;   // 每次主循环只关一个桶，错开电流峰值
            }
        }
    }
}

// 语音命令处理
static void VoiceProcess(void)
{
    uint8_t cmd = asrpro_rx_cmd;
    if (cmd == 0) return;
    asrpro_rx_cmd = 0;

    switch (cmd) {
    case ASRPRO_CMD_BIN_RECYCLABLE: OpenBin(1);  break;
    case ASRPRO_CMD_BIN_HAZARDOUS:  OpenBin(2);  break;
    case ASRPRO_CMD_BIN_KITCHEN:    OpenBin(3);  break;
    case ASRPRO_CMD_BIN_OTHER:      OpenBin(4);  break;
    case ASRPRO_CMD_OPEN_ALL:
        OpenBin(1); OpenBin(2); OpenBin(3); OpenBin(4);
        break;
    case ASRPRO_CMD_CLOSE_ALL:
        CloseBin(1); delay_ms(200);
        CloseBin(2); delay_ms(200);
        CloseBin(3); delay_ms(200);
        CloseBin(4);
        break;
    default: break;
    }
}

// 按键处理：短按 KEY1~KEY4 切换对应垃圾桶桶盖
static void KeyProcess(void)
{
    uint8_t key = KEY_Scan();
    if (key >= 1 && key <= BIN_COUNT) {
        if (g_bin[key - 1].open)
            CloseBin(key);
        else
            OpenBin(key);
    }
}

// 超声波满溢检测
// 满仓时点亮LED并触发蜂鸣器报警
// （WiFi通知由机智云框架负责，此处不再调用ESP01S）
static void OverflowProcess(void)
{
    static uint32_t last_check  = 0;
    static uint8_t  beep_active = 0;
    static uint32_t beep_tick   = 0;
    uint8_t i, any_full = 0;

    // 蜂鸣器鸣响时长到期后关闭
    if (beep_active && (g_tick_ms - beep_tick) >= BEEP_FULL_MS) {
        BEEP_OFF();
        beep_active = 0;
    }

    if ((g_tick_ms - last_check) < ULTRA_CHECK_MS) return;
    last_check = g_tick_ms;

    for (i = 1; i <= BIN_COUNT; i++) {
        uint16_t d = ULTRA_GetDistance_mm(i);
        g_bin[i - 1].dist_mm = (d == 0xFFFF || d > 99) ? 99 : (uint8_t)d;
        g_bin[i - 1].full    = (d != 0xFFFF && d < ULTRA_FULL_MM) ? 1 : 0;
        LED_SetBinStatus(i, g_bin[i - 1].full);
        if (g_bin[i - 1].full) any_full = 1;
    }

    // 有任意垃圾桶满仓时触发蜂鸣器
    if (any_full && !beep_active) {
        BEEP_ON();
        beep_active = 1;
        beep_tick   = g_tick_ms;
    }
}

// OLED 显示刷新
// 布局（128x64，16px字体，每行占2 page，共4行）：
//   page 0-1: "Recycl:OPEN 25mm"
//   page 2-3: "Hazard:CLSD  8mm"
//   page 4-5: "Kitch :FULL  3mm"
//   page 6-7: "Other :CLSD 50mm"
static void OledUpdate(void)
{
    static uint32_t last_t = 0;
    char buf[17];
    const char *state;
    uint8_t i;

    if ((g_tick_ms - last_t) < OLED_UPDATE_MS) return;
    last_t = g_tick_ms;

    for (i = 0; i < BIN_COUNT; i++) {
        if (g_bin[i].full)       state = "FULL";
        else if (g_bin[i].open)  state = "OPEN";
        else                     state = "CLSD";

        snprintf(buf, sizeof(buf), "%s:%-4s %2dmm",
                 bin_name[i], state, (int)g_bin[i].dist_mm);
        OLED_ShowString(0, (uint8_t)(i * 2), (uint8_t *)buf);
    }
}

int main(void) {
    uint8_t i;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    delay_init();
    App_TickInit();

    // 外设初始化
    KEY_Init();             // PC13/PC14/PC15/PA15 按键（同时禁用JTAG）
    LED_Init();             // 8路状态LED
    BEEP_GPIO_Config();     // PB12 蜂鸣器
    OLED_Init();            // PB10/PB11 软件I2C OLED
    SG90_Init();            // TIM3 CH1-4：PA6/PA7/PB0/PB1 舵机
    IR_Init();              // PA4/PA5/PA11/PA12 红外接近传感器
    ULTRA_Init();           // PA8 TRIG + PB6-9 ECHO 超声波传感器
    ASRPRO_Init();          // USART2 PA2/PA3 语音识别模块
    // ESP01S 由机智云框架管理，此处不初始化

    // 初始状态：所有垃圾桶关闭且未满（SG90_Init 已输出关盖脉宽，此处只同步状态）
    for (i = 0; i < BIN_COUNT; i++) {
        g_bin[i].open      = 0;
        g_bin[i].open_tick = 0;
        g_bin[i].full      = 0;
        g_bin[i].dist_mm   = 99;
        LED_SetBinStatus(i + 1, 0);
    }

    // 开机欢迎界面，同时等待红外传感器稳定
    OLED_Clear();
    OLED_ShowString(4,  2, (uint8_t *)"Smart Trash Bin");
    OLED_ShowString(4, 4, (uint8_t *)"Initializing...");
    delay_ms(1500);
    OLED_Clear();

    while (1) {
        VoiceProcess();
        KeyProcess();
        AutoCloseProcess();
        OverflowProcess();
        OledUpdate();
        IRProcess();
    }
}
