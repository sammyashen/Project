#include "dev_buzzer.h"

static BEEP_T g_tBeep;
#define BEEP_ENABLE()	bsp_SetTIMOutPWM_N(GPIOA, GPIO_PIN_7, GPIO_AF6_TIM8, TIM8, TIM_CH_1, g_tBeep.uiFreq, 5000)
#define BEEP_DISABLE()	bsp_SetTIMOutPWM_N(GPIOA, GPIO_PIN_7, GPIO_AF6_TIM8, TIM8, TIM_CH_1, 2700, 0)

static task_t buzzer_task;
static task_t buzzer_fsm_task;

static struct state_machine buzzer_fsm;
static struct state buzzer_idle, buzzer_wait, buzzer_error;

static void beep_start(uint32_t _uiFreq, uint16_t _usBeepTime, uint16_t _usStopTime, uint16_t _usCycle)
{
	if (_usBeepTime == 0)
	{
		return;
	}

	g_tBeep.uiFreq = _uiFreq;
	g_tBeep.usBeepTime = _usBeepTime;
	g_tBeep.usStopTime = _usStopTime;
	g_tBeep.usCycle = _usCycle;
	g_tBeep.usCount = 0;
	g_tBeep.usCycleCount = 0;
	g_tBeep.ucState = 0;
	g_tBeep.ucEnalbe = 1;	

	BEEP_ENABLE();	/* 开始发声 */		
}

static void beep_stop(void)
{
	g_tBeep.ucEnalbe = 0;

	BEEP_DISABLE();	/* 必须在清控制标志后再停止发声，避免停止后在中断中又开启 */
}

static void beep_process(void)
{
	if ((g_tBeep.ucEnalbe == 0) || (g_tBeep.usStopTime == 0))
	{
		return;
	}

	if (g_tBeep.ucState == 0)
	{
		if (g_tBeep.usStopTime > 0)	/* 间断发声 */
		{
			if (++g_tBeep.usCount >= g_tBeep.usBeepTime)
			{
				BEEP_DISABLE();		/* 停止发声 */
				g_tBeep.usCount = 0;
				g_tBeep.ucState = 1;
			}
		}
		else
		{
			;	/* 不做任何处理，连续发声 */
		}
	}
	else if (g_tBeep.ucState == 1)
	{
		if (++g_tBeep.usCount >= g_tBeep.usStopTime)
		{
			/* 连续发声时，直到调用stop停止为止 */
			if (g_tBeep.usCycle > 0)
			{
				if (++g_tBeep.usCycleCount >= g_tBeep.usCycle)
				{
					/* 循环次数到，停止发声 */
					g_tBeep.ucEnalbe = 0;
				}

				if (g_tBeep.ucEnalbe == 0)
				{
					g_tBeep.usStopTime = 0;
					return;
				}
			}

			g_tBeep.usCount = 0;
			g_tBeep.ucState = 0;

			BEEP_ENABLE();			/* 开始发声 */
		}
	}
}

static bool buzzer_evt_compare(void *key, struct event *event)
{
	if(event->type != BUZZER_EVT)   		return false;
	if(strcmp(key, event->data) == 0)		return true;
	else									return false;
}

static struct state buzzer_idle = {   
	.transitions = (struct transition[]){
		{BUZZER_EVT, (void *)"wait", buzzer_evt_compare, NULL, &buzzer_wait},   
	},
	.transition_nums = 1,
};

static uint8_t buzzer_wait_cnt = 0;
static void buzzer_wait_entery(void *state_data, struct event *event)
{
	buzzer_wait_cnt = 0;
	iNeck_3Pro.dev_status = STOPPING;
}
static void buzzer_wait_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	if(++buzzer_wait_cnt >= 20){	//wait 1s
		buzzer_wait_cnt = 0;
		tiny_set_event(&buzzer_fsm_task, EVT_2);
	}
}
static struct state buzzer_wait = {
	.transitions = (struct transition[]){
		{BUZZER_EVT, (void *)"hold", buzzer_evt_compare, &buzzer_wait_process, &buzzer_wait},   
		{BUZZER_EVT, (void *)"change", buzzer_evt_compare, NULL, &buzzer_error}, 
	},
	.transition_nums = 2,
	.action_entry = &buzzer_wait_entery,
};

static void buzzer_error_entery(void *state_data, struct event *event)
{
	Topic_Pushlish(MASS_TOPIC, &(uint32_t){EVT_1});//关闭按摩
}
static struct state buzzer_error = {
	.action_entry = &buzzer_error_entery,
};


/*
*EVT_0：短鸣1声事件
*EVT_1：短鸣N声事件
*EVT_2：长鸣事件
*EVT_3：关闭蜂鸣器
*/
static void buzzer_task_cb(void *para, uint32_t evt)
{
	beep_process();
	
	if(evt & EVT_0){
		tiny_clr_event(&buzzer_task, EVT_0);
		beep_start(2700, 4, 4, 1);
	}

	if(evt & EVT_1){
		tiny_clr_event(&buzzer_task, EVT_1);
		switch(iNeck_3Pro.mass_motor1_speed)
		{
			case SLOW_SPEED:
				beep_start(2700, 4, 4, 1);
			break;

			case MID_SPEED:
				beep_start(2700, 4, 4, 2);
			break;

			case FAST_SPEED:
				beep_start(2700, 4, 4, 3);
			break;

			default:
				beep_stop();
			break;
		}
	}

	if(evt & EVT_2){
		tiny_clr_event(&buzzer_task, EVT_2);
		beep_start(2700, 20, 20, 1);
		tiny_set_event(&buzzer_fsm_task, EVT_0);//启动等待长鸣结束
	}

	tiny_set_event(&buzzer_fsm_task, EVT_1);
}

static void buzzer_fsm_task_cb(void *para, uint32_t evt)
{
	if(evt & EVT_0){
		tiny_clr_event(&buzzer_fsm_task, EVT_0);
		statem_handle_event(&buzzer_fsm, &(struct event){BUZZER_EVT, (void *)"wait"});
	}
	if(evt & EVT_1){
		tiny_clr_event(&buzzer_fsm_task, EVT_1);
		statem_handle_event(&buzzer_fsm, &(struct event){BUZZER_EVT, (void *)"hold"});
	}
	if(evt & EVT_2){
		tiny_clr_event(&buzzer_fsm_task, EVT_2);
		statem_handle_event(&buzzer_fsm, &(struct event){BUZZER_EVT, (void *)"change"});
	}
}

static void buzzer_process_cb(void *msg)
{
	uint32_t *evt = (uint32_t *)msg;

	tiny_set_event(&buzzer_task, *evt);
}

static void buzzer_device_init(void)
{
	tiny_timer_create(&buzzer_task, buzzer_task_cb, NULL);
    tiny_timer_start(&buzzer_task, TIMER_FOREVER, 50);
    tiny_task_create(&buzzer_fsm_task, buzzer_fsm_task_cb, NULL);

    Topic_Subscrib(BUZZER_TOPIC, buzzer_process_cb);

    statem_init(&buzzer_fsm, &buzzer_idle, &buzzer_error);
}
app_initcall(buzzer_device_init);


