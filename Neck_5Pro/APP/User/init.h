#ifndef _INIT_H_
#define _INIT_H_


#define  __used  __attribute__((__used__))

typedef void (*initcall_t)(void);

#define __define_initcall(fn, id) \
    static const initcall_t __initcall_##fn##id __used \
    __attribute__((__section__("initcall" #id "init"))) = fn; 

#define sys_initcall(fn)        __define_initcall(fn, 0) //������ϵͳʱ�ӳ�ʼ��  
#define debug_initcall(fn)      __define_initcall(fn, 1) //���Խӿڳ�ʼ��
#define device_initcall(fn)     __define_initcall(fn, 2) //�豸������ʼ��
#define user_initcall(fn)       __define_initcall(fn, 3) //�û�����Ӧ�ò��ʼ��
    

void do_init_call(void);
    
#endif 
