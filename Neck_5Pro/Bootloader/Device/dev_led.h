#ifndef _DEV_LED_H
#define _DEV_LED_H

#include "n32g45x.h"
#include "dri_aw21024.h"

//reg addr
#define ADDR_RESET		0x7F

#define ADDR_GCFG0		0xAB
#define ADDR_GCFG1		0xAC

#define ADDR_GCR		0x00
#define ADDR_GCR2		0x7A

#define ADDR_PATCFG		0xA0
#define ADDR_PATGO		0xA1

#define ADDR_UPDATE		0x49

#define ADDR_BR			0x01
#define ADDR_COL		0x4A	

#define ADDR_GCOLR		0xA8
#define ADDR_GCOLG		0xA9
#define ADDR_GCOLB		0xAA

#define ADDR_FADEH		0xA6
#define ADDR_FADEL		0xA7

#define ADDR_PATT0		0xA2
#define ADDR_PATT1		0xA3
#define ADDR_PATT2		0xA4
#define ADDR_PATT3		0xA5

//color index
#define RED_COLOR		0x01
#define GREEN_COLOR		0x02
#define BLUE_COLOR		0x03
#define PURPLE_COLOR	0x04

//LED Dev 
typedef struct
{
	uint8_t _ucled_blink_cnt;
	uint8_t _ucled_breath_index;
    ErrorStatus (*LED_Config)(void);
    void (*LED_Sta_Run)(uint8_t _ucColIndex, uint8_t _ucBrightness);
    void (*LED_Mode_Run)(uint8_t _ucModeIndex);
    void (*LED_Neck_BrRun)(FlagStatus _flagRun);
}T_LED_Device;

typedef enum{
	LED_ON_EVT = 0,
	LED_OFF_EVT,
	LED_BLINK_EVT,
	LED_BREATH_EVT
}eLEDEvent;


//外部接口
extern T_LED_Device g_tLED_Dev;



#endif

