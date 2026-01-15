#include "motor.h"
#include "delay.h"
#include "includes.h"
#include "key.h"

// 电机速度全局变量
float MOTOR_SPEED = 0.0;
uint8_t currentDuty = 50;

static uint16_t savedDuty = 50;
static uint8_t motorRunning = 1;

// 事件标志组
extern OS_FLAG_GRP *EventFlags;

// 六步换向表
static const uint8_t commutationTable[8][3] = {
    {0, 0, 0},  // 0 - 无效
    {3, 3, 2},  // 1 - C+, B-  (CH3 PWM, CH2N 导通)
    {2, 2, 1},  // 2 - B+, A-  (CH2 PWM, CH1N 导通)
    {3, 3, 1},  // 3 - C+, A-  (CH3 PWM, CH1N 导通)
    {1, 1, 3},  // 4 - A+, C-  (CH1 PWM, CH3N 导通)
    {1, 1, 2},  // 5 - A+, B-  (CH1 PWM, CH2N 导通)
    {2, 2, 3},  // 6 - B+, C-  (CH2 PWM, CH3N 导通)
    {0, 0, 0},  // 7 - 无效
};

// 设置PWM占空比
static void setPWMDuty(uint8_t channel, uint16_t duty) {
    switch (channel) {
        case 1: TIM_SetCompare1(TIM1, duty); break;
        case 2: TIM_SetCompare2(TIM1, duty); break;
        case 3: TIM_SetCompare3(TIM1, duty); break;
    }
}

// 关闭所有输出
static void disableAllOutputs(void) {
    // 关闭所有通道的输出
    TIM1->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC1NE |
                    TIM_CCER_CC2E | TIM_CCER_CC2NE |
                    TIM_CCER_CC3E | TIM_CCER_CC3NE);
    
    // 占空比设为0
    TIM_SetCompare1(TIM1, 0);
    TIM_SetCompare2(TIM1, 0);
    TIM_SetCompare3(TIM1, 0);
}

// 执行换向
static void applyCommutation(uint8_t pwmChannel, uint8_t highChannel, uint8_t lowChannel, uint16_t duty) {
    // 先关闭所有输出
    TIM1->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC1NE |
                    TIM_CCER_CC2E | TIM_CCER_CC2NE |
                    TIM_CCER_CC3E | TIM_CCER_CC3NE);
    
    // 设置占空比为0
    TIM_SetCompare1(TIM1, 0);
    TIM_SetCompare2(TIM1, 0);
    TIM_SetCompare3(TIM1, 0);
    
    // 使能上桥臂 PWM 输出
    switch (highChannel) {
        case 1: TIM1->CCER |= TIM_CCER_CC1E; break;
        case 2: TIM1->CCER |= TIM_CCER_CC2E; break;
        case 3: TIM1->CCER |= TIM_CCER_CC3E; break;
    }
    
    // 使能下桥臂互补输出（设为常开）
    switch (lowChannel) {
        case 1: TIM1->CCER |= TIM_CCER_CC1NE; break;
        case 2: TIM1->CCER |= TIM_CCER_CC2NE; break;
        case 3: TIM1->CCER |= TIM_CCER_CC3NE; break;
    }
    
    // 设置 PWM 占空比
    setPWMDuty(pwmChannel, 100 - duty);
}

// 根据霍尔状态换向
static void updateCommutation(uint8_t hallState) {
    uint8_t pwmCh, highCh, lowCh;
    
    if (hallState == 0 || hallState == 7) {
        return;  // 无效状态
    }
    
    pwmCh  = commutationTable[hallState][0];
    highCh = commutationTable[hallState][1];
    lowCh  = commutationTable[hallState][2];
    
    if (pwmCh != 0) {
        applyCommutation(pwmCh, highCh, lowCh, currentDuty);
    }
}

// 读取霍尔传感器状态
static uint8_t readHallSensors(void) {
    uint8_t state = 0x00;
    state |= GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5);       // SC -> 位0
    state |= GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) << 1;  // SB -> 位1
    state |= GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7) << 2;  // SA -> 位2
    return state;
}

// 电机运行任务
void brushlessMotorTask(void *pdata) {
    uint8_t hallState;
    uint16_t adcValue;
    OS_FLAGS flags;
    INT8U err;
    (void)pdata;

    while (1) {
        flags = OSFlagPend(EventFlags, KEY1_FLAG | KEY2_FLAG | KEY3_FLAG, 
                          OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME, 1, &err);

        if (err == OS_ERR_NONE) {
            if (flags & KEY1_FLAG) {
                // 启停切换
                if (motorRunning) {
                    savedDuty = currentDuty;
                    currentDuty = 0; // 停止
                    motorRunning = 0;
                } else {
                    currentDuty = savedDuty;
                    motorRunning = 1;
                }
            }
            if (flags & KEY2_FLAG) {
                // 加速
                if (currentDuty <= 90) {
                    currentDuty += 10;
                } else {
                    currentDuty = 100;
                }
                savedDuty = currentDuty;
            }
            if (flags & KEY3_FLAG) {
                // 减速
                if (currentDuty >= 10) {
                    currentDuty -= 10;
                } else {
                    currentDuty = 0;
                }
                savedDuty = currentDuty;
            }
        }

        hallState = readHallSensors();
        updateCommutation(hallState);

        ADC_SoftwareStartConvCmd(ADC1, ENABLE);
        while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
        adcValue = ADC_GetConversionValue(ADC1);
        MOTOR_SPEED = 5.0f * (float)adcValue / 4096.0f * 60;

        OSTimeDlyHMSM(0, 0, 0, 10);
    }
}

// 初始化电机PWM（互补输出模式）
void setupBrushlessMotor(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_BDTRInitTypeDef TIM_BDTRInitStructure;

    // 使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOA | 
                           RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    // 配置上桥臂 PWM 引脚: PA8, PA9, PA10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置下桥臂互补输出引脚: PB13, PB14, PB15
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 配置霍尔传感器输入引脚: PA5, PA6, PA7
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置定时器时基
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Prescaler = 7200 - 1;    // 72MHz / 7200 = 10kHz
    TIM_TimeBaseStructure.TIM_Period = 100 - 1;        // 10kHz / 100 = 100Hz
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    // 配置PWM模式
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;  // 使能互补输出
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;      // 互补输出极性
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    TIM_OCInitStructure.TIM_Pulse = 0;  // 初始占空比为0

    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    TIM_OC2Init(TIM1, &TIM_OCInitStructure);
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);

    // 配置死区时间（防止上下桥臂同时导通）
    TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;
    TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Enable;
    TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;
    TIM_BDTRInitStructure.TIM_DeadTime = 72;           // 死区时间 = 72/72MHz = 1us
    TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;
    TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;
    TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;
    TIM_BDTRConfig(TIM1, &TIM_BDTRInitStructure);

    // 使能预装载
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM1, ENABLE);

    // 启动定时器
    TIM_Cmd(TIM1, ENABLE);

    // 使能PWM输出
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    
    // 初始状态关闭所有输出
    disableAllOutputs();
}

// 初始化ADC采集电路
void setupMotorADC(void) {
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能ADC和GPIOB时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_ADC1, ENABLE);

    // 配置ADC时钟分频
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    // 配置ADC输入引脚PB0
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 配置ADC参数
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    // 使能ADC
    ADC_Cmd(ADC1, ENABLE);

    // 配置ADC通道8（PB0）采样时间
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);
}
