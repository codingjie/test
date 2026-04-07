#ifndef __SD_H
#define __SD_H

#include "stm32f10x.h"
#include <stdint.h>

// SD卡 SDIO 4-bit接口
// PC8:D0  PC9:D1  PC10:D2  PC11:D3  PC12:CLK  PD2:CMD
//
// 依赖 FatFS 中间件: 将 ff.c / ff.h / ffconf.h / diskio.c
// 放入工程 FATFS/ 目录并加入编译。
// Keil: Target Options → C/C++ → 勾选 "Use MicroLIB" 以支持浮点 printf。
//
// CSV 列: time_ms, ax_g, ay_g, az_g,
//          vx_ms, vy_ms, vz_ms,
//          px_m,  py_m,  pz_m
// ax/ay/az 单位 g（原始传感器值）
// vx/vy/vz 单位 m/s（积分速度）
// px/py/pz 单位 m  （积分位移）

uint8_t SD_Init(void);
uint8_t SD_Open(const char *filename);
void    SD_WriteHeader(void);
void    SD_Log(uint32_t t_ms,
               float ax, float ay, float az,
               float vx, float vy, float vz,
               float px, float py, float pz);
void    SD_Sync(void);
void    SD_Close(void);
uint8_t SD_IsReady(void);

#endif
