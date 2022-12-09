#ifndef _DRV_GPIO_H
#define _DRV_GPIO_H

#include "n32l40x.h"

#define CHARGE_ENABLE		GPIO_ResetBits(GPIOB, GPIO_PIN_2)
#define CHARGE_DISABLE		GPIO_SetBits(GPIOB, GPIO_PIN_2)
#define DC_INPUT_DETECT		GPIO_ReadInputDataBit(GPIOC, GPIO_PIN_14)
#define CHARGE_VER_DETECT	GPIO_ReadInputDataBit(GPIOC, GPIO_PIN_15)

#define MCU_PWR_ENABLE		GPIO_SetBits(GPIOB, GPIO_PIN_9)
#define MCU_PWR_DISABLE		GPIO_ResetBits(GPIOB, GPIO_PIN_9)

#define BLE_PWR_ENABLE		GPIO_SetBits(GPIOC, GPIO_PIN_13)
#define BLE_PWR_DISABLE		GPIO_ResetBits(GPIOC, GPIO_PIN_13)

#define MOTOR_1_FORWARD		do{GPIOB->PBC = GPIO_PIN_11;GPIOB->PBSC = GPIO_PIN_10;}while(0)
#define MOTOR_1_REVERSE		do{GPIOB->PBC = GPIO_PIN_10;GPIOB->PBSC = GPIO_PIN_11;}while(0)
#define MOTOR_2_FORWARD		do{GPIOB->PBC = GPIO_PIN_13;GPIOB->PBSC = GPIO_PIN_12;}while(0)
#define MOTOR_2_REVERSE		do{GPIOB->PBC = GPIO_PIN_12;GPIOB->PBSC = GPIO_PIN_13;}while(0)
#define MOTOR_3_FORWARD		do{GPIOA->PBC = GPIO_PIN_12;GPIOA->PBSC = GPIO_PIN_11;}while(0)
#define MOTOR_3_REVERSE		do{GPIOA->PBC = GPIO_PIN_11;GPIOA->PBSC = GPIO_PIN_12;}while(0)

#define PSW1_DETECT			GPIO_ReadInputDataBit(GPIOD, GPIO_PIN_15)
#define PSW2_DETECT			GPIO_ReadInputDataBit(GPIOD, GPIO_PIN_14)

#define HEAT_ON				GPIO_SetBits(GPIOA, GPIO_PIN_6)
#define HEAT_OFF			GPIO_ResetBits(GPIOA, GPIO_PIN_6)

#define BT_LED_ON			GPIO_SetBits(GPIOB, GPIO_PIN_5)
#define BT_LED_OFF			GPIO_ResetBits(GPIOB, GPIO_PIN_5)
#define BT_LED_REV			do{GPIOB->POD ^= GPIO_PIN_5;}while(0)
#define BT_LED_DECTECT		GPIO_ReadOutputDataBit(GPIOB, GPIO_PIN_6)
#define RED_LED_ON			GPIO_ResetBits(GPIOA, GPIO_PIN_15)
#define RED_LED_OFF			GPIO_SetBits(GPIOA, GPIO_PIN_15)
#define RED_LED_REV			do{GPIOA->POD ^= GPIO_PIN_15;}while(0)
#define RED_LED_DECTECT		GPIO_ReadOutputDataBit(GPIOA, GPIO_PIN_15)
#define GREEN_LED_ON		GPIO_SetBits(GPIOB, GPIO_PIN_3)
#define GREEN_LED_OFF		GPIO_ResetBits(GPIOB, GPIO_PIN_3)
#define GREEN_LED_REV		do{GPIOB->POD ^= GPIO_PIN_3;}while(0)
#define GREEN_LED_DECTECT	GPIO_ReadOutputDataBit(GPIOB, GPIO_PIN_3)

void enable_charge(void);
void disable_charge(void);


#endif




