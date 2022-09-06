#include "chart.h"
#include "rtthread.h"

static void update_chart(void *args)
{
	chart_t chart_struct = (chart_t)args;

	rt_kprintf("get notify:%c\r\n",'0'+chart_struct->kk);
}

void Chart_Init(struct Chart* chart)
{
	observer_init(&chart->parant, update_chart);
}

