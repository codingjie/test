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

/* ------------------------------------------------------------------ */
/* Configuration                                                        */
/* ------------------------------------------------------------------ */
#define BIN_COUNT        4       /* number of trash bins                */
#define LID_OPEN_MS   3000       /* auto-close delay after opening (ms) */
#define ULTRA_CHECK_MS 500       /* overflow check interval (ms)        */
#define OLED_UPDATE_MS 300       /* OLED refresh interval (ms)          */
#define BEEP_FULL_MS   200       /* buzzer pulse duration when full (ms) */

/* ------------------------------------------------------------------ */
/* Bin state                                                            */
/* ------------------------------------------------------------------ */
typedef struct {
    uint8_t  open;        /* 0 = closed, 1 = open                    */
    uint32_t open_tick;   /* g_tick_ms when lid was last opened       */
    uint8_t  full;        /* 0 = not full, 1 = full (overflow)        */
} BinState_t;

static BinState_t g_bin[BIN_COUNT];

/* Short display names used on the OLED (6 chars + null) */
static const char * const bin_name[BIN_COUNT] = {
    "Recycl",   /* Bin1 - recyclable  */
    "Hazard",   /* Bin2 - hazardous   */
    "Kitch ",   /* Bin3 - kitchen     */
    "Other "    /* Bin4 - other waste */
};

/* ------------------------------------------------------------------ */
/* Lid control helpers                                                  */
/* ------------------------------------------------------------------ */
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

/* ------------------------------------------------------------------ */
/* IR proximity detection -> auto open                                  */
/* ------------------------------------------------------------------ */
static void IRProcess(void)
{
    uint8_t i;
    for (i = 1; i <= BIN_COUNT; i++) {
        if (IR_Detected(i)) {
            if (!g_bin[i - 1].open) {
                OpenBin(i);
            } else {
                /* person still present: refresh the auto-close timer  */
                g_bin[i - 1].open_tick = g_tick_ms;
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* Auto-close lids after LID_OPEN_MS                                   */
/* ------------------------------------------------------------------ */
static void AutoCloseProcess(void)
{
    uint8_t i;
    for (i = 1; i <= BIN_COUNT; i++) {
        if (g_bin[i - 1].open) {
            if ((g_tick_ms - g_bin[i - 1].open_tick) >= LID_OPEN_MS) {
                CloseBin(i);
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* Voice command handling                                               */
/* ------------------------------------------------------------------ */
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
        CloseBin(1); CloseBin(2); CloseBin(3); CloseBin(4);
        break;
    default: break;
    }
}

/* ------------------------------------------------------------------ */
/* Button handling: short press on KEY1~KEY4 toggles that bin lid       */
/* ------------------------------------------------------------------ */
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

/* ------------------------------------------------------------------ */
/* Overflow detection via ultrasonic sensors                            */
/* Lights LEDs, sounds buzzer, and sends WiFi notification when full.  */
/* ------------------------------------------------------------------ */
static void OverflowProcess(void)
{
    static uint32_t last_check  = 0;
    static uint8_t  beep_active = 0;
    static uint32_t beep_tick   = 0;
    uint8_t i, any_full = 0;
    char    msg[20];

    /* Deactivate buzzer after BEEP_FULL_MS */
    if (beep_active && (g_tick_ms - beep_tick) >= BEEP_FULL_MS) {
        BEEP_OFF();
        beep_active = 0;
    }

    if ((g_tick_ms - last_check) < ULTRA_CHECK_MS) return;
    last_check = g_tick_ms;

    for (i = 1; i <= BIN_COUNT; i++) {
        uint8_t was_full = g_bin[i - 1].full;
        g_bin[i - 1].full = ULTRA_IsFull(i);
        LED_SetBinStatus(i, g_bin[i - 1].full);

        /* Send WiFi notification the moment a bin becomes full */
        if (!was_full && g_bin[i - 1].full) {
            snprintf(msg, sizeof(msg), "BIN%d FULL\r\n", i);
            ESP01S_SendString(msg);
        }
        if (g_bin[i - 1].full) any_full = 1;
    }

    /* Buzzer beep when any bin is full */
    if (any_full && !beep_active) {
        BEEP_ON();
        beep_active = 1;
        beep_tick   = g_tick_ms;
    }
}

/* ------------------------------------------------------------------ */
/* OLED display update                                                  */
/* Layout (128x64, 8 rows of 8px each):
 *   Row 0: "Smart Trash Bin "
 *   Row 2: "Recycl:OPEN FUL"  or  "Recycl:CLSD    "
 *   Row 3: "Hazard:OPEN FUL"
 *   Row 4: "Kitch :CLSD    "
 *   Row 5: "Other :CLSD    "
 * ------------------------------------------------------------------ */
static void OledUpdate(void)
{
    static uint32_t last_t = 0;
    char buf[17];
    uint8_t i;

    if ((g_tick_ms - last_t) < OLED_UPDATE_MS) return;
    last_t = g_tick_ms;

    OLED_ShowString(0, 0, (uint8_t *)"Smart Trash Bin ");

    for (i = 0; i < BIN_COUNT; i++) {
        snprintf(buf, sizeof(buf), "%s:%s %s",
                 bin_name[i],
                 g_bin[i].open ? "OPEN" : "CLSD",
                 g_bin[i].full ? "FUL" : "   ");
        OLED_ShowString(0, (uint8_t)((i + 1) * 2), (uint8_t *)buf);
    }
}

/* ------------------------------------------------------------------ */
/* Main entry point                                                     */
/* ------------------------------------------------------------------ */
int main(void)
{
    uint8_t i;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    delay_init();
    App_TickInit();

    /* Peripheral initialisation */
    KEY_Init();             /* PC13/PC14/PC15/PA15 buttons (also disables JTAG) */
    LED_Init();             /* 8 status LEDs                                    */
    BEEP_GPIO_Config();     /* PB12 buzzer                                      */
    OLED_Init();            /* PB10/PB11 software I2C OLED                      */
    SG90_Init();            /* TIM3 CH1-4: PA6/PA7/PB0/PB1 servos              */
    IR_Init();              /* PA4/PA5/PA11/PA12 IR proximity sensors           */
    ULTRA_Init();           /* PA8 TRIG + PB6-9 ECHO ultrasonic sensors        */
    ASRPRO_Init();          /* USART2 PA2/PA3 voice recognition module          */
    ESP01S_Init();          /* USART1 PA9/PA10 WiFi module                      */

    /* Initial state: all bins closed, not full */
    for (i = 0; i < BIN_COUNT; i++) {
        g_bin[i].open      = 0;
        g_bin[i].open_tick = 0;
        g_bin[i].full      = 0;
        LED_SetBinStatus(i + 1, 0);   /* green LED on for each bin */
    }

    /* Splash screen */
    OLED_Clear();
    OLED_ShowString(4,  2, (uint8_t *)"Smart Trash Bin");
    OLED_ShowString(20, 4, (uint8_t *)"Initializing..");
    delay_ms(1500);
    OLED_Clear();

    while (1) {
        IRProcess();
        VoiceProcess();
        KeyProcess();
        AutoCloseProcess();
        OverflowProcess();
        OledUpdate();
    }
}
