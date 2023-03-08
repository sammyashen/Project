#ifndef __TINY_OS_H
#define __TINY_OS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include "hc32_ddl.h"

#define LIST_HEAD_INIT(name) { 0 }

#define TASK_LIST_HEAD(name) \
    struct task_s name = LIST_HEAD_INIT(name)

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != NULL; pos = pos->next)

#define list_for_null(pos, head) \
    for (pos = head; pos->next != NULL; pos = pos->next)

#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next; pos != NULL; \
          n = pos->next,pos = n)

#define list_for_each_del(pos, n, head) \
    for (n = (head),pos = (head)->next; pos != NULL; \
          n = pos,pos = pos->next)

#define time_after_eq(a,b)  (((int)((a) - (b)) >= 0))

#define disable_irq()   __disable_irq()
#define enable_irq()    __enable_irq()    

#define OS_CPU_SR   uint32_t
#define enter_critical()        \
    do { cpu_sr = __get_PRIMASK(); __disable_irq();} while (0)
#define exit_critical()         \
    do { __set_PRIMASK(cpu_sr);} while (0)


enum TASK_EVT
{
    EVT_0  = 1 << 0,
    EVT_1  = 1 << 1,
    EVT_2  = 1 << 2,
    EVT_3  = 1 << 3,
    EVT_4  = 1 << 4,
    EVT_5  = 1 << 5,
    EVT_6  = 1 << 6,
    EVT_7  = 1 << 7,
    EVT_8  = 1 << 8,
    EVT_9  = 1 << 9,
    EVT_10  = 1 << 10,
    EVT_11  = 1 << 11,
    EVT_12  = 1 << 12,
    EVT_13  = 1 << 13,
    EVT_14  = 1 << 14,
    EVT_15  = 1 << 15,
};

enum
{
    TASK_TASK   = 0x00,
    TASK_TIMER  = 0x01,
};

enum
{
	TIMER_FOREVER = 0,
	TIMER_ONE_SHOT,
};

enum
{
	TASK_IDLE = 0,
	TASK_BUSY,
};

typedef void (*cbFunc)(void *para, uint32_t evt);

typedef struct task_s
{
	uint8_t 		timer_id;		//定时器编号
	uint32_t 		period;			//定时周期
	bool 			is_one_shot;	//单词定时
	bool			is_start;		//计时启动
	uint32_t		timer_tick;		//计时器滴答
	bool			is_run;			//任务运行标志
	bool			is_task;		//是主任务还是定时任务
	uint32_t		evt;			//驱动事件
	cbFunc			func;			//回调函数
	bool			is_busy;		//是否空闲
	void			*user;			//用户接口
	struct task_s 	*next;
}task_t;

#define stimer		task_t;
extern volatile unsigned int tick_cnt;

//主任务创建-->事件驱动
int tiny_task_create(task_t *task, cbFunc func, void *para);
//任务删除
int tiny_task_delete(task_t *task);
//任务轮询
void tiny_task_loop(void);

//定时任务创建-->定时驱动
int tiny_timer_create(task_t *timer, cbFunc func, void *para);
//定时任务开始
int tiny_timer_start(task_t *timer, bool one_shot, uint32_t ms);
//定时任务停止
int tiny_timer_stop(task_t *timer);
//删除定时任务
int tiny_timer_delete(task_t *timer);
//定时任务轮询
void tiny_timer_loop(void);

//设置任务事件
int tiny_set_event(task_t *task, uint32_t evt);
//取消任务事件
int tiny_clr_event(task_t *task, uint32_t evt);

//任务阻塞
void tiny_delay_ms(uint32_t ms);

//设置空闲钩子
int tiny_set_idle_hook(cbFunc func);

//获取tick
uint32_t tiny_tick_get(void);

void tiny_printf(const char *fmt, ...);



#endif


