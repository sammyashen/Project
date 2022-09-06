#ifndef __DRI_BEEP_H
#define __DRI_BEEP_H

#include "n32g45x.h"
#include "dri_pwm.h"

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

typedef struct
{
    void (*BEEP_Config)(void);
    void (*BEEP_On)(uint32_t _uiFreq, uint16_t _usBeepTime, uint16_t _usStopTime, uint16_t _usCycle);
    void (*BEEP_Off)(void);
    void (*BEEP_Handler)(void);
}T_BEEP_Device;



typedef struct
{
	T_BEEP_Device *beep_dev;
}T_BEEP_Model;


//外部调用
void Register_BEEP_Model(T_BEEP_Model *_ptBEEP_Model);


#endif



