#include "lcd.h"
#include "delay.h"
#include "font.h"
#include "math.h"
#include "stm32f10x.h"
#include "string.h"

extern const unsigned char asc2_1608[];
extern const unsigned char chinese_font[];

// 根据你的屏幕 128x160 和 8x16 字体计算：
// 128 / 8 = 16 个字符每行
// 160 / 16 = 10 行
#define TERM_MAX_COLS 16  
#define TERM_MAX_ROWS 10
#define CHAR_WIDTH    8
#define CHAR_HEIGHT   16

static char term_buffer[TERM_MAX_ROWS][TERM_MAX_COLS + 1]; // 文本缓冲区
static uint8_t cur_row = 0; // 当前行
static uint8_t cur_col = 0; // 当前列

// 预设颜色
uint16_t term_fc = WHITE;
uint16_t term_bc = BLUE;

void LCD_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_APB2PeriphClockCmd(LCD_RCC_GPA | LCD_RCC_GPB | RCC_APB2Periph_AFIO, ENABLE);

    /* PA4(CS), PA1(BLK): 推挽输出 */
    GPIO_InitStruct.GPIO_Pin   = LCD_CS_PIN | LCD_BLK_PIN;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LCD_GPA_PORT, &GPIO_InitStruct);

    /* PA5(SCK), PA7(MOSI): 复用推挽 */
    GPIO_InitStruct.GPIO_Pin   = LCD_SCK_PIN | LCD_SDA_PIN;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LCD_GPA_PORT, &GPIO_InitStruct);

    /* PB0(DC), PB1(RES): 推挽输出 */
    GPIO_InitStruct.GPIO_Pin   = LCD_DC_PIN | LCD_RES_PIN;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LCD_GPB_PORT, &GPIO_InitStruct);

    LCD_CS_H;
    LCD_BLK_H;
    LCD_RES_H;
}

void LCD_SPI_Init(void) {
    SPI_InitTypeDef SPI_InitStruct;

    /* SPI1 挂在 APB2 (72MHz), 分频4 -> 18MHz */
    RCC_APB2PeriphClockCmd(LCD_RCC_SPI, ENABLE);

    SPI_InitStruct.SPI_Direction          = SPI_Direction_1Line_Tx;
    SPI_InitStruct.SPI_Mode               = SPI_Mode_Master;
    SPI_InitStruct.SPI_DataSize           = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL               = SPI_CPOL_Low;
    SPI_InitStruct.SPI_CPHA               = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_NSS                = SPI_NSS_Soft;
    SPI_InitStruct.SPI_BaudRatePrescaler  = SPI_BaudRatePrescaler_4; /* 72/4=18MHz */
    SPI_InitStruct.SPI_FirstBit           = SPI_FirstBit_MSB;
    SPI_InitStruct.SPI_CRCPolynomial      = 7;

    SPI_Init(LCD_SPI, &SPI_InitStruct);
    SPI_Cmd(LCD_SPI, ENABLE);
}

/* ================================================================
 *  SPI 数据发送
 * ================================================================ */

/* 通过硬件SPI发送一个字节 */
void LCD_SPI_SendByte(uint8_t data) {
    while (SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_TXE) == RESET)
        ;
    SPI_I2S_SendData(LCD_SPI, data);
    while (SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) == SET)
        ;
}

/* 发送命令 (DC=0) */
void LCD_SendCmd(uint8_t cmd) {
    LCD_CS_L;
    LCD_DC_L;
    LCD_SPI_SendByte(cmd);
    LCD_CS_H;
}

/* 发送8位数据 (DC=1) */
void LCD_SendData(uint8_t data) {
    LCD_CS_L;
    LCD_DC_H;
    LCD_SPI_SendByte(data);
    LCD_CS_H;
}

/* 发送16位数据 (DC=1, 高8位在前) */
void LCD_Send16Bit(uint16_t data) {
    LCD_CS_L;
    LCD_DC_H;
    LCD_SPI_SendByte(data >> 8);
    LCD_SPI_SendByte(data & 0xFF);
    LCD_CS_H;
}

/* 发送: 命令 + 8位数据 */
void LCD_SendCmdData(uint8_t cmd, uint8_t data) {
    LCD_SendCmd(cmd);
    LCD_SendData(data);
}

/* ================================================================
 *  初始化 / 复位 / 背光
 * ================================================================ */

void LCD_Reset(void) {
    LCD_RES_L;
    delay_ms(100);
    LCD_RES_H;
    delay_ms(50);
}

void LCD_BackLight(uint8_t on) {
    if (on)
        LCD_BLK_H;
    else
        LCD_BLK_L;
}

/*
 * ST7735S 初始化序列 — 1.77寸 128x160
 */
