#include "perf_port.h"

#ifdef USING_CODE_PERF
/***************************************user need to modify*********************************************/
//don't forget to modify perf_port.h "extern volatile statis_data_t g_statis_data[]" synchronously
volatile statis_data_t g_statis_data[3] = {
//	thread name			func name		
	{"eg_theadd_1", 	"eg_func_0"}, 		//id = 0
	{"eg_theadd_2", 	"eg_func_1"}, 		//id = 1
	{"main", 			"kkk"}, 	
};
/*******************************************************************************************************/

static void list_code_statis(void)
{
	extern uint32_t SystemCoreClock;
	rt_uint8_t name_max_len = RT_NAME_MAX;
	
	for(rt_uint8_t i = 0;i < (sizeof(g_statis_data)/sizeof(g_statis_data[0])); i++){
		rt_kprintf("%.*s%*s ", name_max_len, g_statis_data[i].thread_name, 
						(name_max_len - rt_strlen((char *)g_statis_data[i].thread_name)), " ");
		rt_kprintf("%.*s%*s ", name_max_len, g_statis_data[i].func_name, 
						(name_max_len - rt_strlen((char *)g_statis_data[i].func_name)), " ");
		rt_uint32_t major;
		rt_uint8_t minor;
		float32_t temp;
		temp = 1000.0 * g_statis_data[i].cnt_avg / (float32_t)SystemCoreClock;
		major = (rt_uint32_t)temp;
		minor = (rt_uint8_t)((rt_uint32_t)(temp * 100) % 100);
		rt_kprintf("%8d.%02d ", major, minor);
		temp = 1000.0 * g_statis_data[i].cnt_max / (float32_t)SystemCoreClock;
		major = (rt_uint32_t)temp;
		minor = (rt_uint8_t)((rt_uint32_t)(temp * 100) % 100);
		rt_kprintf("%8d.%02d ", major, minor);
		temp = 1000.0 * g_statis_data[i].cnt_min / (float32_t)SystemCoreClock;
		major = (rt_uint32_t)temp;
		minor = (rt_uint8_t)((rt_uint32_t)(temp * 100) % 100);
		rt_kprintf("%8d.%02d\n", major, minor);
	}
}

static void statistics(void)
{
	extern long list_thread(void);
	extern void list_mem(void);

	rt_kprintf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
	list_thread();
	rt_kprintf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
	list_mem();
	rt_kprintf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
	cpu_load_average();
	rt_kprintf("---------------------------------------------------------------------\n");
	rt_kprintf(" thread           func            cnt_avg(ms) cnt_max(ms) cnt_min(ms)\n");
	rt_kprintf("---------------- ---------------- ----------- ----------- -----------\n");
	list_code_statis();
	rt_kprintf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
}
MSH_CMD_EXPORT(statistics, show statistics data);	

extern void $Super$$SysTick_Handler(void);
void $Sub$$SysTick_Handler(void)
{
	$Super$$SysTick_Handler();
	user_code_insert_to_systick_handler();
}

static int sys_perf_counter_register(void)
{
	init_cycle_counter(true);
	return 0;
}
INIT_COMPONENT_EXPORT(sys_perf_counter_register);


#endif


