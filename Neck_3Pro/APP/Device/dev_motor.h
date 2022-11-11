#ifndef _DEV_MOTOR_H
#define _DEV_MOTOR_H

#include "include.h"

#define MOTOR_EVT			0xAA

typedef struct
{
	uint16_t pwm_curr;
	uint16_t pwm_target;
}motor_t;

#endif




