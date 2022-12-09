#include "dev_heat.h"

const uint16_t temperature_adc[1] = {1500};//43℃

static task_t heat_task;
static pid_t heat_pid = {
	.Sv = 2048,
	.Pv = 2048,
	.Kp = 2.6,
	.Ti = 16.5,
	.Td = 0.2,
	.T  = 0.2,
	.OUT0 = 0,
	.pwmcycle = 100,
};

/*
*EVT_0：开启加热
*EVT_1：关闭加热
*/
static void heat_task_cb(void *para, uint32_t evt)
{
	if(evt & EVT_0){
		tiny_clr_event(&heat_task, EVT_0);
		if(iNeck_3Pro.heat_level == HEAT_NONE)		iNeck_3Pro.heat_level = HEAT_WARM;
	}

	if(evt & EVT_1){
		tiny_clr_event(&heat_task, EVT_1);
		if(iNeck_3Pro.heat_level == HEAT_WARM)		iNeck_3Pro.heat_level = HEAT_NONE;
	}

	if(iNeck_3Pro.heat_level == HEAT_WARM){
		heat_pid.Sv = temperature_adc[0];
		heat_pid.Pv = ADC_GetSample(NTC_SAMPLE);
		if(heat_pid.Pv > 3800 || heat_pid.Pv < 400)		iNeck_3Pro.heat_level = HEAT_NONE;
		pid_calc(&heat_pid);
		bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_6, GPIO_AF2_TIM3, TIM3, TIM_CH_1, 5, (heat_pid.OUT * 100));
	}else{
		heat_pid.SEk = 0;
		heat_pid.Pv_1 = 0;
		bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_6, GPIO_AF2_TIM3, TIM3, TIM_CH_1, 5, 0);
	}
}

static void heat_process_cb(void *msg)
{
	uint32_t *evt = (uint32_t *)msg;

	tiny_set_event(&heat_task, *evt);
}

static void heat_device_init(void)
{
	tiny_timer_create(&heat_task, heat_task_cb, NULL);
    tiny_timer_start(&heat_task, TIMER_FOREVER, 200);

    Topic_Subscrib(HEAT_TOPIC, heat_process_cb);
}
app_initcall(heat_device_init);


