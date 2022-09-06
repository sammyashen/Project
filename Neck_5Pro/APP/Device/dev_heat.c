#include "dev_heat.h"
#include "drv_gpio.h"
#include "dev_adc.h"
#include "include.h"
#include "topic.h"
#include "pid.h"

const uint16_t gc_heat_target[3] = {0, 2680, 2780};/* 40、43 */

heat_t ntc1_heat_struct,ntc2_heat_struct;
rt_timer_t heat_sftimer = RT_NULL;

static void ntc1_heat_output(void)
{
	ntc1_heat_struct.heat_cnt++;
	if(ntc1_heat_struct.heat_cnt >= ntc1_heat_struct.heat_pid.pwmcycle)
		ntc1_heat_struct.heat_cnt = 0;

	if(ntc1_heat_struct.heat_cnt < ntc1_heat_struct.heat_pid.OUT)
		HEAT1_ON;
	else
		HEAT1_OFF;
}

static void ntc2_heat_output(void)
{
	ntc2_heat_struct.heat_cnt++;
	if(ntc2_heat_struct.heat_cnt >= ntc2_heat_struct.heat_pid.pwmcycle)
		ntc2_heat_struct.heat_cnt = 0;

	if(ntc2_heat_struct.heat_cnt < ntc2_heat_struct.heat_pid.OUT)
		HEAT2_ON;
	else
		HEAT2_OFF;
}


void user_tick_hook(void)
{
	if(ntc1_heat_struct.heat_run == SET)		
	{
		ntc1_heat_output();
	}
	else
		HEAT1_OFF;
	
	if(ntc2_heat_struct.heat_run == SET)		
	{
		ntc2_heat_output();
	}
	else
		HEAT2_OFF;
}

static void heat_sftimer_cb(void *para)
{
	adc_filter_t *adc_filter_struct = get_adc_filter_struct();
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();

	if(
		(adc_filter_struct->ntc1_filter_adc < 500) ||	//开路
		(adc_filter_struct->ntc1_filter_adc > 4000) ||	//短路
		(adc_filter_struct->ntc1_filter_adc > 2810)		//限幅
	)
	{
		ntc1_heat_struct.heat_run = RESET;	
		ntc2_heat_struct.heat_run = RESET;
		return;
	}
	else
	{
		if(neck_5pro_dev_struct->ble_pack.heat_level != NONE_HEAT)
			ntc1_heat_struct.heat_run = SET;	
		else
			ntc1_heat_struct.heat_run = RESET;
	}

	if(
		(adc_filter_struct->ntc2_filter_adc < 500) ||	//开路
		(adc_filter_struct->ntc2_filter_adc > 4000) ||	//短路
		(adc_filter_struct->ntc2_filter_adc > 2810)		//限幅
	)
	{
		ntc1_heat_struct.heat_run = RESET;	
		ntc2_heat_struct.heat_run = RESET;
		return;
	}
	else
	{
		if(neck_5pro_dev_struct->ble_pack.heat_level != NONE_HEAT)
			ntc2_heat_struct.heat_run = SET;	
		else
			ntc2_heat_struct.heat_run = RESET;
	}

	if(ntc1_heat_struct.heat_run == SET)
	{
		ntc1_heat_struct.heat_pid.Pv = adc_filter_struct->ntc1_filter_adc;
		pid_calc(&ntc1_heat_struct.heat_pid);
	}

	if(ntc2_heat_struct.heat_run == SET)
	{
		ntc2_heat_struct.heat_pid.Pv = adc_filter_struct->ntc2_filter_adc;
		pid_calc(&ntc2_heat_struct.heat_pid);
	}
}

void heat_sftimer_init(void)
{
	heat_sftimer = rt_timer_create("heat_sftimer", heat_sftimer_cb, RT_NULL, 200, RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_PERIODIC);
//	if(heat_sftimer != RT_NULL)
//		rt_timer_start(heat_sftimer);
}
void get_ntc(void)
{
	LOG_I("ntc1_Sv:%d,ntc2_Sv:%d",(uint16_t)(ntc1_heat_struct.heat_pid.Sv), (uint16_t)(ntc2_heat_struct.heat_pid.Sv));
	LOG_I("ntc1_run:%d,ntc2_run:%d", ntc1_heat_struct.heat_run, ntc2_heat_struct.heat_run);
}
MSH_CMD_EXPORT(get_ntc, get ntc);


static void heat_init(void)
{
	ntc1_heat_struct.heat_pid.Sv = ntc2_heat_struct.heat_pid.Sv = gc_heat_target[1];

	//ntc1 pid
	ntc1_heat_struct.heat_pid.Kp = 2.4;
	ntc1_heat_struct.heat_pid.Ti = 30000;
	ntc1_heat_struct.heat_pid.Td = 120;

	//ntc2 pid
	ntc2_heat_struct.heat_pid.Kp = 2.4;
	ntc2_heat_struct.heat_pid.Ti = 30000;
	ntc2_heat_struct.heat_pid.Td = 120;

	ntc1_heat_struct.heat_pid.T = ntc2_heat_struct.heat_pid.T = 200;
	ntc1_heat_struct.heat_pid.pwmcycle = ntc2_heat_struct.heat_pid.pwmcycle = 200;
	ntc1_heat_struct.heat_pid.OUT0 = ntc2_heat_struct.heat_pid.OUT0 = 1;

	ntc1_heat_struct.heat_cnt = ntc2_heat_struct.heat_cnt = 0;
}
user_initcall(heat_init);

static void heat_target_topic_cb(void *msg)
{
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();

	ntc1_heat_struct.heat_pid.Sv = ntc2_heat_struct.heat_pid.Sv = gc_heat_target[neck_5pro_dev_struct->ble_pack.heat_level];
}

static void heat_dev_no_work_cb(void *msg)
{
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();

	ntc1_heat_struct.heat_run = RESET;	
	ntc2_heat_struct.heat_run = RESET;
	neck_5pro_dev_struct->ble_pack.heat_level = NONE_HEAT;
	if(heat_sftimer != RT_NULL)		rt_timer_stop(heat_sftimer);
}

static void heat_dev_work_cb(void *msg)
{
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();

	neck_5pro_dev_struct->ble_pack.heat_level = WARM_HEAT;
	ntc1_heat_struct.heat_pid.Sv = ntc2_heat_struct.heat_pid.Sv = gc_heat_target[neck_5pro_dev_struct->ble_pack.heat_level];
	if(heat_sftimer != RT_NULL)		rt_timer_start(heat_sftimer);
}

static void heat_about_topic_subscrib(void)
{
	Topic_Subscrib(DEV_HEAT_SW, heat_target_topic_cb);
	Topic_Subscrib(DEV_NO_WORK, heat_dev_no_work_cb);
	Topic_Subscrib(DEV_WORK, heat_dev_work_cb);
}
user_initcall(heat_about_topic_subscrib);




