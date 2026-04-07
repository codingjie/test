#ifndef __ADC_H
#define __ADC_H

#include "stm32f10x.h"
#include "stm32f10x_adc.h"

extern uint16_t ADCValues[3];

void ADC1_GPIO_Config(void);
void ADC1_Config(void);
uint16_t ADC1_GetValue(uint8_t channel);
void ADC1_ReadAll(void);

#endif
