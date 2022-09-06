#ifndef _DEV_BUZZER_H
#define _DEV_BUZZER_H

#include "n32g45x.h"
#include "rtthread.h"
#include "init.h"
#include "drv_pwm.h"

typedef struct _BEEP_T
{
	uint8_t ucEnalbe;
	uint8_t ucState;
	uint16_t usBeepTime;
	uint16_t usStopTime;
	uint16_t usCycle;
	uint16_t usCount;
	uint16_t usCycleCount;
	uint32_t uiFreq;
}BEEP_T;

void beep_process(void);

void buzzer_sftimer_init(void);


#endif




