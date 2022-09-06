#ifndef _DRV_SF_I2C_LED_H
#define _DRV_SF_I2C_LED_H

#include "n32g45x.h"


void sf_i2c_led_init(void);
void sf_i2c_led_start(void);
void sf_i2c_led_stop(void);
void sf_i2c_led_sendbyte(uint8_t _ucByte);
uint8_t sf_i2c_led_readbyte(void);
uint8_t sf_i2c_led_waitack(void);
void sf_i2c_led_ack(void);
void sf_i2c_led_nack(void);
uint8_t sf_i2c_led_check_device(uint8_t _Address);

#endif


