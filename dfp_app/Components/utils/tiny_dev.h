#ifndef __TINY_DEV_H__
#define __TINY_DEV_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "rtthread.h"
#include <string.h>
#include <stdbool.h>

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
    bool  (*init)   	(void *args);
    int   (*read)   	(void *buf, int size);
    int   (*write)  	(const void *buf, int size);
    bool  (*control)	(int cmd, void *args);
}tiny_device_ops, *tiny_device_ops_t;

typedef struct{
    const char * name;
    tiny_device_ops_t dops;
    rt_list_t node;
}tiny_device, *tiny_device_t;

bool tiny_device_register(tiny_device_t dev);
bool tiny_device_unregister(tiny_device_t dev);
tiny_device_t tiny_device_find(const char *name);

#ifdef __cplusplus
}
#endif

#endif 





