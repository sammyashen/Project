#ifndef _DEV_MOTOR_H
#define _DEV_MOTOR_H

#include "n32g45x.h"
#include "rtthread.h"
#include "init.h"
#include "drv_pwm.h"

#define FG2_THRESHOLD		180
#define FG2_FILTER			10

#define FG1_THRESHOLD		80
#define FG1_FILTER			30

#define KARTUN_T			20
#define SPEED_STEP			200


typedef enum{
	CHECK_PLACE_PRE = 0,
	CHECK_PLACE,
	GO_INSIDE,
	GO_MIDDLE,
	GO_OUTSIDE,
	START_MASS,
	STOP_MASS,
}eMODE_OPT;

typedef enum{
	GO_VRF_PLACE = 0,
	START_TARGET_PLACE,
	STOP_TARGET_PLACE,
}eMODE_OPTnd;

typedef struct{
	eMODE_OPT mode_opt;
	eMODE_OPTnd mode_optnd;

	uint16_t mass_speed_target;
	uint16_t mass_speed_pwm;
	uint8_t  mass_block;
	uint16_t mass_speed_fb;
	
	uint16_t tra_speed_target;
	uint16_t tra_speed_pwm;
	uint8_t  tra_block;
	uint16_t tra_speed_fb;
}dev_motor_t;

typedef enum{
	eFIND_TROUGH = 0,
	eRESET_SPEED,
}eKARTUN_STA;

typedef struct{
	eKARTUN_STA kartun_sta;
	int Ek;
	int Ek_1;
	uint8_t Sample;
	uint16_t Pv;
}kartun_t;


extern __IO uint16_t g_usFG1Freq;
extern __IO uint16_t g_usFG2Freq;


void motor_create_thread(void);
void motor_sftimer_init(void);


#endif




