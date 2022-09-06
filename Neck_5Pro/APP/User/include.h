#ifndef _INCLUDE_H
#define _INCLUDE_H

#include "n32g45x.h"
#include "rtthread.h"
#define DBG_TAG	"main"
#define DBG_LVL	DBG_LOG
#include <rtdbg.h>

typedef enum{
	BUZZER_YOWL = 1,
	BUZZER_TONE,
	DEV_WORK,
	DEV_NO_WORK,
	DEV_STA_UPDATE,
	DEV_MODE_CHANGE,
	DEV_BLE_CMD,
	DEV_HEAT_SW,
	DEV_MANUAL_SW,
	MANUAL_START,
	MANUAL_STOP,
	UPDATE_HEART_PACK,
}eSYS_TOPIC;

typedef enum{
	NONE_STA = 0,
	DEV_NORMAL,
	DEV_LOW_PWR,
	DEV_LOW_PWR_PORT,
	DEV_CHARGING,
	DEV_CHARGE_DONE,
	DEV_CHARGE_PORT,
	DEV_BLE_CONNECTED,
	DEV_AGING,
}eDEV_STA;

typedef enum{
	NONE_MODE = 0,
	SOFT_MODE,
	MID_MODE,
	HARD_MODE,
	USER_MODE,
}eMASS_MODE;

typedef enum{
	NONE_STRENGTH = 0,
	SOFT_STRENGTH,
	MID_STRENGTH,
	HARD_STRENGTH,
}eMASS_STRENGTH;

typedef enum{
	MASS_FORWARD = 0,
	MASS_REVERSE,
}eMASS_DIR;

typedef enum{
	NONE_SPEED = 0,
	SLOW_SPEED,
	MID_SPEED,
	HARD_SPEED,
}eTRA_SPEED;

typedef enum{
	TRA_FORWARD = 0,
	TRA_REVERSE,
}eTRA_DIR;

typedef enum{
	NONE_HEAT = 0,
	WARM_HEAT,
	HOT_HEAT,
}eHEAT_LEVEL;

typedef enum{
	PLACE_0 = 0,
	PLACE_1,
	PLACE_2,
	PLACE_3,
	PLACE_4,
	PLACE_5,
	PLACE_6,
	PLACE_7,
}eMASS_PLACE;

typedef enum{
	MOVING = 0,
	OUTSIDE,
	MIDDLE,
	INSIDE,
}eCURR_PLACE;

typedef struct{
	eMASS_MODE mass_mode;
	eMASS_STRENGTH mass_strength;
	eMASS_DIR mass_dir;
	eHEAT_LEVEL heat_level;
	uint8_t curr_temp;
	uint16_t mass_time;
	uint16_t remain_time;
	eMASS_PLACE mass_place;
	eCURR_PLACE curr_place;
	FlagStatus is_manual;
	uint8_t batt_val;
}ble_pack_t;

typedef struct{
	uint16_t mass_cnt;
	eTRA_SPEED tra_speed;
	eTRA_DIR tra_dir;
	FlagStatus manual_tra_start;
	FlagStatus is_dev_on;
	FlagStatus is_dev_work;
	FlagStatus is_aging_done;
}global_t;

typedef struct{
	rt_uint8_t is_save_recipe;
	eHEAT_LEVEL heat_recipe;
	eMASS_STRENGTH strength_recipe;
	eMASS_DIR dir_recipe;
	eMASS_PLACE place_recipe;
}flash_para_t;

typedef struct{
	eDEV_STA dev_status;
	eDEV_STA ble_con_pre_sta;//保存ble连接时当前设备状态
	ble_pack_t ble_pack;
	global_t dev_global;
	flash_para_t flash_para;
	uint8_t  low_pwr_flag;
}neck_5pro_t;

neck_5pro_t *get_neck_5pro_struct(void);

#endif


