#ifndef __DRI_SYSCLK_H
#define __DRI_SYSCLK_H

#include "n32g45x.h"

typedef enum{
	USE_HSE_FOR_CLKSRC = 0,
	USE_HSI_FOR_CLKSRC
}eSelectClkSrc;

typedef struct
{
    void (*SysClkConfig)(eSelectClkSrc _ClkSrc, uint32_t _ClkFreq);
}T_SysClk_Device;

//外部调用
extern T_SysClk_Device g_tSysClk_Dev;

void SetSysClockToHSI(void);
void SetSysClockToHSE(void);
void SetSysClockToPLL(uint32_t freq, uint8_t src);

#endif

