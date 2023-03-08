#ifndef __INIT_H_
#define __INIT_H_


#define  __used  __attribute__((__used__))

typedef void (*initcall_t)(void);

#define __define_initcall(fn, id) \
    static const initcall_t __initcall_##fn##id __used \
    __attribute__((__section__("initcall" #id "init"))) = fn; 

#define sys_initcall(fn)       __define_initcall(fn, 0) //ϵͳ����ʼ��
#define debug_initcall(fn)     __define_initcall(fn, 1) //���Խӿڼ���ʼ��
#define device_initcall(fn)    __define_initcall(fn, 2) //������ʼ��
#define app_initcall(fn)       __define_initcall(fn, 3) //Ӧ�ò��ʼ��
    

void do_init_call(void);
    
#endif 

