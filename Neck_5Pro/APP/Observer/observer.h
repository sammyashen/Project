#ifndef _OBSERVER_H_
#define _OBSERVER_H_

#include "list.h"

typedef void(*notify_fun_t)(void *args);

typedef struct{
	struct list_head node;
	notify_fun_t notify;
}observer, *observer_t;

void observer_init(observer_t observer, notify_fun_t notify);

#endif /* OBSERVER_H_ */


