#include "lcd.h"
#include "delay.h"
#include "includes.h"

// 外部字库数据声明
extern unsigned char data_0[];
extern unsigned char data_1[];
extern unsigned char data_2[];
extern unsigned char data_3[];
extern unsigned char data_4[];
extern unsigned char data_5[];
extern unsigned char data_6[];
extern unsigned char data_7[];
extern unsigned char data_8[];
extern unsigned char data_9[];
extern unsigned char data_dot[];

extern unsigned char data_zhuan[];
extern unsigned char data_su[];
extern unsigned char data_mao[];
extern unsigned char data_R[];
extern unsigned char data_zhan[];
extern unsigned char data_kong[];
extern unsigned char data_bi[];
extern unsigned char data_zheng[];
extern unsigned char data_fu[];
extern unsigned char data_ting[];
extern unsigned char data_clear[128];

// 外部变量声明
extern float MOTOR_SPEED; // 角速度
extern uint8_t currentDuty;

// 设置GPIO引脚电平
static void setGPIOPin(uint16_t pin, BitAction state) {
    GPIO_WriteBit(GPIOC, pin, state);
}

// 批量设置数据引脚
static void setDataPins(BitAction state) {
    GPIO_WriteBit(GPIOC,
                  D0_Pin | D1_Pin | D2_Pin | D3_Pin | D4_Pin | D5_Pin | D6_Pin |
                      D7_Pin,
                  state);
}

// 数据转换函数
void convertDataToBits(uint16_t data, BitAction *transform, uint16_t length) {
    uint16_t i = 0;
    for (i = 0; i < length; i++) {
        switch ((data >> i) & 0x01) {
        case 1:
            *(transform + i) = Bit_SET;
            break;
        case 0:
            *(transform + i) = Bit_RESET;
            break;
        default:;
        }
    }
}

// 写数据到LCD
void writeDisplayData(uint16_t ucData) {
    uint16_t i = 0;
    BitAction dataBits[8];

    ucData = ~ucData;

    // 转换数据为位数组
    convertDataToBits(ucData, dataBits, 8);

    setDataPins(Bit_SET);
    setGPIOPin(RW_Pin, Bit_RESET);
    setGPIOPin(RS_Pin, Bit_SET);

    // 逐位写入数据
    for (i = 0; i < 8; i++) {
        switch (i) {
        case 0:
            setGPIOPin(D0_Pin, *(dataBits + i));
            break;
        case 1:
            setGPIOPin(D1_Pin, *(dataBits + i));
            break;
        case 2:
            setGPIOPin(D2_Pin, *(dataBits + i));
            break;
        case 3:
            setGPIOPin(D3_Pin, *(dataBits + i));
            break;
        case 4:
            setGPIOPin(D4_Pin, *(dataBits + i));
            break;
        case 5:
            setGPIOPin(D5_Pin, *(dataBits + i));
            break;
        case 6:
            setGPIOPin(D6_Pin, *(dataBits + i));
            break;
        case 7:
            setGPIOPin(D7_Pin, *(dataBits + i));
            break;
        default:;
        }
    }

    setGPIOPin(EN_Pin, Bit_SET);
    setGPIOPin(EN_Pin, Bit_RESET);
}

// 写命令到LCD
void writeDisplayCmd(uint16_t ucCMD) {
    uint16_t i = 0;
    BitAction cmdBits[8];

    // 转换命令为位数组
    convertDataToBits(ucCMD, cmdBits, 8);

    setDataPins(Bit_SET);
    setGPIOPin(RW_Pin, Bit_RESET);
    setGPIOPin(RS_Pin, Bit_RESET);

    // 逐位写入命令
    for (i = 0; i < 8; i++) {
        switch (i) {
        case 0:
            setGPIOPin(D0_Pin, *(cmdBits + i));
            break;
        case 1:
            setGPIOPin(D1_Pin, *(cmdBits + i));
            break;
        case 2:
            setGPIOPin(D2_Pin, *(cmdBits + i));
            break;
        case 3:
            setGPIOPin(D3_Pin, *(cmdBits + i));
            break;
        case 4:
            setGPIOPin(D4_Pin, *(cmdBits + i));
            break;
        case 5:
            setGPIOPin(D5_Pin, *(cmdBits + i));
            break;
        case 6:
            setGPIOPin(D6_Pin, *(cmdBits + i));
            break;
        case 7:
            setGPIOPin(D7_Pin, *(cmdBits + i));
            break;
        default:;
        }
    }

    setGPIOPin(EN_Pin, Bit_SET);
    setGPIOPin(EN_Pin, Bit_RESET);
}

// LCD硬件初始化
void initDisplayHardware(void) {
    setGPIOPin(RST_Pin, Bit_SET);
    writeDisplayCmd(0x38);        // 8位接口模式
    writeDisplayCmd(0x0F);        // 显示开启
    writeDisplayCmd(0x01);        // 清屏
    writeDisplayCmd(0x06);        // 自动递增
    writeDisplayCmd(LCDSTARTROW); // 起始行
}

