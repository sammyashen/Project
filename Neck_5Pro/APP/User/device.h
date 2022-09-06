#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "init.h"
#include "rtthread.h"
#include "list.h"
#include <string.h>
#include <stdbool.h>

typedef struct{
    bool  (*init)   	(void *args);
    int   (*read)   	(void *buffer, int size);
    int   (*write)  	(const void *buffer, int size);
    bool  (*control)	(int cmd, void *args);
}mini_device_ops, *mini_device_ops_t;

typedef struct{
    const char * name;
    mini_device_ops_t dops;
    struct list_head node;
}mini_device, *mini_device_t;

//外部接口
bool mini_device_register(mini_device_t dev);
mini_device_t mini_device_find(const char *name);


#endif 

