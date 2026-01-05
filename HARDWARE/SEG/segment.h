#ifndef __SEGMENT_H
#define __SEGMENT_H

#include "stm32f10x.h"
#include "stm32f10x_tim.h"

/* 段选端口 PB0-PB7 */
#define SEG_PORT    GPIOB
#define SEG_MASK    0x00FF  // PB0-PB7

/* 位选引脚 */
#define S1_PIN      GPIO_Pin_12  // 数码管1
#define S2_PIN      GPIO_Pin_13  // 数码管1
#define S3_PIN      GPIO_Pin_14  // 数码管2
#define S4_PIN      GPIO_Pin_15  // 数码管2
#define COM_PORT    GPIOB

/* 数码管选择 */
typedef enum {
    DIGIT_1 = 0,  // 数码管1
    DIGIT_2       // 数码管2
} Digit_Select_t;

/* 函数声明 */
void Segment_Init(void);
void Segment_DisplayNum(Digit_Select_t digit, uint8_t num);
void Segment_DisplayAll(uint8_t num1, uint8_t num2);
void Segment_Clear(void);
void Segment_Scan(void);  // 动态扫描，放在定时器中断中调用

#endif /* __SEGMENT_H */
