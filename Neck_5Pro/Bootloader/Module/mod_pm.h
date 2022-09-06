#ifndef __MOD_PM_H
#define __MOD_PM_H

#include "include.h"


//外部调用
extern rt_mq_t mod_pm_mq;
extern rt_event_t mod_pm_evt;


void PMthread_entry(void *para);



#endif

