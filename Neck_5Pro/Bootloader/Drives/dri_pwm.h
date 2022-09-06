#ifndef __DRI_PWM_H
#define __DRI_PWM_H

#include "n32g45x.h"



typedef struct
{
    void (*TIMOutputPWM)(GPIO_Module* GPIOx, uint16_t GPIO_Pin, TIM_Module* TIMx, uint8_t _ucChannel,
	 uint32_t _ulFreq, uint32_t _ulDutyCycle);
	void(*TIMOutputPWM_N)(GPIO_Module* GPIOx, uint16_t GPIO_Pin, TIM_Module* TIMx, uint8_t _ucChannel,
	 uint32_t _ulFreq, uint32_t _ulDutyCycle);
}T_PWM_Device;



typedef struct
{
	T_PWM_Device *pwm_TIM1CH1_dev;
}T_PWM_Model;


//外部调用
void Register_PWM_Model(T_PWM_Model *_ptPWM_Model);
void bsp_SetTIMOutPWM(GPIO_Module* GPIOx, uint16_t GPIO_Pin, TIM_Module* TIMx, uint8_t _ucChannel,
	 uint32_t _ulFreq, uint32_t _ulDutyCycle);
void bsp_SetTIMOutPWM_N(GPIO_Module* GPIOx, uint16_t GPIO_Pin, TIM_Module* TIMx, uint8_t _ucChannel,
	 uint32_t _ulFreq, uint32_t _ulDutyCycle);	



#endif




