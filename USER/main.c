#include "stm32f10x.h"
#include "delay.h"
#include "lcd.h"

int main(void) {
    delay_init();
    LCD_Init();

    // 1~4 静态显示测试
    LCD_Clear(BLACK);
    LCD_ShowString(0,  0,  WHITE,  BLACK, "Hello STM32!");
    LCD_ShowString(0,  16, YELLOW, BLACK, "LCD Test OK");
    LCD_ShowString(0,  32, GREEN,  BLACK, "SPI1 18MHz");
    LCD_ShowString(0,  48, CYAN,   BLACK, "Num:");
    LCD_ShowNumber(40, 48, CYAN,   BLACK, 12345);
    LCD_Fill(0,  70, 40,  100, RED);
    LCD_Fill(44, 70, 84,  100, GREEN);
    LCD_Fill(88, 70, 127, 100, BLUE);
    delay_ms(2000);  // 看2秒

    // 5~6 清屏后再测 Printf/滚动
    LCD_Clear(BLACK);
    LCD_Printf("Printf Test\n");
    LCD_Printf("Val=%d\n", 42);
    for (int i = 0; i < 15; i++) {
        LCD_Printf("Line %d\n", i);
        delay_ms(200);
    }

    while (1) {
        delay_ms(500);
    }
}
