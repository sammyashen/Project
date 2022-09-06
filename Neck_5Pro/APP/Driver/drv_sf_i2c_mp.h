#ifndef _DRV_SF_I2C_MP_H
#define _DRV_SF_I2C_MP_H

#include "n32g45x.h"


void sf_i2c_mp_init(void);
void sf_i2c_mp_start(void);
void sf_i2c_mp_stop(void);
void sf_i2c_mp_sendbyte(uint8_t _ucByte);
uint8_t sf_i2c_mp_readbyte(void);
uint8_t sf_i2c_mp_waitack(void);
void sf_i2c_mp_ack(void);
void sf_i2c_mp_nack(void);
uint8_t sf_i2c_mp_check_device(uint8_t _Address);

#endif



