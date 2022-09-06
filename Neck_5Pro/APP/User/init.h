#ifndef _INIT_H_
#define _INIT_H_


#define  __used  __attribute__((__used__))

typedef void (*initcall_t)(void);

#define __define_initcall(fn, id) \
    static const initcall_t __initcall_##fn##id __used \
    __attribute__((__section__("initcall" #id "init"))) = fn; 

#define sys_initcall(fn)        __define_initcall(fn, 0) //可用作系统时钟初始化  
#define debug_initcall(fn)      __define_initcall(fn, 1) //调试接口初始化
#define device_initcall(fn)     __define_initcall(fn, 2) //设备驱动初始化
#define user_initcall(fn)       __define_initcall(fn, 3) //用户级或应用层初始化
    

void do_init_call(void);
    
#endif 