void LCD_Init(void) {
    LCD_GPIO_Init();
    LCD_SPI_Init();
    LCD_Reset();

    /* Sleep Out */
    LCD_SendCmd(0x11);
    delay_ms(120);

    /* ---------- 基本配置 ---------- */
    LCD_SendCmd(0x36);  /* Memory Data Access Control */
    LCD_SendData(0x00); /* 先写默认值, 后面 SetRotation 会覆盖 */

    LCD_SendCmd(0x3A);  /* Interface Pixel Format */
    LCD_SendData(0x05); /* 16-bit RGB565 */

    /* 帧率 (Normal / Idle / Partial) */
    LCD_SendCmd(0xB1);
    LCD_SendData(0x05);
    LCD_SendData(0x3C);
    LCD_SendData(0x3C);

    LCD_SendCmd(0xB2);
    LCD_SendData(0x05);
    LCD_SendData(0x3C);
    LCD_SendData(0x3C);

    LCD_SendCmd(0xB3);
    LCD_SendData(0x05);
    LCD_SendData(0x3C);
    LCD_SendData(0x3C);
    LCD_SendData(0x05);
    LCD_SendData(0x3C);
    LCD_SendData(0x3C);

    LCD_SendCmd(0xB4); /* Display Inversion */
    LCD_SendData(0x03);

    /* ---------- 电源控制 ---------- */
    LCD_SendCmd(0xC0);
    LCD_SendData(0x2E);
    LCD_SendData(0x06);
    LCD_SendData(0x04);

    LCD_SendCmd(0xC1);
    LCD_SendData(0xC0);
    LCD_SendData(0xC2);

    LCD_SendCmd(0xC2);
    LCD_SendData(0x0D);
    LCD_SendData(0x0D);

    LCD_SendCmd(0xC3);
    LCD_SendData(0x8D);
    LCD_SendData(0xEE);

    LCD_SendCmd(0xC4);
    LCD_SendData(0x8D);
    LCD_SendData(0xEE);

    LCD_SendCmd(0xC5); /* VCOM */
    LCD_SendData(0x00);

    /* ---------- 显示方向 ---------- */
    LCD_SendCmd(0x36);
    LCD_SendData(0xC0); /* MY=1, MX=1 — 默认 rotation=0 */

    /* ---------- Gamma ---------- */
    LCD_SendCmd(0xE0);
    LCD_SendData(0x1B);
    LCD_SendData(0x21);
    LCD_SendData(0x10);
    LCD_SendData(0x15);
    LCD_SendData(0x2B);
    LCD_SendData(0x25);
    LCD_SendData(0x1F);
    LCD_SendData(0x23);
    LCD_SendData(0x22);
    LCD_SendData(0x22);
    LCD_SendData(0x2B);
    LCD_SendData(0x37);
    LCD_SendData(0x00);
    LCD_SendData(0x15);
    LCD_SendData(0x02);
    LCD_SendData(0x3F);

    LCD_SendCmd(0xE1);
    LCD_SendData(0x1A);
    LCD_SendData(0x20);
    LCD_SendData(0x0F);
    LCD_SendData(0x15);
    LCD_SendData(0x2A);
    LCD_SendData(0x25);
    LCD_SendData(0x1E);
    LCD_SendData(0x23);
    LCD_SendData(0x23);
    LCD_SendData(0x22);
    LCD_SendData(0x2B);
    LCD_SendData(0x37);
    LCD_SendData(0x00);
    LCD_SendData(0x15);
    LCD_SendData(0x02);
    LCD_SendData(0x3F);

    /* ---------- 显示开启 ---------- */
    LCD_SendCmd(0x2C);
    LCD_SendCmd(0x29); /* Display ON */

    LCD_Clear(BLACK);
}

/*
 * 设置显示区域 [x_start, y_start] ~ [x_end, y_end]
 */
void LCD_SetRegion(uint16_t x_start, uint16_t y_start, uint16_t x_end,
                   uint16_t y_end) {
    LCD_SendCmd(0x2A); /* Column Address Set */
    LCD_SendData(0x00);
    LCD_SendData(x_start);
    LCD_SendData(0x00);
    LCD_SendData(x_end);

    LCD_SendCmd(0x2B); /* Row Address Set */
    LCD_SendData(0x00);
    LCD_SendData(y_start);
    LCD_SendData(0x00);
    LCD_SendData(y_end);

    LCD_SendCmd(0x2C); /* Memory Write */
}

void LCD_SetCursor(uint16_t x, uint16_t y) { LCD_SetRegion(x, y, x, y); }

/* ================================================================
 *  清屏 / 区域填充
 * ================================================================ */

void LCD_Clear(uint16_t color) {
    uint32_t i;
    uint32_t total = (uint32_t)LCD_WIDTH * LCD_HEIGHT;

    LCD_SetRegion(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

    LCD_CS_L;
    LCD_DC_H;
    for (i = 0; i < total; i++) {
        LCD_SPI_SendByte(color >> 8);
        LCD_SPI_SendByte(color & 0xFF);
    }
    LCD_CS_H;
}

void LCD_Fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
              uint16_t color) {
    uint32_t total = (uint32_t)(x2 - x1 + 1) * (y2 - y1 + 1);

    LCD_SetRegion(x1, y1, x2, y2);

    LCD_CS_L;
    LCD_DC_H;
    while (total--) {
        LCD_SPI_SendByte(color >> 8);
        LCD_SPI_SendByte(color & 0xFF);
    }
    LCD_CS_H;
}

