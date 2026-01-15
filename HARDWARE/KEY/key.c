#include "key.h"
#include "includes.h"

// 事件标志组
extern OS_FLAG_GRP *EventFlags;

// 初始化按键GPIO
void setupKeyboard(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    // 修改：使能GPIOB时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    // 修改：配置PB1, PB2, PB3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

// 按键扫描任务
void keyResponseTask(void *pdata) {
    INT8U err;
    (void)pdata;

    while (1) {
        // KEY1 - PB1
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0) {
            OSFlagPost(EventFlags, KEY1_FLAG, OS_FLAG_SET, &err);
            while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0) {
                OSTimeDlyHMSM(0, 0, 0, 10);  // 等待松开
            }
        }

        // KEY2 - PB2
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_2) == 0) {
            OSFlagPost(EventFlags, KEY2_FLAG, OS_FLAG_SET, &err);
            while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_2) == 0) {
                OSTimeDlyHMSM(0, 0, 0, 10);
            }
        }

        // KEY3 - PB3
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3) == 0) {
            OSFlagPost(EventFlags, KEY3_FLAG, OS_FLAG_SET, &err);
            while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3) == 0) {
                OSTimeDlyHMSM(0, 0, 0, 10);
            }
        }

        OSTimeDlyHMSM(0, 0, 0, 10);  // 扫描周期
    }
}
