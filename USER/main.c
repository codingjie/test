/**
 ******************************************************************************
 * @file    main.c
 * @brief   PWM 频率 & 占空比测量仪
 *
 * 功能：
 *  - TIM2 CH1/CH2 (PA0) 测量 1Hz-10kHz 方波的频率和占空比
 *  - ST7735S LCD (128×160) 实时显示测量结果
 *  - AT24C02 EEPROM 循环存储最近 10 组数据
 *  - KEY1(PB10)：进入历史查询 / 翻页（向旧）
 *  - KEY2(PB11)：手动保存当前数据 / 退出历史查询
 *
 * LCD 布局 (8×16 字体, 16列×10行)：
 *  y=  0  "=  PWM  METER  ="  蓝底白字标题
 *  y= 16  空行
 *  y= 32  "FREQ: XXXXX Hz  "  频率
 *  y= 48  "DUTY:   XXX %   "  占空比
 *  y= 64  空行
 *  y= 72  分隔线 (2px)
 *  y= 80  "[LIVE]          " 或 "[HIST  N/10]    "
 *  y= 96  空行
 *  y=112  "K1:HIST  K2:SAVE" 或 "K1:PREV  K2:EXIT"
 *  y=128  空行
 *  y=144  临时状态消息
 ******************************************************************************
 */

#include "stm32f10x.h"
#include "delay.h"
#include "lcd.h"
#include "tim.h"
#include "key.h"
#include "at24c02.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* ================================================================
 *  格式化打印到 LCD 固定行（16 字符宽，不足补空格）
 * ================================================================ */
static void lcd_putline(uint8_t y, uint16_t fc, uint16_t bc,
                        const char *fmt, ...) {
    char buf[17];
    int  len;
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    len = strlen(buf);
    while (len < 16) buf[len++] = ' ';
    buf[16] = '\0';
    LCD_ShowString(0, y, fc, bc, buf);
}

/* ================================================================
 *  界面绘制
 * ================================================================ */

/* 静态框架：清屏 + 标题 + 分隔线（模式切换时调用）*/
static void draw_frame(void) {
    LCD_Clear(BLACK);
    LCD_Fill(0, 0, 127, 15, BLUE);                  /* 标题背景 */
    lcd_putline(0,  WHITE, BLUE,  "=  PWM  METER  =");
    LCD_Fill(0, 72, 127, 73, GRAY1);                /* 水平分隔线 */
}

/* 模式指示行 + 按键提示行 */
static void draw_mode_hints(uint8_t mode, uint8_t view_idx) {
    if (mode == 0) {    /* LIVE */
        lcd_putline(80,  GREEN, BLACK, "[LIVE]");
        lcd_putline(112, WHITE, BLACK, "K1:HIST  K2:SAVE");
    } else {            /* HIST */
        uint8_t cnt = EEPROM_GetCount();
        lcd_putline(80,  YELLOW, BLACK, "[HIST %2u/%-2u]",
                    (unsigned)(view_idx + 1), (unsigned)cnt);
        lcd_putline(112, WHITE,  BLACK, "K1:PREV  K2:EXIT");
    }
}

/* 频率 + 占空比行 */
static void draw_data(uint32_t freq, uint8_t duty, uint8_t present) {
    if (!present) {
        lcd_putline(32, RED, BLACK, "FREQ: NO SIGNAL ");
        lcd_putline(48, RED, BLACK, "DUTY:    ---    ");
    } else {
        lcd_putline(32, YELLOW, BLACK, "FREQ: %5lu Hz  ",
                    (unsigned long)freq);
        lcd_putline(48, CYAN,   BLACK, "DUTY:  %3u %%    ",
                    (unsigned)duty);
    }
}

/* 临时状态消息（显示后延迟清除）*/
static void flash_msg(const char *msg, uint16_t fc) {
    lcd_putline(144, fc, BLACK, msg);
    delay_ms(800);
    lcd_putline(144, BLACK, BLACK, "");
}

/* ================================================================
 *  按键扫描（边沿检测 + 消抖）
 *  返回：0=无事件  1=KEY1按下  2=KEY2按下
 * ================================================================ */
