#ifndef __PERF_PORT_H__
#define __PERF_PORT_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "perf_counter.h"
#include "rtthread.h"
#include "n32l40x.h"
#include "cpu_usage.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

#define USING_CODE_PERF

#ifdef USING_CODE_PERF

typedef struct{
	rt_uint8_t thread_name[RT_NAME_MAX];
	rt_uint8_t func_name[RT_NAME_MAX];
	rt_uint8_t index;
	rt_uint32_t cnt[16];
	rt_uint32_t cnt_avg;
	rt_uint32_t cnt_max;
	rt_uint32_t cnt_min;
	rt_uint8_t reserve[1];
}__attribute__((packed, aligned(4))) statis_data_t;
/***************************************user need to modify*********************************************/
extern volatile statis_data_t g_statis_data[3];
/*******************************************************************************************************/

#define _code_perf_start(id)																						\
	rt_uint32_t cnt_##id##_eg = 0;																					\
	__cycleof__("eg", {cnt_##id##_eg = __cycle_count__;})															

#define _code_perf_end(id)																							\
	do{																												\
		if(id > (sizeof(g_statis_data)/sizeof(statis_data_t) - 1)){													\
			__perf_counter_printf__("[%s] : line = %d, id error.\r\n", __func__, __LINE__);							\
		}else{																										\
			if(g_statis_data[id].cnt_min == 0){																		\
				g_statis_data[id].cnt_avg = g_statis_data[id].cnt_max = g_statis_data[id].cnt_min = cnt_##id##_eg;	\
			}else{																									\
				if(cnt_##id##_eg > g_statis_data[id].cnt_max)		g_statis_data[id].cnt_max = cnt_##id##_eg;		\
				if(cnt_##id##_eg < g_statis_data[id].cnt_min)		g_statis_data[id].cnt_min = cnt_##id##_eg;		\
			}																										\
			g_statis_data[id].cnt[g_statis_data[id].index++] = cnt_##id##_eg;										\
			if(g_statis_data[id].index >= (sizeof(g_statis_data[id].cnt)/sizeof(g_statis_data[id].cnt[0]))){		\
				rt_uint32_t sum = 0;																				\
				for(rt_uint8_t j = 0;j < g_statis_data[id].index;j++){												\
					sum += g_statis_data[id].cnt[j];																\
				}																									\
				g_statis_data[id].cnt_avg = sum>>4;																	\
				g_statis_data[id].index = 0;																		\
			}																										\
		}																											\
	}while(0)
#else
#define _code_perf_start(id)	__NOP();
#define _code_perf_end(id)		__NOP()
#endif

#ifdef __cplusplus
}
#endif

#endif 






