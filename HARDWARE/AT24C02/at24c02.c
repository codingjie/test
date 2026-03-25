#include "at24c02.h"
#include "delay.h"

/* ================================================================
 *  SDA 方向切换
 * ================================================================ */
static void SDA_Out(void) {
    GPIO_InitTypeDef g;
    g.GPIO_Pin   = IIC_SDA_PIN;
    g.GPIO_Mode  = GPIO_Mode_Out_PP;
    g.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IIC_PORT, &g);
}

static void SDA_In(void) {
    GPIO_InitTypeDef g;
    g.GPIO_Pin  = IIC_SDA_PIN;
    g.GPIO_Mode = GPIO_Mode_IPU;    /* 内部上拉，模拟开漏 */
    GPIO_Init(IIC_PORT, &g);
}

/* ================================================================
 *  软件 I2C 初始化
 * ================================================================ */
void IIC_Init(void) {
    GPIO_InitTypeDef g;
    RCC_APB2PeriphClockCmd(IIC_RCC, ENABLE);

    g.GPIO_Pin   = IIC_SCL_PIN | IIC_SDA_PIN;
    g.GPIO_Mode  = GPIO_Mode_Out_PP;
    g.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IIC_PORT, &g);

    IIC_SCL_H;
    IIC_SDA_H;
}

/* ================================================================
 *  I2C 时序基元 (SCL 半周期 ~5µs → ~100kHz)
 * ================================================================ */
static void IIC_Start(void) {
    SDA_Out();
    IIC_SDA_H; delay_us(5);
    IIC_SCL_H; delay_us(5);
    IIC_SDA_L; delay_us(5);   /* SDA↓ while SCL高 = START */
    IIC_SCL_L; delay_us(5);
}

static void IIC_Stop(void) {
    SDA_Out();
    IIC_SCL_L; delay_us(2);
    IIC_SDA_L; delay_us(2);
    IIC_SCL_H; delay_us(5);
    IIC_SDA_H; delay_us(5);   /* SDA↑ while SCL高 = STOP */
}

static void IIC_Ack(void) {
    SDA_Out();
    IIC_SCL_L; delay_us(2);
    IIC_SDA_L; delay_us(2);
    IIC_SCL_H; delay_us(5);
    IIC_SCL_L; delay_us(2);
}

static void IIC_NAck(void) {
    SDA_Out();
    IIC_SCL_L; delay_us(2);
    IIC_SDA_H; delay_us(2);
    IIC_SCL_H; delay_us(5);
    IIC_SCL_L; delay_us(2);
}

/* 返回 0=ACK  1=NACK/超时 */
static uint8_t IIC_WaitAck(void) {
    uint8_t timeout = 200;
    SDA_In();
    delay_us(1);
    IIC_SCL_H; delay_us(5);
    while (IIC_SDA_READ) {
        if (--timeout == 0) { IIC_Stop(); return 1; }
        delay_us(1);
    }
    IIC_SCL_L; delay_us(2);
    return 0;
}

static void IIC_SendByte(uint8_t byte) {
    int i;
    SDA_Out();
    IIC_SCL_L;
    for (i = 7; i >= 0; i--) {
        if (byte & (1u << i)) IIC_SDA_H; else IIC_SDA_L;
        delay_us(2);
        IIC_SCL_H; delay_us(5);
        IIC_SCL_L; delay_us(2);
    }
}

static uint8_t IIC_RecvByte(void) {
    uint8_t byte = 0;
    int i;
    SDA_In();
    for (i = 7; i >= 0; i--) {
        IIC_SCL_L; delay_us(5);
        IIC_SCL_H; delay_us(5);
        if (IIC_SDA_READ) byte |= (1u << i);
    }
    IIC_SCL_L;
    return byte;
}

/* ================================================================
 *  AT24C02 字节读写  (单字节写时序，无页边界问题)
 * ================================================================ */
void AT24C02_WriteByte(uint8_t addr, uint8_t data) {
    IIC_Start();
    IIC_SendByte(AT24C02_ADDR_W);
    IIC_WaitAck();
    IIC_SendByte(addr);
    IIC_WaitAck();
    IIC_SendByte(data);
    IIC_WaitAck();
    IIC_Stop();
    delay_ms(5);    /* tWR 最大 5ms */
}

uint8_t AT24C02_ReadByte(uint8_t addr) {
    uint8_t data;
    /* 伪写 → 设置内部地址 */
    IIC_Start();
    IIC_SendByte(AT24C02_ADDR_W);
    IIC_WaitAck();
    IIC_SendByte(addr);
    IIC_WaitAck();
    /* 重复起始 → 读 */
    IIC_Start();
    IIC_SendByte(AT24C02_ADDR_R);
    IIC_WaitAck();
    data = IIC_RecvByte();
    IIC_NAck();
    IIC_Stop();
    return data;
}

/* ================================================================
 *  EEPROM 记录管理
 *  循环队列：s_wridx = 下一个写入槽 (0-9)
 *            s_cnt   = 有效记录数 (0-10)
 *  idx=0 → 最旧，idx=s_cnt-1 → 最新
 * ================================================================ */
static uint8_t s_wridx = 0;
static uint8_t s_cnt   = 0;

void EEPROM_Init(void) {
    IIC_Init();
    s_wridx = AT24C02_ReadByte(EEPROM_META_WRIDX);
    s_cnt   = AT24C02_ReadByte(EEPROM_META_CNT);
    /* 有效性校验：首次上电 EEPROM 为 0xFF */
    if (s_wridx >= EEPROM_MAX_REC) { s_wridx = 0; s_cnt = 0; }
    if (s_cnt   >  EEPROM_MAX_REC) { s_wridx = 0; s_cnt = 0; }
}

void EEPROM_SaveRecord(uint32_t freq, uint8_t duty) {
    uint8_t base = EEPROM_REC_BASE + s_wridx * EEPROM_REC_SIZE;
    AT24C02_WriteByte(base + 0, (uint8_t)(freq >>  0));
    AT24C02_WriteByte(base + 1, (uint8_t)(freq >>  8));
    AT24C02_WriteByte(base + 2, (uint8_t)(freq >> 16));
    AT24C02_WriteByte(base + 3, (uint8_t)(freq >> 24));
    AT24C02_WriteByte(base + 4, duty);

    s_wridx = (s_wridx + 1) % EEPROM_MAX_REC;
    if (s_cnt < EEPROM_MAX_REC) s_cnt++;
    AT24C02_WriteByte(EEPROM_META_WRIDX, s_wridx);
    AT24C02_WriteByte(EEPROM_META_CNT,   s_cnt);
}

uint8_t EEPROM_GetCount(void) { return s_cnt; }

uint8_t EEPROM_ReadRecord(uint8_t idx, PwmRecord *rec) {
    uint8_t slot, base;
    if (idx >= s_cnt || rec == 0) return 0;
    /* slot = (写指针 - 总数 + idx + MAX) % MAX */
    slot = (uint8_t)((s_wridx - s_cnt + idx + EEPROM_MAX_REC) % EEPROM_MAX_REC);
    base = EEPROM_REC_BASE + slot * EEPROM_REC_SIZE;
    rec->freq = (uint32_t) AT24C02_ReadByte(base + 0)
              | ((uint32_t)AT24C02_ReadByte(base + 1) <<  8)
              | ((uint32_t)AT24C02_ReadByte(base + 2) << 16)
              | ((uint32_t)AT24C02_ReadByte(base + 3) << 24);
    rec->duty = AT24C02_ReadByte(base + 4);
    return 1;
}
