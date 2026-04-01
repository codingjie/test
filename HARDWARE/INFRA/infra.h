#ifndef __INFRA_H
#define __INFRA_H

#include "stm32f10x.h"
#include "stm32f10x_exti.h"

#define INFRA_GPIO_PORT    GPIOB
#define INFRA_GPIO_CLK     RCC_APB2Periph_GPIOB
#define INFRA_PIN          GPIO_Pin_0

void INFRA_GPIO_Config(void);

#endif
