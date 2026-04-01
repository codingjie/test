#include "motor.h"

void MOTOR_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(MOTOR_GPIO_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = MOTOR_SERVO_PIN | MOTOR_FAN_PIN | MOTOR_PUMP_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_GPIO_PORT, &GPIO_InitStructure);

    MOTOR_SERVO_OFF();
    MOTOR_FAN_OFF();
    MOTOR_PUMP_OFF();
}
