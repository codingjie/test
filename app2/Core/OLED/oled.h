#ifndef OLED_H__
#define OLED_H__
#include "i2c.h"
#include "oled_data.h"

#define OLED_ADDRESS 0x78
#define OLED_hi2c hi2c1

#define OLED_UNFILLED			0
#define OLED_FILLED			1

#define OLED_PAGE 8
#define OLED_COLUMN 128
extern uint8_t OLED_GRAM[OLED_PAGE][OLED_COLUMN];

/* 初始化与刷新 */
/* 初始化 OLED 显示器并清屏 */
void OLED_Init(void);
/* 将 GRAM 缓存推送到整屏显示 */
void OLED_Update(void);
/* 将 GRAM 指定区域推送到显示：X/Y 为左上角坐标，Width/Height 为区域尺寸 */
void OLED_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);
/* 清除整屏 GRAM 缓存 */
void OLED_Clear(void);
/* 清除指定矩形区域的 GRAM */
void OLED_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);
/* 反色整屏（取反 GRAM） */
void OLED_Reverse(void);
/* 反色指定区域 */
void OLED_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);

/* 文本/图像显示 */
/* 在屏幕指定位置显示位图，Width/Height 为图像尺寸，Image 指向位图数据（按页排列） */
void OLED_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image);
/* 在指定位置显示单字符（FontSize 使用宏 OLED_8X16/OLED_6X8） */
void OLED_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize);
/* 按字符串显示文本（不跨行） */
void OLED_ShowString(int16_t X, int16_t Y, char *String, uint8_t FontSize);
/* 显示无符号整数（固定长度，不自动省略前导0） */
void OLED_ShowNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
/* 显示带符号整数，最前面显示符号位 */
void OLED_ShowSignedNum(int16_t X, int16_t Y, int32_t Number, uint8_t Length, uint8_t FontSize);
/* 以十六进制显示数值（大写字母） */
void OLED_ShowHexNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
/* 以二进制显示数值 */
void OLED_ShowBinNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
/* 显示浮点数：IntLength 整数部分宽度，FraLength 小数部分宽度 */
void OLED_ShowFloatNum(int16_t X, int16_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize);
/* 按字节数组显示中文（使用 oled_data 中的中文字库） */
void OLED_ShowChinese(int16_t X, int16_t Y, char *Chinese);
/* 类 printf 的显示函数，支持格式化输出 */
void OLED_Printf(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...);

/* 基元绘图函数 */
/* 在坐标 (X,Y) 处点亮像素（坐标原点为屏幕左上角） */
void OLED_DrawPoint(int16_t X, int16_t Y);
/* 画直线，从 (X0,Y0) 到 (X1,Y1) */
void OLED_DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1);
/* 画矩形，IsFilled 为 0 表示空心，非 0 表示实心 */
void OLED_DrawRectangle(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled);
/* 画三角形，IsFilled 控制是否填充 */
void OLED_DrawTriangle(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint8_t IsFilled);
/* 画圆，IsFilled 控制是否填充 */
void OLED_DrawCircle(int16_t X, int16_t Y, uint8_t Radius, uint8_t IsFilled);
/* 画椭圆，A 为水平半轴，B 为垂直半轴，IsFilled 控制是否填充 */
void OLED_DrawEllipse(int16_t X, int16_t Y, uint8_t A, uint8_t B, uint8_t IsFilled);
/* 画圆弧，从 StartAngle 到 EndAngle（度） */
void OLED_DrawArc(int16_t X, int16_t Y, uint8_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled);


#endif

