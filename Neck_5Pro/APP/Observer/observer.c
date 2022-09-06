#include "observer.h"

void observer_init(observer_t observer, notify_fun_t notify)
{
	observer->node.next = NULL;
	observer->node.prev = NULL;
	observer->notify = notify;
}