static uint8_t key_scan(void) {
    static uint8_t prev1 = 0, prev2 = 0;
    uint8_t k1 = KEY1_PRESSED;
    uint8_t k2 = KEY2_PRESSED;
    uint8_t ev = 0;

    if (k1 && !prev1) {
        delay_ms(20);                       /* 消抖 */
        if (KEY1_PRESSED) ev = 1;
    } else if (k2 && !prev2) {
        delay_ms(20);
        if (KEY2_PRESSED) ev = 2;
    }
    prev1 = k1;
    prev2 = k2;
    return ev;
}

/* ================================================================
 *  主程序
 * ================================================================ */
int main(void) {
    /* ---- 初始化 ---- */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    delay_init();
    LCD_Init();
    KEY_Init();
    EEPROM_Init();
    PWM_Capture_Init();

    /* ---- 初始界面 ---- */
    draw_frame();
    draw_mode_hints(0, 0);
    lcd_putline(32, WHITE, BLACK, "  Waiting...    ");
    lcd_putline(48, WHITE, BLACK, "                ");

    /* ---- 应用状态 ---- */
    uint8_t  mode         = 0;        /* 0=LIVE  1=HIST */
    uint8_t  view_idx     = 0;        /* 历史查看索引(0=最旧) */

    uint32_t cur_freq     = 0;
    uint8_t  cur_duty     = 0;
    uint8_t  sig_ok       = 0;

    /* 防抖：仅当值发生变化时刷新 FREQ/DUTY 行，避免屏幕闪烁 */
    uint32_t disp_freq    = 0xFFFFFFFFu;
    uint8_t  disp_duty    = 0xFF;
    uint8_t  disp_sig     = 0xFF;

    /* 历史模式：仅 KEY2 退出，无自动超时 */

    /* ---- 主循环 ---- */
    for (;;) {
        uint8_t key = key_scan();

        /* 更新测量数据 */
        if (PWM_Capture_Ready()) {
            cur_freq = PWM_Get_Freq();
            cur_duty = PWM_Get_Duty();
            sig_ok   = 1;
        }
        if (!PWM_Signal_Present()) {
            sig_ok   = 0;
            cur_freq = 0;
            cur_duty = 0;
        }

        /* ============================================================
         *  LIVE 模式
         * ============================================================ */
        if (mode == 0) {
            /* 仅数据变化时刷新显示 */
            if (cur_freq != disp_freq ||
                cur_duty  != disp_duty  ||
                sig_ok    != disp_sig) {
                draw_data(cur_freq, cur_duty, sig_ok);
                disp_freq = cur_freq;
                disp_duty = cur_duty;
                disp_sig  = sig_ok;
            }

            /* KEY1：进入历史查询 */
            if (key == 1) {
                uint8_t cnt = EEPROM_GetCount();
                if (cnt > 0) {
                    mode        = 1;
                    view_idx    = cnt - 1;  /* 从最新记录开始 */
                    draw_frame();
                    draw_mode_hints(1, view_idx);
                    PwmRecord rec;
                    EEPROM_ReadRecord(view_idx, &rec);
                    draw_data(rec.freq, rec.duty, 1);
                } else {
                    flash_msg("No records yet  ", RED);
                }
            }
            /* KEY2：手动保存 */
            else if (key == 2) {
                if (sig_ok) {
                    EEPROM_SaveRecord(cur_freq, cur_duty);
                    flash_msg("   Saved!       ", GREEN);
                } else {
                    flash_msg("No signal!      ", RED);
                }
            }

        /* ============================================================
         *  HIST 模式  (只有 KEY2 可退出)
         * ============================================================ */
        } else {
            /* KEY1：查看上一条（更旧）记录 */
            if (key == 1) {
                uint8_t cnt = EEPROM_GetCount();
                view_idx = (view_idx == 0) ? cnt - 1 : view_idx - 1;
                draw_mode_hints(1, view_idx);
                PwmRecord rec;
                EEPROM_ReadRecord(view_idx, &rec);
                draw_data(rec.freq, rec.duty, 1);
            }
            /* KEY2：退出历史查询 */
            else if (key == 2) {
                mode = 0;
                disp_freq = 0xFFFFFFFFu;
                disp_sig  = 0xFF;
                draw_frame();
                draw_mode_hints(0, 0);
            }
        }

        delay_ms(50);
    }
}
