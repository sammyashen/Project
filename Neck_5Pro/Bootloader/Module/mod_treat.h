#ifndef __MOD_TREAT_H
#define __MOD_TREAT_H

#include "include.h"


//外部调用
extern rt_mq_t mod_treat_mq;
extern rt_event_t mod_treat_evt;

void TREATthread_entry(void *para);



#endif


