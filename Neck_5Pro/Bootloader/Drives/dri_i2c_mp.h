#ifndef _DRI_I2C_MP_H
#define _DRI_I2C_MP_H

#include "n32g45x.h"

#define I2C_MP_WR	0		/* Ð´¿ØÖÆbit */
#define I2C_MP_RD	1		/* ¶Á¿ØÖÆbit */

void bsp_InitI2C_MP(void);
void i2c_Start_MP(void);
void i2c_Stop_MP(void);
void i2c_SendByte_MP(uint8_t _ucByte);
uint8_t i2c_ReadByte_MP(void);
uint8_t i2c_WaitAck_MP(void);
void i2c_Ack_MP(void);
void i2c_NAck_MP(void);
uint8_t i2c_CheckDevice_MP(uint8_t _Address);

#endif

