#include "app_i2c.h"

#define I2C_TIMEOUT 10000

// C?u hình I2C1
void I2C_Config(void)
{
    // Kích ho?t clock cho GPIOB, AFIO và I2C1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    // C?u hình chân PB6 (SCL) và PB7 (SDA) ? ch? d? AF_OD, t?c d? 50MHz
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // C?u hình I2C1
    I2C_InitTypeDef I2C_InitStructure;
    I2C_InitStructure.I2C_ClockSpeed = 200000; // 200kHz
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Disable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &I2C_InitStructure);

    // Kích ho?t I2C1
    I2C_Cmd(I2C1, ENABLE);
}

// Ghi 1 byte t?i slave I2C
void I2C1_write(uint8_t HW_address, uint8_t sub, uint8_t data)
{
    uint32_t ticks = I2C_TIMEOUT;

    // START
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) && --ticks);
    if (ticks == 0) return;

    // G?i d?a ch? thi?t b? (write)
    ticks = I2C_TIMEOUT;
    I2C_Send7bitAddress(I2C1, HW_address, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && --ticks);
    if (ticks == 0) return;

    // G?i thanh ghi con n?u sub h?p l?
    if (sub != 0xFF) {
        ticks = I2C_TIMEOUT;
        I2C_SendData(I2C1, sub);
        while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) && --ticks);
        if (ticks == 0) return;
    }

    // G?i d? li?u
    ticks = I2C_TIMEOUT;
    I2C_SendData(I2C1, data);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) && --ticks);
    if (ticks == 0) return;

    // STOP
    I2C_GenerateSTOP(I2C1, ENABLE);
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) && --ticks);
}

// Ð?c 1 byte t? slave I2C
char I2C1_read(uint8_t HW_address, uint8_t sub)
{
    uint32_t ticks = I2C_TIMEOUT;
    uint8_t data = 0;

    // START l?n 1
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) && --ticks);
    if (ticks == 0) return 0;

    // G?i d?a ch? thi?t b? (write)
    ticks = I2C_TIMEOUT;
    I2C_Send7bitAddress(I2C1, HW_address, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && --ticks);
    if (ticks == 0) return 0;

    // G?i thanh ghi con n?u c?n
    if (sub != 0xFF) {
        ticks = I2C_TIMEOUT;
        I2C_SendData(I2C1, sub);
        while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) && --ticks);
        if (ticks == 0) return 0;
    }

    // START l?n 2
    ticks = I2C_TIMEOUT;
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) && --ticks);
    if (ticks == 0) return 0;

    // G?i d?a ch? thi?t b? (read)
    ticks = I2C_TIMEOUT;
    I2C_AcknowledgeConfig(I2C1, DISABLE);
    I2C_Send7bitAddress(I2C1, HW_address, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && --ticks);
    if (ticks == 0) return 0;

    // Nh?n d? li?u
    ticks = I2C_TIMEOUT;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) && --ticks);
    if (ticks == 0) return 0;
    data = I2C_ReceiveData(I2C1);

    // STOP
    I2C_GenerateSTOP(I2C1, ENABLE);
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) && --ticks);

    return data;
}

// Ð?c nhi?u byte t? slave I2C
void I2C1_read_buf(uint8_t HW_address, uint8_t sub, uint8_t *p_buf, uint8_t buf_size)
{
    uint32_t ticks = I2C_TIMEOUT;

    // START l?n 1
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) && --ticks);
    if (ticks == 0) return;

    // G?i d?a ch? thi?t b? (write)
    ticks = I2C_TIMEOUT;
    I2C_Send7bitAddress(I2C1, HW_address, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && --ticks);
    if (ticks == 0) return;

    // G?i thanh ghi con n?u c?n
    if (sub != 0xFF) {
        ticks = I2C_TIMEOUT;
        I2C_SendData(I2C1, sub);
        while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) && --ticks);
        if (ticks == 0) return;
    }

    // START l?n 2
    ticks = I2C_TIMEOUT;
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) && --ticks);
    if (ticks == 0) return;

    // G?i d?a ch? thi?t b? (read)
    ticks = I2C_TIMEOUT;
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    I2C_Send7bitAddress(I2C1, HW_address, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && --ticks);
    if (ticks == 0) return;

    // Ð?c d? li?u
    for (uint8_t i = 0; i < buf_size; i++) {
        ticks = I2C_TIMEOUT;
        if (i == (buf_size - 1)) {
            I2C_AcknowledgeConfig(I2C1, DISABLE);
        }
        while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) && --ticks);
        if (ticks == 0) return;
        p_buf[i] = I2C_ReceiveData(I2C1);
    }

    // STOP
    I2C_GenerateSTOP(I2C1, ENABLE);
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
}

// Kh?i t?o BH1750
void BH1750_Init(void)
{
    I2C_Config();
    I2C1_write(BH1750_ADDR, 0xFF, BH1750_POWER_ON);
    I2C1_write(BH1750_ADDR, 0xFF, BH1750_ONE_H_MODE);
}

// Ð?c d? li?u ánh sáng t? BH1750
float BH1750_ReadLux(void)
{
    uint8_t tmp[2];
    I2C1_read_buf(BH1750_ADDR, 0xFF, tmp, 2);
    uint16_t raw = (tmp[0] << 8) | tmp[1];
    return (float)raw / 1.2;
}
