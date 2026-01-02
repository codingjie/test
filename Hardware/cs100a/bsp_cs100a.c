#include "bsp_cs100a.h"
#include "FreeRTOS.h"
#include "task.h"

STRUCT_CAPTURE TIM_ICUserValueStructure = {0, 0, 0, 0};

/**
  * @brief  ECHO引脚GPIO配置（定时器输入捕获）
  */
static void CS100A_ECHO_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* 开启GPIO时钟 */
    RCC_AHB1PeriphClockCmd(ECHO_GPIO_CLK, ENABLE);
    
    /* 配置GPIO复用功能 */
    GPIO_PinAFConfig(ECHO_GPIO_PORT, ECHO_GPIO_PINSOURCE, ECHO_GPIO_AF);
    
    /* 配置GPIO */
    GPIO_InitStructure.GPIO_Pin = ECHO_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        // 复用模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;      // 下拉
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    
    GPIO_Init(ECHO_GPIO_PORT, &GPIO_InitStructure);
}

/**
  * @brief  TRIG引脚GPIO配置
  */
static void CS100A_TRIG_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* 开启GPIO时钟 */
    RCC_AHB1PeriphClockCmd(TRIG_GPIO_CLK, ENABLE);
    
    /* 配置GPIO */
    GPIO_InitStructure.GPIO_Pin = TRIG_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;       // 输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      // 推挽
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        // 上拉
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    
    GPIO_Init(TRIG_GPIO_PORT, &GPIO_InitStructure);
    
    /* 默认低电平 */
    GPIO_ResetBits(TRIG_GPIO_PORT, TRIG_GPIO_PIN);
}

/**
  * @brief  定时器输入捕获配置
  */
static void GENERAL_TIM_Config(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* 开启定时器时钟 */
    RCC_APB1PeriphClockCmd(GENERAL_TIM_CLK, ENABLE);
    
    /* 定时器基本配置 */
    TIM_TimeBaseStructure.TIM_Prescaler = GENERAL_TIM_PRESCALER;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = GENERAL_TIM_PERIOD;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    
    TIM_TimeBaseInit(GENERAL_TIMx, &TIM_TimeBaseStructure);
    
    /* 输入捕获配置 */
    TIM_ICInitStructure.TIM_Channel = GENERAL_TIM_CHANNEL;
    TIM_ICInitStructure.TIM_ICPolarity = GENERAL_TIM_START_ICPolarity;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = 0x00;
    
    TIM_ICInit(GENERAL_TIMx, &TIM_ICInitStructure);
    
    /* 配置中断优先级 */
    NVIC_InitStructure.NVIC_IRQChannel = GENERAL_TIM_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    
    NVIC_Init(&NVIC_InitStructure);
    
    /* 使能捕获中断和更新中断 */
    TIM_ITConfig(GENERAL_TIMx, GENERAL_TIM_IT_CCx | TIM_IT_Update, ENABLE);
    
    /* 使能定时器 */
    TIM_Cmd(GENERAL_TIMx, ENABLE);
}

/**
  * @brief  输出一个大于10us的高电平触发测距
  */
void CS100A_TRIG(void)
{
    GPIO_SetBits(TRIG_GPIO_PORT, TRIG_GPIO_PIN);
    
    /* 延时约15us（180MHz主频） */
    for(volatile uint16_t i = 0; i < 500; i++);
    
    GPIO_ResetBits(TRIG_GPIO_PORT, TRIG_GPIO_PIN);
}

/**
  * @brief  超声波测距模块初始化
  */
void CS100A_Init(void)
{
    CS100A_TRIG_GPIO_Config();
    CS100A_ECHO_GPIO_Config();
    GENERAL_TIM_Config();
}

/**
  * @brief  获取测量距离（单位：cm）
  * @retval 距离值，单位厘米
  */
float CS100A_GetDistance(void) {
    uint32_t time_us;
    float distance;
    uint16_t timeout = 1000;  // 超时计数
    
    // 清除标志位
    TIM_ICUserValueStructure.ucFinishFlag = 0;
    
    // 发送触发信号
    CS100A_TRIG();
    
    // 等待测量完成（加入任务延时，避免阻塞）
    while(TIM_ICUserValueStructure.ucFinishFlag == 0)
    {
        vTaskDelay(pdMS_TO_TICKS(1));  // 让出CPU
        if(--timeout == 0) return -1;   // 超时
    }
    
    // 计算时间
    time_us = TIM_ICUserValueStructure.usPeriod * (GENERAL_TIM_PERIOD + 1) 
              + TIM_ICUserValueStructure.usCtr;
    
    // 计算距离 (cm)
    distance = (float)time_us * 0.017f;
    
    return distance;
}

/**
  * @brief  定时器中断服务函数
  */
void GENERAL_TIM_IRQHANDLER(void)
{
    /* 更新中断 - 计数器溢出 */
    if(TIM_GetITStatus(GENERAL_TIMx, TIM_IT_Update) != RESET)
    {
        TIM_ICUserValueStructure.usPeriod++;
        TIM_ClearITPendingBit(GENERAL_TIMx, TIM_IT_Update);
    }
    
    /* 捕获中断 */
    if(TIM_GetITStatus(GENERAL_TIMx, GENERAL_TIM_IT_CCx) != RESET)
    {
        /* 第一次捕获 - 上升沿 */
        if(TIM_ICUserValueStructure.ucStartFlag == 0)
        {
            TIM_SetCounter(GENERAL_TIMx, 0);
            TIM_ICUserValueStructure.usPeriod = 0;
            TIM_ICUserValueStructure.ucStartFlag = 1;
            
            /* 切换为下降沿捕获 */
            TIM_OC3PolarityConfig(GENERAL_TIMx, GENERAL_TIM_END_ICPolarity);
        }
        /* 第二次捕获 - 下降沿 */
        else
        {
            TIM_ICUserValueStructure.usCtr = GENERAL_TIM_GetCapturex(GENERAL_TIMx);
            TIM_ICUserValueStructure.ucFinishFlag = 1;
            TIM_ICUserValueStructure.ucStartFlag = 0;
            
            /* 切换回上升沿捕获 */
            TIM_OC3PolarityConfig(GENERAL_TIMx, GENERAL_TIM_START_ICPolarity);
        }
        
        TIM_ClearITPendingBit(GENERAL_TIMx, GENERAL_TIM_IT_CCx);
    }
}
