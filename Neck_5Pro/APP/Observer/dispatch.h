#ifndef _DISPATCH_H_
#define _DISPATCH_H_

#include "observer.h"
#include "list.h"
#include "init.h"

typedef struct dispatch *dispatch_t;

struct dispatch{
	void *args;
	struct list_head list;
	void (*add)(dispatch_t dispatch_struct, observer_t observer_struct);
	void (*remove)(dispatch_t dispatch_struct, observer_t observer_struct);
	void (*notify)(dispatch_t dispatch_struct);
};

extern struct dispatch dispatch_struct;


#endif /* SUBJECT_H_ */


