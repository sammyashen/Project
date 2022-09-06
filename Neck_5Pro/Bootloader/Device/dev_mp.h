#ifndef _DEV_MP_H
#define _DEV_MP_H

#include "n32g45x.h"
#include "dri_mp2651.h"

//reg addr
#define ADDR_REG05	0x05
#define ADDR_REG06	0x06
#define ADDR_REG07	0x07
#define ADDR_REG08	0x08
#define ADDR_REG09	0x09
#define ADDR_REG0A	0x0A
#define ADDR_REG0B	0x0B
#define ADDR_REG0C	0x0C
#define ADDR_REG0D	0x0D
#define ADDR_REG0E	0x0E
#define ADDR_REG0F	0x0F
#define ADDR_REG10	0x10
#define ADDR_REG11	0x11
#define ADDR_REG12	0x12
#define ADDR_REG14	0x14
#define ADDR_REG15	0x15
#define ADDR_REG18	0x18
#define ADDR_REG19	0x19



//only read reg
#define ADDR_REG16	0x16
#define ADDR_REG17	0x17
#define ADDR_REG22	0x22
#define ADDR_REG23	0x23
#define ADDR_REG24	0x24
#define ADDR_REG25	0x25
#define ADDR_REG26	0x26
#define ADDR_REG27	0x27
#define ADDR_REG28	0x28
#define ADDR_REG29	0x29
#define ADDR_REG2A	0x2A
#define ADDR_REG2B	0x2B
#define ADDR_REG2C	0x2C
#define ADDR_REG2D	0x2D


//外部接口
ErrorStatus MP_DEV_Configure(void);




#endif