/* ================================================================
 *  基本绘制: 点 / 线 / 圆 / 矩形
 * ================================================================ */

void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color) {
    LCD_SetCursor(x, y);
    LCD_Send16Bit(color);
}

/* ================================================================
 *  显示: 图片 / 字符 / 字符串 / 数字 / 中文
 * ================================================================ */

void LCD_ShowImage(uint16_t x, uint16_t y, uint16_t length, uint16_t width,
                   const unsigned char *p) {
    uint32_t total = (uint32_t)length * width;
    uint8_t picH, picL;

    LCD_SetRegion(x, y, x + length - 1, y + width - 1);

    LCD_CS_L;
    LCD_DC_H;
    for (uint32_t i = 0; i < total; i++) {
        picL = p[2 * i];
        picH = p[2 * i + 1];
        LCD_SPI_SendByte(picH);
        LCD_SPI_SendByte(picL);
    }
    LCD_CS_H;

    LCD_SetRegion(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
}

void LCD_ShowChar(uint8_t x, uint8_t y, uint16_t fc, uint16_t bc, char c) {
    int k = (c - 32) * CHAR_HEIGHT;
    LCD_SetRegion(x, y, x + CHAR_WIDTH - 1, y + CHAR_HEIGHT - 1);
    LCD_CS_L;
    LCD_DC_H;
    for (int i = 0; i < CHAR_HEIGHT; i++) {
        for (int j = 0; j < CHAR_WIDTH; j++) {
            uint16_t color = (asc2_1608[k + i] & (0x80 >> j)) ? fc : bc;
            LCD_SPI_SendByte(color >> 8);
            LCD_SPI_SendByte(color & 0xFF);
        }
    }
    LCD_CS_H;
}

void LCD_ShowString(uint8_t x, uint8_t y, uint16_t fc, uint16_t bc, const char *c) {
    int t = strlen(c);
    for (int i = 0; i < t; i++) {
        if (x >= LCD_WIDTH) {
            x = 0;
            y += 16;
        }
        LCD_ShowChar(x, y, fc, bc, c[i]);
        x += 8;
    }
}

void LCD_ShowNumber(uint8_t x, uint8_t y, uint16_t fc, uint16_t bc,
                    long long num) {
    uint8_t k = 0;
    char s[20];
    long long t = num;

    if (num == 0) {
        s[0] = '0';
        s[1] = '\0';
        LCD_ShowString(x, y, fc, bc, s);
        return;
    }

    while (t) {
        t /= 10;
        k++;
    }

    if (num < 0) {
        s[0] = '-';
        s[k + 1] = '\0';
        num = -num;
    } else {
        s[k] = '\0';
        k -= 1;
    }

    while (num) {
        s[k--] = '0' + num % 10;
        num /= 10;
    }

    LCD_ShowString(x, y, fc, bc, s);
}

/* ############ LCD调试相关函数 ############ */
static void LCD_Scroll(void) {
    // 1. 将缓冲区每一行往上搬
    for (int i = 0; i < TERM_MAX_ROWS - 1; i++) {
        memcpy(term_buffer[i], term_buffer[i + 1], TERM_MAX_COLS + 1);
    }
    // 2. 最后一行清空
    memset(term_buffer[TERM_MAX_ROWS - 1], 0, TERM_MAX_COLS + 1);
    
    // 3. 全屏重绘（为了效率，也可以只重绘改变的部分，但全绘最稳）
    LCD_Clear(term_bc);
    for (int i = 0; i < TERM_MAX_ROWS; i++) {
        if (term_buffer[i][0] != '\0') {
            LCD_ShowString(0, i * CHAR_HEIGHT, term_fc, term_bc, term_buffer[i]);
        }
    }
}

void LCD_PutChar(char c) {
    if (c == '\n') { // 处理换行符
        cur_col = 0;
        cur_row++;
    } else if (c == '\r') {
        cur_col = 0;
    } else {
        // 存入缓冲区
        term_buffer[cur_row][cur_col] = c;
        // 实时显示这个字符（可选，为了速度也可以只在 Printf 结束时画）
        LCD_ShowChar(cur_col * CHAR_WIDTH, cur_row * CHAR_HEIGHT, term_fc, term_bc, c);
        
        cur_col++;
        // 如果一行写满了，自动换行
        if (cur_col >= TERM_MAX_COLS) {
            cur_col = 0;
            cur_row++;
        }
    }

    // 如果超出了最大行数，向上滚动
    if (cur_row >= TERM_MAX_ROWS) {
        LCD_Scroll();
        cur_row = TERM_MAX_ROWS - 1;
    }
}

void LCD_Printf(const char *format, ...) {
    char buf[128]; // 临时格式化缓冲区
    va_list arg;
    
    va_start(arg, format);
    vsnprintf(buf, sizeof(buf), format, arg);
    va_end(arg);

    // 逐个字符处理
    for (int i = 0; buf[i] != '\0'; i++) {
        LCD_PutChar(buf[i]);
    }
}
