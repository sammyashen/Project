#ifndef _INCLUDE_H
#define _INCLUDE_H

#include "n32l40x.h"
#include "n32l40x.h"
#include "tiny_os.h"
#include "topic.h"
#include "init.h"
#include "shell.h"
#include "pid.h"
//#include "ladrc.h"
#include "state_machine.h"
#include "mass.h"
#include "iap.h"
#include "bsp.h"
#include "debug.h"
//#include "cm_backtrace.h"

#include "drv_adc.h"
#include "drv_gpio.h"
#include "drv_key.h"
#include "drv_pwm.h"

#include "dev_uart.h"
#include "dev_led.h"
#include "dev_buzzer.h"
#include "dev_heat.h"

#include <stdlib.h>

#define BYTE0(dwTemp)	(*((uint8_t *)(&dwTemp) + 0))
#define BYTE1(dwTemp)	(*((uint8_t *)(&dwTemp) + 1))
#define BYTE2(dwTemp)	(*((uint8_t *)(&dwTemp) + 2))
#define BYTE3(dwTemp)	(*((uint8_t *)(&dwTemp) + 3))

//temperature
#define TEMP_60			1150
#define TEMP_45			1500
#define TEMP_43			1550

//topic 
#define BUZZER_TOPIC	0x01
#define LED_TOPIC		0x02
#define HEAT_TOPIC		0x03
#define MOTOR_TOPIC		0x04
#define BLE_TOPIC		0x05
#define MASS_TOPIC		0x06
#define CHANGE_TOPIC	0x07

//mass mode
#define MASS_MODE			0x00
#define KNEAD_MODE			0x01
#define NECK_SOOTH_MODE		0x02

//mass speed
#define NONE_SPEED		0x00
#define SLOW_SPEED		0x01
#define MID_SPEED		0x02
#define FAST_SPEED		0x03

//mass dir
#define MASS_FORWARD	0x01
#define MASS_REVERSE	0x02

//dev status
#define SLEEP			0x00
#define WORK			0x01
#define LOW_PWR			0x02
#define CHARGING		0x03
#define CHARGDONE		0x04
#define DEV_ERROR		0x05
#define STOPPING		0x06
#define AGINGDONE		0x07

//ble status
#define DISCONNECTED	0x00
#define CONNECTED		0x01

//heat level
#define HEAT_NONE		0x00
#define HEAT_WARM		0x01

//heart cnt
#define HEART_CNT		0x05

typedef struct{
	uint8_t dev_status;
	uint8_t ble_status;
	
	uint8_t mass_mode;
	uint16_t mass_time;
	uint16_t mass_remain;
	
	uint8_t mass_motor1_speed;
	uint8_t mass_motor1_dir;

	uint8_t mass_motor2_speed;
	uint8_t mass_motor2_dir;

	uint8_t mass_motor3_speed;
	uint8_t mass_motor3_dir;
	
	FlagStatus is_travel;
	uint8_t heat_level;
	FlagStatus is_aging;

	uint8_t bat1_level;
	uint8_t bat2_level;

	uint8_t block_cnt;
	uint8_t aging_cnt;

	FlagStatus is_manual_tra;

	FlagStatus is_handshake;

	uint8_t heart_cnt;

	uint8_t reserve[1];
}__attribute__((packed, aligned(4))) ineck_3pro_t;
extern volatile ineck_3pro_t iNeck_3Pro;

#endif


