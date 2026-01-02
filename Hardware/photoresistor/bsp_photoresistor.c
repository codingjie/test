#include "bsp_photoresistor.h"

__IO uint16_t ADC_ConvertedValue;

/**
 * @brief  光敏电阻的GPIO配置
 * @param  无
 * @retval 无
 */
static void PhotoResistor_GPIO_Config(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    // 打开 ADC IO端口时钟
    ADC_GPIO_APBxClock_FUN(ADC_GPIO_CLK, ENABLE);
    // 打开 数字量 IO端口时钟
    PhotoResistor_GPIO_APBxClock_FUN(PhotoResistor_GPIO_CLK, ENABLE);

    // 配置 ADC IO 引脚模式（必须为模拟输入）
    GPIO_InitStructure.GPIO_Pin = ADC_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;        // 模拟输入
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    // 不上拉不下拉
    GPIO_Init(ADC_PORT, &GPIO_InitStructure);

    // 配置 数字量 IO 引脚模式（浮空输入）
    GPIO_InitStructure.GPIO_Pin = PhotoResistor_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;        // 数字输入
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    // 不上拉不下拉
    GPIO_Init(PhotoResistor_PORT, &GPIO_InitStructure);
}

/**
 * @brief  光敏电阻相关的ADC配置
 * @param  无
 * @retval 无
 */
static void PhotoResistor_ADC_Mode_Config(void) {
    ADC_InitTypeDef ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 打开ADC时钟
    ADC_APBxClock_FUN(ADC_CLK, ENABLE);

    // ADC Common 结构体参数初始化
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                        // 独立ADC模式
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;                     // 时钟4分频
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;         // 禁止DMA直接访问模式
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;   // 采样时间间隔
    ADC_CommonInit(&ADC_CommonInitStructure);

    // ADC Init 结构体参数初始化
    ADC_StructInit(&ADC_InitStructure);
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;                          // 12位分辨率
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                                   // 禁止扫描模式
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                              // 连续转换
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;     // 禁止外部触发
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;           // 软件触发时随意赋值
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                          // 数据右对齐
    ADC_InitStructure.ADC_NbrOfConversion = 1;                                      // 1个转换通道
    ADC_Init(ADCx, &ADC_InitStructure);

    // 配置ADC中断
    NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 配置ADC通道，采样时间56周期
    ADC_RegularChannelConfig(ADCx, ADC_CHANNEL, 1, ADC_SampleTime_56Cycles);

    // 使能转换完成中断
    ADC_ITConfig(ADCx, ADC_IT_EOC, ENABLE);

    // 使能ADC
    ADC_Cmd(ADCx, ENABLE);

    // 软件触发开始转换
    ADC_SoftwareStartConv(ADCx);
}

/**
 * @brief  光敏电阻初始化（对外接口）
 * @param  无
 * @retval 无
 */
void PhotoResistor_Init(void) {
    PhotoResistor_GPIO_Config();
    PhotoResistor_ADC_Mode_Config();
}

/**
 * @brief  获取ADC转换值
 * @param  无
 * @retval ADC转换值（0~4095）
 */
uint16_t PhotoResistor_GetValue(void) {
    return ADC_ConvertedValue;
}

/**
 * @brief  获取数字量输出状态
 * @param  无
 * @retval 0: 低电平, 1: 高电平
 */
uint8_t PhotoResistor_GetDigitalState(void) {
    return GPIO_ReadInputDataBit(PhotoResistor_PORT, PhotoResistor_PIN);
}

/**
 * @brief  ADC转换完成中断服务程序
 * @param  无
 * @retval 无
 */
void ADC_IRQHandler(void) {
    if (ADC_GetITStatus(ADCx, ADC_IT_EOC) == SET) {
        ADC_ConvertedValue = ADC_GetConversionValue(ADCx);
    }
    ADC_ClearITPendingBit(ADCx, ADC_IT_EOC);
}
