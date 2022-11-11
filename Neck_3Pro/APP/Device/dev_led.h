#ifndef _DEV_LED_H
#define _DEV_LED_H

#include "include.h"

typedef struct 
{
	uint8_t ucEnalbe;
	uint8_t ucState;
	uint16_t usOnTime;
	uint16_t usOffTime;
	uint16_t usCycle;
	uint16_t usCount;
	uint16_t usCycleCount;
}ble_led_t;

#endif


