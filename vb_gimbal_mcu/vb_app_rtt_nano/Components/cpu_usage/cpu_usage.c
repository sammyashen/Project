#include "cpu_usage.h"
#include "rtthread.h"

#ifndef CPU_USAGE_PERIOD_TICK
#define CPU_USAGE_PERIOD_TICK 	RT_TICK_PER_SECOND
#endif

static cpu_usage_t *cpu_usage_obj(void)
{
    static struct cpu_usage _usage;
    return &_usage;
}

static void timeout(void *param)
{
    cpu_usage_t *obj = param;

    if (obj->state == CPU_USAGE_STATE_ACTIVATED)
    {
        if (++obj->period == CPU_USAGE_PERIOD_TICK)
        {
            obj->period = 0;
            for(rt_uint8_t i = 0;obj->thread_stat[i].tid != RT_NULL;i++){
            	obj->thread_stat[i].load = obj->thread_stat[i].usage_tick - obj->thread_stat[i].last_tick;
            	obj->thread_stat[i].last_tick = obj->thread_stat[i].usage_tick;
            }
        }

		for(rt_uint8_t i = 0;obj->thread_stat[i].tid != RT_NULL;i++){
			if(rt_thread_self() == obj->thread_stat[i].tid)
				obj->thread_stat[i].usage_tick++;
		}
    }
}

static void cpu_usage_thread_deregister(rt_thread_t thread)
{
	cpu_usage_t *obj = cpu_usage_obj();

	if (obj->state == CPU_USAGE_STATE_ACTIVATED)
	{
		if(thread != RT_NULL){
			for(rt_uint8_t i = 0;obj->thread_stat[i].tid != RT_NULL;i++){
				if(obj->thread_stat[i].tid == thread){
					obj->thread_stat[i].tid = RT_NULL;
					break;
				}
			}
		}else{
			rt_kprintf("can't find this thread.\r\n");
		}
	}
}

extern rt_err_t $Super$$rt_thread_delete(rt_thread_t thread);
rt_err_t $Sub$$rt_thread_delete(rt_thread_t thread)
{
	rt_err_t ret = RT_EOK;
	ret = $Super$$rt_thread_delete(thread);
	if(ret == RT_EOK){
		cpu_usage_thread_deregister(thread);
	}

	return ret;
}

static void cpu_usage_thread_register(rt_thread_t thread)
{
    cpu_usage_t *obj = cpu_usage_obj();
    rt_timer_t t = &obj->time;

	if (rt_object_get_type(&t->parent) == RT_Object_Class_Timer)
	{
        rt_timer_stop(t);
        rt_timer_detach(t);
    }

    if (rt_object_get_type(&t->parent) != RT_Object_Class_Timer)
    {
        /* init timer */
        rt_timer_init(t, "usage", timeout, obj, 1, RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_HARD_TIMER);
		/* get thread handle */
		if(thread != RT_NULL){
			for(rt_uint8_t i = 0;i < (sizeof(obj->thread_stat)/sizeof(_thread_stat_t));i++){
				if(obj->thread_stat[i].tid == RT_NULL){
					obj->thread_stat[i].tid = thread;
					break;
				}else{
					if(obj->thread_stat[i].tid == thread){
						rt_kprintf("thread already registered.\r\n");
						break;
					}
				}

				if(i >= (sizeof(obj->thread_stat)/sizeof(_thread_stat_t) - 1)){
					rt_kprintf("buff is full.thread[%s] register failed.\r\n", thread->name);
				}
			}
		}else{
			rt_kprintf("can't find this thread.\r\n");
		}
        /* set flags */
        obj->state = CPU_USAGE_STATE_ACTIVATED;
        /* start */
        rt_timer_start(t);
    }
}

void cpu_load_average(void)
{
    cpu_usage_t *obj = cpu_usage_obj();
    rt_tick_t usage = 0;
    float load = 0.0;

    if (obj->state == CPU_USAGE_STATE_ACTIVATED)
    {
		for(rt_uint8_t i = 0;obj->thread_stat[i].tid != RT_NULL;i++){
			if(i < (sizeof(obj->thread_stat)/sizeof(_thread_stat_t))){
				usage = obj->thread_stat[i].load;
				load = 100.0 * usage / CPU_USAGE_PERIOD_TICK;
				rt_kprintf("thread:[%s]			---- 		%6.02f%%\r\n", obj->thread_stat[i].tid->name, load);
			}
		}
    }else{
		rt_kprintf("can't cpu usage.\r\n");
    }
}

extern rt_err_t $Super$$rt_thread_startup(rt_thread_t thread);
rt_err_t $Sub$$rt_thread_startup(rt_thread_t thread)
{
	rt_err_t ret = RT_EOK;
	ret = $Super$$rt_thread_startup(thread);
	if(ret == RT_EOK){
		cpu_usage_thread_register(thread);
	}

	return ret;
}


