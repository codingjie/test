#ifndef __ADC_H
#define __ADC_H

#include "stm32f10x.h"
#include "stm32f10x_adc.h"

// ADC 通道分配
// PA0 -> ADC1_CH0 -> MQ-2 烟雾传感器
// PA1 -> ADC1_CH1 -> MQ-7 CO 传感器
#define ADC_CH_SMOKE   ADC_Channel_0
#define ADC_CH_CO      ADC_Channel_1

void    ADC1_GPIO_Config(void);
void    ADC1_Config(void);
uint16_t ADC1_GetValue(uint8_t channel);

#endif
