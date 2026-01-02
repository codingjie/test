#include "bsp_timer_encoder.h"

uint8_t dirction_flag = 0;          //方向标志位

static void Encoder_GPIO_Config(void) {
    GPIO_InitTypeDef gpio_initstruct;
    
    /* 使能GPIO时钟 */
    ENCODER_RCC_CLK_ENABLE(ENCODER_RCC_CLK, ENABLE);
    ENCODER_KEY_RCC_CLK_ENABLE(ENCODER_KEY_RCC_CLK, ENABLE);

    /* 配置编码器A、B相GPIO复用功能 */
    GPIO_PinAFConfig(ENCODER_A_GPIO_PORT, ENCODER_A_GPIO_PINSOURCE, ENCODER_A_GPIO_AF);
    GPIO_PinAFConfig(ENCODER_B_GPIO_PORT, ENCODER_B_GPIO_PINSOURCE, ENCODER_B_GPIO_AF);

    /* 初始化编码器A相GPIO */
    gpio_initstruct.GPIO_Mode = GPIO_Mode_AF;       // 复用模式
    gpio_initstruct.GPIO_PuPd = GPIO_PuPd_UP;       // 上拉
    gpio_initstruct.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_initstruct.GPIO_OType = GPIO_OType_PP;
    gpio_initstruct.GPIO_Pin = ENCODER_A_GPIO_PIN;
    GPIO_Init(ENCODER_A_GPIO_PORT, &gpio_initstruct);
    
    /* 初始化编码器B相GPIO */
    gpio_initstruct.GPIO_Pin = ENCODER_B_GPIO_PIN;
    GPIO_Init(ENCODER_B_GPIO_PORT, &gpio_initstruct);
    
    /* 初始化编码器按键GPIO */
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 1. 开启 SYSCFG 时钟 (F4系列配置中断映射必须开启) */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* 2. 连接 EXTI 线到 PA8 */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource8);

    /* 3. 配置 EXTI8 中断线 */
    EXTI_InitStructure.EXTI_Line = EXTI_Line8;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    /* 设为双边沿触发，以模拟你原有的按下/松开检测 */
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; 
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* 4. 配置 NVIC 中断优先级 */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn; // EXTI8 属于 9_5 组
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  TIM编码器接口模式配置
  * @param  无
  * @retval 无
  */
static void TIMX_Encoder_Config(void)
{
    TIM_TimeBaseInitTypeDef tim_base_initstruct;
    TIM_ICInitTypeDef tim_ic_initstruct;

    /* 使能TIM_ENCODER时钟 */
    TIM_ENCODER_RCC_CLK_ENABLE(TIM_ENCODER_RCC_CLK, ENABLE);

    /* =============================配置TIM_ENCODER时基单元============================ */
    /* 初始化TIM_ENCODER */
    tim_base_initstruct.TIM_ClockDivision = TIM_CKD_DIV1;
    tim_base_initstruct.TIM_CounterMode = TIM_CounterMode_Up;
    tim_base_initstruct.TIM_Period = 65536 - 1; /* ARR */
    tim_base_initstruct.TIM_Prescaler = 1 - 1;  /* PSC */
    tim_base_initstruct.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM_ENCODER, &tim_base_initstruct);

    /* =============================配置TIM_ENCODER输入捕获单元========================== */
    TIM_ICStructInit(&tim_ic_initstruct);   /* 结构体赋初值 */
    tim_ic_initstruct.TIM_Channel = ENCODER_A_CHANNEL | ENCODER_B_CHANNEL;
    tim_ic_initstruct.TIM_ICFilter = 0;

    TIM_ICInit(TIM_ENCODER, &tim_ic_initstruct);
    
    /* 配置TIM编码器接口 */
    TIM_EncoderInterfaceConfig(TIM_ENCODER, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);

    /* 使能TIMX */
    TIM_Cmd(TIM_ENCODER, ENABLE);
}

/**
  * @brief  获取编码器转值
  * @param  无
  * @retval 无
  */
void Encoder_Get_Val(float *cycle_count)
{
    int16_t temp;
    /* 获取当前计数值 */
    temp = TIM_ENCODER->CNT;

    /* 圈数累加 */
    *cycle_count += (float)temp / 80;

    if(temp == 0)   //静止
    {
        dirction_flag = STILLNESS;
    }
    else if(temp > 0)  //正方向转
    {
        dirction_flag = POSITIVE_DIRECTION;
    }
    else  //反方向转 (temp < 0)
    {
        dirction_flag = REVERSE_DIRECTION;
    }
    
    /* 清零CNT */
    TIM_ENCODER->CNT = 0;
}

/**
  * @brief  编码器GPIO初始化，TIM使用编码器接口模式
  * @param  无
  * @retval 无
  */
void TIMX_Encoder_Init(void)
{
    Encoder_GPIO_Config();
    TIMX_Encoder_Config();
}
