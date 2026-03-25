#ifndef __AT24C02_H
#define __AT24C02_H

#include "stm32f10x.h"

/* ================================================================
 *  软件 I2C 引脚  —  PB6=SCL  PB7=SDA  (与硬件 I2C1 同脚，软件模拟)
 * ================================================================ */
#define IIC_PORT      GPIOB
#define IIC_RCC       RCC_APB2Periph_GPIOB
#define IIC_SCL_PIN   GPIO_Pin_6
#define IIC_SDA_PIN   GPIO_Pin_7

#define IIC_SCL_H    (IIC_PORT->BSRR = IIC_SCL_PIN)
#define IIC_SCL_L    (IIC_PORT->BRR  = IIC_SCL_PIN)
#define IIC_SDA_H    (IIC_PORT->BSRR = IIC_SDA_PIN)
#define IIC_SDA_L    (IIC_PORT->BRR  = IIC_SDA_PIN)
#define IIC_SDA_READ ((IIC_PORT->IDR  & IIC_SDA_PIN) != 0)

/* AT24C02 器件地址 (A2=A1=A0=GND → 0x50 七位) */
#define AT24C02_ADDR_W  0xA0
#define AT24C02_ADDR_R  0xA1

/* ================================================================
 *  EEPROM 存储布局
 *
 *  Addr 0      : write_index (下一个写入槽位, 0-9, 循环)
 *  Addr 1      : record_count (有效记录数, 0-10)
 *  Addr 2+n*5  : record[n]  → freq[3:0](uint32 LE) + duty(uint8)
 *
 *  共 10 条 × 5 字节 = 50 字节，末尾地址 51，AT24C02(256B) 完全满足
 * ================================================================ */
#define EEPROM_META_WRIDX   0
#define EEPROM_META_CNT     1
#define EEPROM_REC_BASE     2
#define EEPROM_REC_SIZE     5
#define EEPROM_MAX_REC      10

typedef struct {
    uint32_t freq;   /* Hz */
    uint8_t  duty;   /* 0-100 */
} PwmRecord;

/* 底层 I2C */
void    IIC_Init(void);
void    AT24C02_WriteByte(uint8_t addr, uint8_t data);
uint8_t AT24C02_ReadByte(uint8_t addr);

/* 上层记录管理 */
void    EEPROM_Init(void);
void    EEPROM_SaveRecord(uint32_t freq, uint8_t duty);
uint8_t EEPROM_GetCount(void);
/* idx: 0=最旧, count-1=最新; 返回 1 成功 0 失败 */
uint8_t EEPROM_ReadRecord(uint8_t idx, PwmRecord *rec);

#endif /* __AT24C02_H */
