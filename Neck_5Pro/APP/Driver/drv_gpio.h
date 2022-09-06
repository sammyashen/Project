#ifndef _DRV_GPIO_H
#define _DRV_GPIO_H

#include "n32g45x.h"
#include "init.h"

#define PWR_OFF		GPIO_ResetBits(GPIOB, GPIO_PIN_13)
#define PWR_ON		GPIO_SetBits(GPIOB, GPIO_PIN_13)

#define ESP32_PWR_OFF		GPIO_ResetBits(GPIOC, GPIO_PIN_9)
#define ESP32_PWR_ON		GPIO_SetBits(GPIOC, GPIO_PIN_9)

#define HEAT1_ON		GPIO_SetBits(GPIOA, GPIO_PIN_2)	
#define HEAT1_OFF		GPIO_ResetBits(GPIOA, GPIO_PIN_2)
#define HEAT2_ON		GPIO_SetBits(GPIOA, GPIO_PIN_4)
#define HEAT2_OFF		GPIO_ResetBits(GPIOA, GPIO_PIN_4)

#define BOOST_ENABLE		GPIO_SetBits(GPIOB, GPIO_PIN_2)
#define BOOST_DISABLE		GPIO_ResetBits(GPIOB, GPIO_PIN_2)
#define MOTOR_TRA_FORWARD	GPIO_SetBits(GPIOC, GPIO_PIN_5)				//行程电机正转
#define MOTOR_TRA_REVERSE	GPIO_ResetBits(GPIOC, GPIO_PIN_5)			//行程电机反转
#define MOTOR_MASS_FORWARD	GPIO_SetBits(GPIOC, GPIO_PIN_4)				//按摩电机正转
#define MOTOR_MASS_REVERSE	GPIO_ResetBits(GPIOC, GPIO_PIN_4)			//按摩电机反转
#define TRA_SW1_DETECT		GPIO_ReadInputDataBit(GPIOB, GPIO_PIN_10)	//最内侧检测
#define TRA_SW2_DETECT		GPIO_ReadInputDataBit(GPIOB, GPIO_PIN_11)	//最外侧检测



#define USB_CHK_INPUT		GPIO_ReadInputDataBit(GPIOB, GPIO_PIN_15)


#endif




