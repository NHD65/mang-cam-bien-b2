#ifndef _APP_I2C_H
#define _APP_I2C_H

#include "define.h"  // Ð?ng b? v?i tên file define

void I2C1_config(void);
void I2C1_write(uint8_t HW_address, uint8_t sub, uint8_t data);
char I2C1_read(uint8_t HW_address, uint8_t sub);
void I2C1_read_buf(uint8_t HW_address, uint8_t sub, uint8_t *p_buf, uint8_t buf_size);

#endif
