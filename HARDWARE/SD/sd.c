#include "sd.h"
#include <string.h>
#include <stdio.h>
#include "ff.h"      // FatFS — 需将 ff.c/ff.h/ffconf.h/diskio.c 加入工程
#include "diskio.h"

// ---- SDIO GPIO 初始化 ----
// SDIO 外设时钟及寄存器配置由 FatFS diskio.c 负责；
// 此处仅配置 GPIO 引脚复用。
static void sdio_gpio_init(void)
{
    GPIO_InitTypeDef gi;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD
                           | RCC_APB2Periph_AFIO, ENABLE);

    // PC8~PC11: D0~D3  PC12: CLK — 复用推挽
    gi.GPIO_Pin   = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10
                  | GPIO_Pin_11 | GPIO_Pin_12;
    gi.GPIO_Mode  = GPIO_Mode_AF_PP;
    gi.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &gi);

    // PD2: CMD — 复用推挽
    gi.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOD, &gi);
}

// ---- 内部状态 ----
static FATFS   fs;
static FIL     fil;
static uint8_t sd_ready  = 0;
static uint8_t fil_open  = 0;
static uint8_t sync_cnt  = 0;
static char    line[128];

// ---- 接口实现 ----

uint8_t SD_Init(void)
{
    sdio_gpio_init();
    sd_ready = (f_mount(&fs, "0:", 1) == FR_OK) ? 1 : 0;
    return sd_ready;
}

uint8_t SD_Open(const char *filename)
{
    if (!sd_ready) return 0;
    fil_open = (f_open(&fil, filename, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK) ? 1 : 0;
    sync_cnt = 0;
    return fil_open;
}

void SD_WriteHeader(void)
{
    UINT bw;
    const char *hdr =
        "time_ms,ax_g,ay_g,az_g,vx_ms,vy_ms,vz_ms,px_m,py_m,pz_m\r\n";
    if (!fil_open) return;
    f_write(&fil, hdr, strlen(hdr), &bw);
    f_sync(&fil);
}

void SD_Log(uint32_t t_ms,
            float ax, float ay, float az,
            float vx, float vy, float vz,
            float px, float py, float pz)
{
    UINT bw;
    int  len;
    if (!fil_open) return;

    len = snprintf(line, sizeof(line),
                   "%lu,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\r\n",
                   (unsigned long)t_ms,
                   ax, ay, az, vx, vy, vz, px, py, pz);
    if (len <= 0 || len >= (int)sizeof(line)) return;
    f_write(&fil, line, (UINT)len, &bw);

    // 每写 20 条同步一次，兼顾性能与掉电安全
    if (++sync_cnt >= 20) {
        sync_cnt = 0;
        f_sync(&fil);
    }
}

void SD_Sync(void)
{
    if (fil_open) f_sync(&fil);
}

void SD_Close(void)
{
    if (!fil_open) return;
    f_sync(&fil);
    f_close(&fil);
    fil_open = 0;
}

uint8_t SD_IsReady(void) { return sd_ready; }
