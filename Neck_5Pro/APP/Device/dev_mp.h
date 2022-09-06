#ifndef _DEV_MP_H
#define _DEV_MP_H

#include "n32g45x.h"
#include "rtthread.h"
#include "init.h"
#include "drv_sf_i2c_mp.h"

#define MP2651_ADDR			0x10
#define SGM41513_ADDR		0x34

//mp2651 reg
//reg addr
#define MP2651_REG05	0x05
#define MP2651_REG06	0x06
#define MP2651_REG07	0x07
#define MP2651_REG08	0x08
#define MP2651_REG09	0x09
#define MP2651_REG0A	0x0A
#define MP2651_REG0B	0x0B
#define MP2651_REG0C	0x0C
#define MP2651_REG0D	0x0D
#define MP2651_REG0E	0x0E
#define MP2651_REG0F	0x0F
#define MP2651_REG10	0x10
#define MP2651_REG11	0x11
#define MP2651_REG12	0x12
#define MP2651_REG14	0x14
#define MP2651_REG15	0x15
#define MP2651_REG18	0x18
#define MP2651_REG19	0x19

//only read reg
#define MP2651_REG16	0x16
#define MP2651_REG17	0x17
#define MP2651_REG22	0x22
#define MP2651_REG23	0x23
#define MP2651_REG24	0x24
#define MP2651_REG25	0x25
#define MP2651_REG26	0x26
#define MP2651_REG27	0x27
#define MP2651_REG28	0x28
#define MP2651_REG29	0x29
#define MP2651_REG2A	0x2A
#define MP2651_REG2B	0x2B
#define MP2651_REG2C	0x2C
#define MP2651_REG2D	0x2D

//sgm41513 reg
#define SGM41513_REG00	0x00
#define SGM41513_REG01	0x01
#define SGM41513_REG02	0x02
#define SGM41513_REG03	0x03
#define SGM41513_REG04	0x04
#define SGM41513_REG05	0x05
#define SGM41513_REG06	0x06
#define SGM41513_REG07	0x07
#define SGM41513_REG08	0x08
#define SGM41513_REG09	0x09
#define SGM41513_REG0A	0x0A
#define SGM41513_REG0B	0x0B
#define SGM41513_REG0C	0x0C
#define SGM41513_REG0D	0x0D
#define SGM41513_REG0E	0x0E
#define SGM41513_REG0F	0x0F




uint8_t get_mp2651_chargesta(void);
uint16_t get_mp2651_chargecurr(void);
uint8_t get_sgm41513_chargesta(void);
void mpdevice_charge_init(void);
void mpdevice_off_init(void);


#endif




