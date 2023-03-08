#include "tiny_os.h"

cbFunc idle_handle = NULL;
static TASK_LIST_HEAD(task_list);
volatile uint32_t cnt = 0;

//查询任务是否存在
static bool tiny_task_is_exists(task_t *task)
{
    task_t *cur ;
    list_for_each(cur, &task_list)
    {
        if(cur->timer_id == task->timer_id)
        {
            return true;
        }
    }
    return false;
}

int tiny_task_create(task_t *task, cbFunc func, void *para)
{
    task_t *cur  ;
    OS_CPU_SR cpu_sr;
    enter_critical();
    if((NULL == task )||(tiny_task_is_exists(task)))
    {
        exit_critical();
        return false;
    }
    task->is_task   = TASK_TASK;
    task->is_start  = true;
    task->is_run    = true;
    task->func      = func;
    task->evt       = 0;
    task->user      = para;
    task->is_busy   = TASK_IDLE;
    list_for_null(cur,&task_list)
    {

    }
    task->timer_id = cur->timer_id + 1;
    cur->next  = task;
    task->next = NULL;
    exit_critical();
    return true;
}

int tiny_task_delete(task_t *task)
{
    task_t *cur,*n;
    OS_CPU_SR cpu_sr;
    enter_critical();
    list_for_each_del(cur,n,&task_list)
    {
        if(cur->timer_id == task->timer_id)
        {
            n->next = cur->next;
            exit_critical();
            return true;
        }
    }
    return false;
}

static uint8_t idel_cnt = 0;
void tiny_task_loop(void)
{
    uint32_t events =0;
    task_t *cur,*n=NULL ;
    OS_CPU_SR cpu_sr;
    bool busy = TASK_IDLE;
    
    list_for_each_safe(cur,n,&task_list)
    {
        //检查定时任务是否超时
        if((TASK_TIMER == cur->is_task)&& (cur->is_start) && (time_after_eq(cnt, cur->timer_tick)))
        {
            enter_critical();
            cur->is_run = true;
            cur->timer_tick = cnt + cur->period;
            exit_critical();
        }
        if(cur->is_run)
        {
            if((cur->is_one_shot)&&(TASK_TIMER == cur->is_task))
            {
               cur->is_start = false;
            }
            if(NULL !=cur->func)
            {
                events = cur->evt;
                if(events)
                {
                    enter_critical();
                    cur->evt = 0;
                    exit_critical();
                }
                cur->func(cur->user,events);
            }
            busy |= cur->is_busy;
            if(TASK_TIMER == cur->is_task)
            {
                enter_critical();
                cur->is_run = false;
                exit_critical();
            }
        }
    }
    if(!busy)
    {
        if(++idel_cnt >2 )
        {
            idel_cnt = 0;
            if(idle_handle)
            {
                idel_cnt = 0;
                idle_handle(0,busy);
            }

        }
    }
    else
    {
        idel_cnt = 0;
    }
}

int tiny_timer_create(task_t *timer, cbFunc func, void *para)
{
    task_t *cur ;
    OS_CPU_SR cpu_sr;
    enter_critical();

    if((NULL == timer )||(tiny_task_is_exists(timer)))
    {
        exit_critical();
        return false;
    }
    timer->is_task   	= TASK_TIMER;
    timer->is_run    	= false;
    timer->period    	= 0;
    timer->is_one_shot  = true;
    timer->is_start     = false;
    timer->timer_tick   = 0;
    timer->func      	= func;
    timer->evt     		= 0;
    timer->user	        = para;
    timer->is_busy	    = TASK_IDLE;

    list_for_null(cur,&task_list)
    {

    }
    timer->timer_id = cur->timer_id + 1;
    cur->next   = timer;
    timer->next = NULL;
    exit_critical();
    return true;

}

int tiny_timer_start(task_t *timer, bool one_shot, uint32_t ms)
{
    task_t *cur ;
    OS_CPU_SR cpu_sr;
    enter_critical();
    list_for_each(cur,&task_list)
    {
        if(cur->timer_id == timer->timer_id)
        {
            timer->period    	 = ms;
            timer->is_one_shot   = one_shot;
            timer->is_start      = true;
            timer->timer_tick    = cnt + ms;
            timer->is_task		 = TASK_TIMER;
            exit_critical();
            return true;
        }
    }
    exit_critical();
    return false;
}

int tiny_timer_stop(task_t *timer)
{
    task_t *cur;
    OS_CPU_SR cpu_sr;
    enter_critical();
    list_for_each(cur,&task_list)
    {
        if(cur->timer_id == timer->timer_id)
        {
            timer->is_start = false;
            exit_critical();
            return true;
        }
    }
    exit_critical();
    return false;
}

int tiny_timer_delete(task_t *timer)
{
    return tiny_task_delete(timer);
}

void tiny_timer_loop(void)
{
    cnt++;
}

int tiny_set_event(task_t *task, uint32_t evt)
{
    task_t *cur;
    OS_CPU_SR cpu_sr;
    enter_critical();
    list_for_each(cur,&task_list)
    {
        if(cur->timer_id == task->timer_id)
        {
            cur->evt |= evt;
            exit_critical();
            return true;
        }
    }
    exit_critical();
    return false;
}

int tiny_clr_event(task_t *task, uint32_t evt)
{
    task_t *cur;
    OS_CPU_SR cpu_sr;
    enter_critical();
    list_for_each(cur,&task_list)
    {
        if(cur->timer_id == task->timer_id)
        {
            cur->evt &= ~(evt);
            exit_critical();
            return true;
        }
    }
    exit_critical();
    return false;
}

void tiny_delay_ms(uint32_t ms)
{
    unsigned int start = cnt;
    while((cnt-start) <= ms);
}

int tiny_set_idle_hook(cbFunc func)
{
    idle_handle = func;
    return 0;
}

uint32_t tiny_tick_get(void)
{
    /* return the global tick */
    return cnt;
}

#include "dev_uart.h"
void tiny_printf(const char *fmt, ...)
{
    char buffer[256];
    va_list vargs;
    va_start(vargs, fmt);
    memset(buffer, 0, sizeof(buffer));
    vsnprintf(buffer, sizeof(buffer) - 1, fmt, vargs);
    va_end(vargs);
    
    uart_write(DEV_UART1, (uint8_t *)&buffer[0], strlen(buffer));
}




