#include "segment.h"

/* 共阴数码管段码表 (0-9) */
/* 段码顺序: DP G F E D C B A (PB7-PB0) */
static const uint8_t seg_code[] = {
    0x3F,  // 0
    0x06,  // 1
    0x5B,  // 2
    0x4F,  // 3
    0x66,  // 4
    0x6D,  // 5
    0x7D,  // 6
    0x07,  // 7
    0x7F,  // 8
    0x6F,  // 9
};

/* 显示缓冲区 */
static uint8_t display_buf[2] = {0, 0};
static uint8_t scan_index = 0;

/**
 * @brief  设置段选 (PB0-PB7)
 */
static void Segment_SetCode(uint8_t code)
{
    uint16_t temp;
    temp = SEG_PORT->ODR;
    temp &= ~SEG_MASK;
    temp |= (code & SEG_MASK);
    SEG_PORT->ODR = temp;
}

/**
 * @brief  关闭所有位选
 */
static void Segment_AllOff(void)
{
    GPIO_ResetBits(COM_PORT, S1_PIN | S2_PIN | S3_PIN | S4_PIN);
}

/**
 * @brief  选择数码管
 */
static void Segment_Select(Digit_Select_t digit)
{
    Segment_AllOff();
    
    if (digit == DIGIT_1)
    {
        GPIO_SetBits(COM_PORT, S1_PIN | S2_PIN);
    }
    else
    {
        GPIO_SetBits(COM_PORT, S3_PIN | S4_PIN);
    }
}

/**
 * @brief  定时器初始化
 */
static void Segment_TIM_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* 使能时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    
    /* 定时器配置 */
    TIM_InitStructure.TIM_Period = (200 - 1);
    TIM_InitStructure.TIM_Prescaler = (1600 - 1); // // 16M / 1600
    TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_InitStructure);
    
    /* NVIC 配置 */
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    /* 使能更新中断 */
    TIM_ClearFlag(TIM4, TIM_FLAG_Update);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
    
    /* 启动定时器 */
    TIM_Cmd(TIM4, ENABLE);
}

/**
 * @brief  初始化数码管
 */
void Segment_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    /* PB0-PB7 段选 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 |
                                  GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* PB12-PB15 位选 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    Segment_AllOff();
    Segment_SetCode(0x00);
    display_buf[0] = 0;
    display_buf[1] = 0;
    scan_index = 0;
    
    /* 初始化定时器 */
    Segment_TIM_Init();
}

/**
 * @brief  在指定数码管显示数字
 */
void Segment_DisplayNum(Digit_Select_t digit, uint8_t num)
{
    if (num > 9) num = 0;
    display_buf[digit] = num;
}

/**
 * @brief  同时设置两个数码管显示值
 */
void Segment_DisplayAll(uint8_t num1, uint8_t num2)
{
    if (num1 > 9) num1 = 0;
    if (num2 > 9) num2 = 0;
    display_buf[0] = num1;
    display_buf[1] = num2;
}

/**
 * @brief  清除显示
 */
void Segment_Clear(void)
{
    Segment_AllOff();
    Segment_SetCode(0x00);
}

/**
 * @brief  动态扫描函数（在定时器中断中调用，建议 2-5ms）
 */
void Segment_Scan(void)
{
    Segment_AllOff();
    Segment_SetCode(seg_code[display_buf[scan_index]]);
    Segment_Select((Digit_Select_t)scan_index);
    
    scan_index++;
    if (scan_index >= 2) scan_index = 0;
}

/**
 * @brief  TIM4 中断服务函数
 */
void TIM4_IRQHandler(void) {
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
    {
        Segment_Scan();
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    }
}
