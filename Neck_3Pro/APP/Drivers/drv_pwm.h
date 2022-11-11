#ifndef __DRV_PWM_H
#define __DRV_PWM_H

#include "n32l40x.h"

void bsp_SetTIMOutPWM(GPIO_Module* GPIOx, uint16_t GPIO_Pin, uint32_t GPIO_ALTx, TIM_Module* TIMx, uint8_t _ucChannel,
	 uint32_t _ulFreq, uint32_t _ulDutyCycle);
void bsp_SetTIMOutPWM_N(GPIO_Module* GPIOx, uint16_t GPIO_Pin, uint32_t GPIO_ALTx, TIM_Module* TIMx, uint8_t _ucChannel,
	 uint32_t _ulFreq, uint32_t _ulDutyCycle);	


#endif