// 显示一行自定义内容
void displayCustomLine(uint16_t ucPage, uint16_t ucLine, uint16_t ucWidth,
                       unsigned char *ucaRow) {
    uint16_t ucCount;

    if (ucLine < 64) {
        // 左半屏
        setGPIOPin(CS1_Pin, Bit_SET);
        setGPIOPin(CS2_Pin, Bit_RESET);
        writeDisplayCmd(LCDPAGE + ucPage);
        writeDisplayCmd(LCDLINE + ucLine);

        if ((ucLine + ucWidth) < 64) {
            for (ucCount = 0; ucCount < ucWidth; ucCount++)
                writeDisplayData(*(ucaRow + ucCount));
        } else {
            // 跨屏显示
            for (ucCount = 0; ucCount < 64 - ucLine; ucCount++)
                writeDisplayData(*(ucaRow + ucCount));

            setGPIOPin(CS1_Pin, Bit_RESET);
            setGPIOPin(CS2_Pin, Bit_SET);
            writeDisplayCmd(LCDPAGE + ucPage);
            writeDisplayCmd(LCDLINE);

            for (ucCount = 64 - ucLine; ucCount < ucWidth; ucCount++)
                writeDisplayData(*(ucaRow + ucCount));
        }
    } else {
        // 右半屏
        setGPIOPin(CS1_Pin, Bit_RESET);
        setGPIOPin(CS2_Pin, Bit_SET);
        writeDisplayCmd(LCDPAGE + ucPage);
        writeDisplayCmd(LCDLINE + ucLine - 64);

        for (ucCount = 0; ucCount < ucWidth; ucCount++)
            writeDisplayData(*(ucaRow + ucCount));
    }
}

void clearDisplayAll(void) {
    uint16_t page, col;

    for (page = 0; page < 8; page++) {
        // 清除左半屏 (0-63列)
        setGPIOPin(CS1_Pin, Bit_SET);
        setGPIOPin(CS2_Pin, Bit_RESET);
        writeDisplayCmd(LCDPAGE + page);
        writeDisplayCmd(LCDLINE);
        for (col = 0; col < 64; col++)
            writeDisplayData(0xff);

        // 清除右半屏 (64-127列)
        setGPIOPin(CS1_Pin, Bit_RESET);
        setGPIOPin(CS2_Pin, Bit_SET);
        writeDisplayCmd(LCDPAGE + page);
        writeDisplayCmd(LCDLINE);
        for (col = 0; col < 64; col++)
            writeDisplayData(0xff);
    }
}

// 打印16点高度内容
void render16PointText(uint16_t startpage, uint16_t startline, uint16_t width, unsigned char *data) {
    displayCustomLine(startpage, startline, width, data);
    displayCustomLine(startpage + 1, startline, width, data + width);
}

// 初始化LCD显示
void setupLCDDisplay(void) {
    GPIO_InitTypeDef lcdPins;

    // 使能GPIOC时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    // 配置LCD所有控制和数据引脚
    lcdPins.GPIO_Pin = D7_Pin | RST_Pin | CS1_Pin | CS2_Pin | EN_Pin | RW_Pin |
                       RS_Pin | D0_Pin | D1_Pin | D2_Pin | D3_Pin | D4_Pin |
                       D5_Pin | D6_Pin;
    lcdPins.GPIO_Mode = GPIO_Mode_Out_PP;
    lcdPins.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &lcdPins);

    // 初始化所有引脚为低电平
    GPIO_WriteBit(GPIOC,
                  D7_Pin | RST_Pin | CS1_Pin | CS2_Pin | EN_Pin | RW_Pin |
                      RS_Pin | D0_Pin | D1_Pin | D2_Pin | D3_Pin | D4_Pin |
                      D5_Pin | D6_Pin,
                  Bit_RESET);

    // LCD硬件初始化
    initDisplayHardware();

    // 清屏操作
    // clearDisplayAll();
    // 显示转速
    render16PointText(0, 66, 16, data_zhuan);
    render16PointText(0, 84, 16, data_su);
    render16PointText(0, 100, 9, data_mao);
    render16PointText(0, 45, 9, data_R);
    // 显示占空比
    render16PointText(3, 66, 16, data_zhan);
    render16PointText(3, 84, 16, data_kong);
    render16PointText(3, 102, 16, data_bi);
    render16PointText(3, 118, 9, data_mao);
}

// 渲染数字显示
static void renderDigit(uint16_t page, uint16_t pos, uint16_t digit) {
    switch (digit) {
    case 0:
        render16PointText(page, pos, 9, data_0);
        break;
    case 1:
        render16PointText(page, pos, 9, data_1);
        break;
    case 2:
        render16PointText(page, pos, 9, data_2);
        break;
    case 3:
        render16PointText(page, pos, 9, data_3);
        break;
    case 4:
        render16PointText(page, pos, 9, data_4);
        break;
    case 5:
        render16PointText(page, pos, 9, data_5);
        break;
    case 6:
        render16PointText(page, pos, 9, data_6);
        break;
    case 7:
        render16PointText(page, pos, 9, data_7);
        break;
    case 8:
        render16PointText(page, pos, 9, data_8);
        break;
    case 9:
        render16PointText(page, pos, 9, data_9);
        break;
    default:;
    }
}

// LCD显示更新任务
void displayUpdateTask(void *pdata) {
    uint16_t speedInt = 0;
    
    while (1) {
        // 显示角速度
        speedInt = (uint16_t)(MOTOR_SPEED * 10);
        if (speedInt > 9999)
            speedInt = 9999;

        // 显示角速度: XXX.X
        renderDigit(0, 0,  (speedInt / 1000) % 10); // 百位
        renderDigit(0, 9,  (speedInt / 100)  % 10); // 十位
        renderDigit(0, 18, (speedInt / 10)   % 10); // 个位
        render16PointText(0, 27, 9, data_dot);      // 小数点
        renderDigit(0, 36, speedInt % 10);          // 小数位

        // 显示占空比
        renderDigit(3, 18,  currentDuty / 100 % 10); // 百位
        renderDigit(3, 27,  currentDuty / 10  % 10); // 十位
        renderDigit(3, 36, currentDuty % 10); // 个位
        OSTimeDlyHMSM(0, 0, 0, 500);
    }
}
