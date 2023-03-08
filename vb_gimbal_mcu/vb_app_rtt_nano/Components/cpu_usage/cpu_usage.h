#ifndef __CPU_USAGE_H__
#define __CPU_USAGE_H__

#include "rtthread.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CPU_USAGE_STATE_DEACTIVATED  (0x0)
#define CPU_USAGE_STATE_ACTIVATED    (0x1)

typedef struct _thread_stat
{
    rt_tick_t usage_tick;
    rt_tick_t last_tick;
    rt_tick_t load;
    rt_thread_t tid;
}_thread_stat_t;

struct cpu_usage
{
    struct rt_timer time;           /* Period timer */
    rt_uint8_t state;               /* Running state */
    rt_tick_t period;               /* Stat Period */
    _thread_stat_t thread_stat[32];	/* can store 32 thread */
};
typedef struct cpu_usage cpu_usage_t;

void cpu_load_average(void);

#ifdef __cplusplus
}
#endif

#endif
