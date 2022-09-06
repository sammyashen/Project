#ifndef __MOD_HMI_H
#define __MOD_HMI_H

#include "include.h"

//消息、事件定义
#define IS_PWR_NORMAL		(1 << 1)
#define IS_CHARGE_HIGH_T	(1 << 2)
#define IS_CHARGE_ING		(1 << 3)
#define IS_CHARGE_DONE		(1 << 4)
#define IS_LOWPWR_NOPROT	(1 << 5)
#define IS_LOWPWR_PROT		(1 << 6)

#define IS_DEV_ON			(1 << 1)
#define IS_DEV_OFF			(1 << 3)

#define IS_SW_MODE			(1 << 2)
#define IS_MOVE_ON			(1 << 3)
#define IS_MOVE_OFF			(1 << 4)
#define IS_MOVE_AUTO_MODE	(1 << 5)
#define IS_MOVE_HAND_MODE	(1 << 6)


//外部调用
extern rt_event_t led_sta_evt;
extern rt_event_t led_mode_evt;
extern rt_event_t beezer_evt;

extern rt_mq_t mod_hmi_mq;

void HMIthread_entry(void *para);



#endif


