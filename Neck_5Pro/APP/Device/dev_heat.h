#ifndef _DEV_HEAT_H
#define _DEV_HEAT_H

#include "n32g45x.h"
#include "rtthread.h"
#include "init.h"
#include "pid.h"

typedef struct{
	pid_t heat_pid;
	u16 heat_cnt;
	FlagStatus heat_run;
}heat_t;

void heat_sftimer_init(void);
void user_tick_hook(void);


#endif


