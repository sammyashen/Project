#ifndef _DEV_LED_H
#define _DEV_LED_H

#include "n32g45x.h"
#include "rtthread.h"
#include "init.h"
#include "drv_sf_i2c_led.h"

#define AW21024_ADDR		0x60

//reg addr
#define ADDR_RESET		0x7F

#define ADDR_GCFG0		0xAB
#define ADDR_GCFG1		0xAC

#define ADDR_GCR		0x00
#define ADDR_GCCR		0x6E
#define ADDR_GCR2		0x7A

#define ADDR_PATCFG		0xA0
#define ADDR_PATGO		0xA1

#define ADDR_UPDATE		0x49

#define ADDR_BR			0x01
#define ADDR_COL		0x4A	

#define ADDR_WBR		0x90
#define ADDR_WBG		0x91
#define ADDR_WBB		0x92

#define ADDR_GCOLR		0xA8
#define ADDR_GCOLG		0xA9
#define ADDR_GCOLB		0xAA

#define ADDR_FADEH		0xA6
#define ADDR_FADEL		0xA7

#define ADDR_PATT0		0xA2
#define ADDR_PATT1		0xA3
#define ADDR_PATT2		0xA4
#define ADDR_PATT3		0xA5

typedef enum{
	NONE = 0,
	RED,
	GREEN,
	BLUE,
	PURPLE
}eLEDColor;

typedef enum{
	LED_OFF = 0,
	LED_ON,
	LED_BLINK,
	LED_BREATH,
}eLEDOpt;

typedef struct{
	eLEDOpt opt_type;
	uint8_t disp_num;
	uint8_t is_blink_forever;
	uint8_t blink_cnt;
	uint8_t blink_target;
}mode_led_ctrl_t;

typedef struct{
	eLEDOpt opt_type;
	eLEDColor color;
	uint8_t is_blink_forever;
	uint8_t blink_cnt;
	uint8_t blink_target;
	uint8_t breath_dir;
	uint8_t breath_index;
}sta_led_ctrl_t;

typedef struct{
	mode_led_ctrl_t mode_led_ctrl;
	sta_led_ctrl_t sta_led_ctrl;
}led_ctrl_t;

void led_sftimer_init(void);
void set_background_led(uint8_t brightness);


#endif



