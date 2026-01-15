#include "sys.h"
#include "key.h"
#include "motor.h"
#include "delay.h"
#include "includes.h"
#include "lcd.h"

// 事件标志组定义
OS_FLAG_GRP *EventFlags;

// 初始化任务配置
#define INIT_TASK_PRIO 10
#define INIT_TASK_STK_SIZE 64
OS_STK INIT_TASK_STK[INIT_TASK_STK_SIZE];
void systemInitTask(void *pdata);

// 显示任务配置
#define DISPLAY_TASK_PRIO 8
#define DISPLAY_TASK_STK_SIZE 64
OS_STK DISPLAY_TASK_STK[DISPLAY_TASK_STK_SIZE];
void displayUpdateTask(void *pdata);

// 电机任务配置
#define MOTOR_TASK_PRIO 7
#define MOTOR_TASK_STK_SIZE 64
OS_STK MOTOR_TASK_STK[MOTOR_TASK_STK_SIZE];
void brushlessMotorTask(void *pdata);

// 按键响应任务配置
#define KEY_RESP_TASK_PRIO 6
#define KEY_RESP_TASK_STK_SIZE 64
OS_STK KEY_RESP_TASK_STK[KEY_RESP_TASK_STK_SIZE];

void keyResponseTask(void *pdata);

int main(void) {
	// 配置中断优先级分组
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	// 初始化延时功能
	delay_init();

	// 初始化外设
	setupKeyboard();        // 三个按键(KEY1/KEY2/KEY3)
	setupBrushlessMotor();  // 无刷电机PWM
	setupMotorADC();        // ADC采集
    setupLCDDisplay();

	// 初始化RTOS
	OSInit();

	// 创建初始化任务
	OSTaskCreate(systemInitTask, (void *)0,
	             (OS_STK *)&INIT_TASK_STK[INIT_TASK_STK_SIZE - 1],
	             INIT_TASK_PRIO);

	// 启动多任务调度
	OSStart();
}

// 系统初始化任务
void systemInitTask(void *pdata) {
	INT8U err;
	OS_CPU_SR cpu_sr = 0;

	OS_ENTER_CRITICAL(); // 进入临界区

	// 创建事件标志组
	EventFlags = OSFlagCreate(0x00, &err);

    // 创建按键扫描任务
    OSTaskCreate(keyResponseTask, (void *)0,
                (OS_STK *)&KEY_RESP_TASK_STK[KEY_RESP_TASK_STK_SIZE - 1],
                KEY_RESP_TASK_PRIO);

	// 创建电机控制任务
	OSTaskCreate(brushlessMotorTask, (void *)0,
	             (OS_STK *)&MOTOR_TASK_STK[MOTOR_TASK_STK_SIZE - 1],
	             MOTOR_TASK_PRIO);

	// 创建显示更新任务
	OSTaskCreate(displayUpdateTask, (void *)0,
	             (OS_STK *)&DISPLAY_TASK_STK[DISPLAY_TASK_STK_SIZE - 1],
	             DISPLAY_TASK_PRIO);

	// 挂起初始化任务
	OSTaskSuspend(INIT_TASK_PRIO);

	OS_EXIT_CRITICAL(); // 退出临界区
}
