#include "mass.h"

static task_t mass_task;
static task_t mass_fsm_task;

static struct state_machine mass_fsm;
static struct state mass_idle, mass_start, mass_work, mass_stop, mass_error;
static struct state mass_work_idle, mass_work_normal, mass_work_aging;
static struct state work_normal_idle, work_normal_hold, work_normal_forward, work_normal_reverse;
static struct state work_aging_idle;

static bool mass_evt_compare(void *key, struct event *event)
{
	if(event->type != MASS_EVT)   		return false;
	if(strcmp(key, event->data) == 0)	return true;
	else								return false;
}

static struct state mass_idle = {   
	.transitions = (struct transition[]){
		{MASS_EVT, (void *)"start", mass_evt_compare, NULL, &mass_start},   
	},
	.transition_nums = 1,
};

static void mass_start_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint32_t evt = 0;
	
	if(iNeck_3Pro.mass_motor1_dir == MASS_FORWARD)		evt |= EVT_0;
	else												evt |= EVT_2;
	if(iNeck_3Pro.mass_motor2_dir == MASS_FORWARD)		evt |= EVT_5;
	else												evt |= EVT_3;
	
	//等待M3到达后端
	if(PSW2_DETECT == RESET){
		//停止M3
		tiny_set_event(&mass_fsm_task, EVT_3);
		evt |= EVT_7;
	}
	else{
		//开启M3后移
		iNeck_3Pro.mass_motor3_dir = MASS_REVERSE;
		evt |= EVT_8;
	}
	Topic_Pushlish(MOTOR_TOPIC, &evt);
}
static struct state mass_start = {
	.transitions = (struct transition[]){
		{MASS_EVT, (void *)"hold", mass_evt_compare, &mass_start_process, &mass_start},   
		{MASS_EVT, (void *)"change", mass_evt_compare, NULL, &mass_work}, 
		{MASS_EVT, (void *)"stop", mass_evt_compare, NULL, &mass_stop},
		{MASS_EVT, (void *)"block", mass_evt_compare, NULL, &work_normal_forward},
	},
	.transition_nums = 4,
//	.action_entry = &mass_start_entery,
};

static struct state mass_work = {
	.state_entry = &mass_work_idle,
	.transitions = (struct transition[]){
		{MASS_EVT, (void *)"stop", mass_evt_compare, NULL, &mass_stop},
	},
	.transition_nums = 1,
};

static struct state mass_work_idle = {		
	.state_parent = &mass_work,
	.transitions = (struct transition[]){
		{MASS_EVT, (void *)"hold", mass_evt_compare, NULL, &mass_work_normal},	
	},
	.transition_nums = 1,
};

static struct state mass_work_normal = {		
	.state_parent = &mass_work,
	.state_entry = &work_normal_idle,
	.transitions = (struct transition[]){
		{MASS_EVT, (void *)"free", mass_evt_compare, NULL, &mass_work_aging},	
	},
	.transition_nums = 1,
};

static struct state work_normal_idle = {		
	.state_parent = &mass_work_normal,
	.transitions = (struct transition[]){
		{MASS_EVT, (void *)"hold", mass_evt_compare, NULL, &work_normal_hold},	
	},
	.transition_nums = 1,
};

static uint8_t _sec_cnt,_10s_cnt;
static void work_normal_hold_entery(void *state_data, struct event *event)
{
	_sec_cnt = 0;
	_10s_cnt = 0;
}
static void work_normal_hold_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint32_t evt = 0;
	//关闭M3、开启M1、M2按摩，并启动10s倒计时
	if(iNeck_3Pro.mass_motor1_dir == MASS_FORWARD)		evt |= EVT_0;
	else												evt |= EVT_2;
	if(iNeck_3Pro.mass_motor2_dir == MASS_FORWARD)		evt |= EVT_5;
	else												evt |= EVT_3;
	evt |= EVT_7;
	Topic_Pushlish(MOTOR_TOPIC, &evt);
	//如果处于手动移位下，跳过10s顶点按摩阶段
	if(iNeck_3Pro.is_manual_tra == SET){
		tiny_set_event(&mass_fsm_task, EVT_3);
	}	
	if(++_sec_cnt >= 20){
		_sec_cnt = 0;
		if(++_10s_cnt >= 10){
			_10s_cnt = 0;
			tiny_set_event(&mass_fsm_task, EVT_3);
		}
	}
}
static struct state work_normal_hold = {		
	.state_parent = &mass_work_normal,
	.transitions = (struct transition[]){
		{MASS_EVT, (void *)"hold", mass_evt_compare, &work_normal_hold_process, &work_normal_hold},	
		{MASS_EVT, (void *)"change", mass_evt_compare, NULL, &work_normal_forward},
	},
	.transition_nums = 2,
	.action_entry = &work_normal_hold_entery,
};

static void work_normal_forward_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint32_t evt = 0;
	//根据模式设置M1、M2
	if(iNeck_3Pro.mass_mode == MASS_MODE){
		//若处于手动移位下，M1、M2维持转动
		if(iNeck_3Pro.is_manual_tra == SET){
			if(iNeck_3Pro.mass_motor1_dir == MASS_FORWARD)		evt |= EVT_0;
			else												evt |= EVT_2;
			if(iNeck_3Pro.mass_motor2_dir == MASS_FORWARD)		evt |= EVT_5;
			else												evt |= EVT_3;
		}
		else{
			evt |= (EVT_1 | EVT_4);
		}
	}
	else{ 
		if(iNeck_3Pro.mass_motor1_dir == MASS_FORWARD)		evt |= EVT_0;
		else												evt |= EVT_2;
		if(iNeck_3Pro.mass_motor2_dir == MASS_FORWARD)		evt |= EVT_5;
		else												evt |= EVT_3;
	}
	//等待M3到达前端
	if(PSW1_DETECT == RESET){
		//停止M3
		evt |= EVT_7;
		tiny_set_event(&mass_fsm_task, EVT_3);
	}
	else{
		//启动M3前移
		if(iNeck_3Pro.is_travel == SET){
			iNeck_3Pro.mass_motor3_dir = MASS_FORWARD;
			evt |= EVT_6;
		}else{
			iNeck_3Pro.mass_motor3_dir = MASS_FORWARD;
			evt |= EVT_7;
		}
	}
	Topic_Pushlish(MOTOR_TOPIC, &evt);
}
static struct state work_normal_forward = {		
	.state_parent = &mass_work_normal,
	.transitions = (struct transition[]){
		{MASS_EVT, (void *)"hold", mass_evt_compare, &work_normal_forward_process, &work_normal_forward},	
		{MASS_EVT, (void *)"change", mass_evt_compare, NULL, &work_normal_reverse},
		{MASS_EVT, (void *)"block", mass_evt_compare, NULL, &work_normal_reverse},
	},
	.transition_nums = 3,
};

static void work_normal_reverse_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint32_t evt = 0;
	//根据模式设置M1、M2
	if(iNeck_3Pro.mass_mode == KNEAD_MODE){
		//若处于手动移位下，维持M1、M2按摩
		if(iNeck_3Pro.is_manual_tra == SET){
			if(iNeck_3Pro.mass_motor1_dir == MASS_FORWARD)		evt |= EVT_0;
			else												evt |= EVT_2;
			if(iNeck_3Pro.mass_motor2_dir == MASS_FORWARD)		evt |= EVT_5;
			else												evt |= EVT_3;
		}
		else{
			evt |= (EVT_1 | EVT_4);
		}
	}
	else{ 
		if(iNeck_3Pro.mass_motor1_dir == MASS_FORWARD)		evt |= EVT_0;
		else												evt |= EVT_2;
		if(iNeck_3Pro.mass_motor2_dir == MASS_FORWARD)		evt |= EVT_5;
		else												evt |= EVT_3;
	}
	//等待M3到达后端
	if(PSW2_DETECT == RESET){
		//停止M3
		evt |= EVT_7;
		tiny_set_event(&mass_fsm_task, EVT_3);
	}
	else{
		//启动M3后移
		if(iNeck_3Pro.is_travel == SET){
			iNeck_3Pro.mass_motor3_dir = MASS_REVERSE;
			evt |= EVT_8;
		}else{
			iNeck_3Pro.mass_motor3_dir = MASS_REVERSE;
			evt |= EVT_7;
		}
	}
	Topic_Pushlish(MOTOR_TOPIC, &evt);
}
static struct state work_normal_reverse = {		
	.state_parent = &mass_work_normal,
	.transitions = (struct transition[]){
		{MASS_EVT, (void *)"hold", mass_evt_compare, &work_normal_reverse_process, &work_normal_reverse},	
		{MASS_EVT, (void *)"change", mass_evt_compare, NULL, &work_normal_hold},
		{MASS_EVT, (void *)"block", mass_evt_compare, NULL, &work_normal_forward},
	},
	.transition_nums = 3,
};

static struct state mass_work_aging = {		
	.state_parent = &mass_work,
	.state_entry = &work_aging_idle,
};

static uint8_t _free_sec_cnt;
static uint16_t _aging_time_cnt;
static void work_aging_idle_entery(void *state_data, struct event *event)
{
	Topic_Pushlish(MOTOR_TOPIC, &(uint32_t){EVT_1|EVT_4|EVT_7});
	Topic_Pushlish(HEAT_TOPIC, &(uint32_t){EVT_1});
	_free_sec_cnt = 0;
	_aging_time_cnt = 0;
}
static void work_aging_idle_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	if(++_free_sec_cnt >= 20){
		_free_sec_cnt = 0;
		if(++_aging_time_cnt >= FREE_SEC){
			_aging_time_cnt = 0;
			if(++iNeck_3Pro.aging_cnt < 5)
				tiny_set_event(&mass_fsm_task, EVT_3);
			else{
				iNeck_3Pro.aging_cnt = 5;
				if(iNeck_3Pro.dev_status != AGINGDONE){
					iNeck_3Pro.dev_status = AGINGDONE;
					Topic_Pushlish(LED_TOPIC, &(uint32_t){EVT_3|EVT_7});
//					Topic_Pushlish(BUZZER_TOPIC, &(uint32_t){EVT_2});
				}
			}
		}
	}
}
static void work_aging_idle_exit(void *state_data, struct event *event)
{
	iNeck_3Pro.mass_remain = iNeck_3Pro.mass_time;
	Topic_Pushlish(HEAT_TOPIC, &(uint32_t){EVT_0});
}
static struct state work_aging_idle = {		
	.state_parent = &mass_work_aging,
	.transitions = (struct transition[]){
		{MASS_EVT, (void *)"hold", mass_evt_compare, &work_aging_idle_process, &work_aging_idle},	
		{MASS_EVT, (void *)"change", mass_evt_compare, NULL, &mass_work_normal},
	},
	.transition_nums = 2,
	.action_entry = &work_aging_idle_entery,
	.action_exit  = &work_aging_idle_exit,
};

static void mass_stop_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint32_t evt = 0;
	
	//关闭M1、M2按摩
	evt |= EVT_1;
	evt |= EVT_4;
	
	//等待M3到达前端
	if(PSW1_DETECT == RESET){
		//停止M3
		evt |= EVT_7;
		if(iNeck_3Pro.dev_status != SLEEP){
			if(DC_INPUT_DETECT == SET)		iNeck_3Pro.dev_status = CHARGING;
			else							iNeck_3Pro.dev_status = SLEEP;
//			tiny_set_event(&mass_fsm_task, EVT_3);
			MCU_PWR_DISABLE;
		}
	}
	else{
		//启动M3前移
		iNeck_3Pro.mass_motor3_dir = MASS_FORWARD;
		evt |= EVT_6;
	}
	Topic_Pushlish(MOTOR_TOPIC, &evt);
}
static struct state mass_stop = {
	.transitions = (struct transition[]){
		{MASS_EVT, (void *)"hold", mass_evt_compare, &mass_stop_process, &mass_stop},
		{MASS_EVT, (void *)"change", mass_evt_compare, NULL, &mass_error},
	},
	.transition_nums = 2,
//	.action_entry = &mass_stop_entery,
};

static void mass_error_entery(void *state_data, struct event *event)
{
	MCU_PWR_DISABLE;
}
static struct state mass_error = {
	.action_entry = &mass_error_entery,
};

/*
*EVT_0：按摩开始
*EVT_1: 按摩关闭
*EVT_2: 按摩间歇
*/
static void mass_task_cb(void *para, uint32_t evt)
{
	static uint8_t fliter_cnt[3] = {0};

	if(evt & EVT_0){
		tiny_clr_event(&mass_task, EVT_0);
		tiny_set_event(&mass_fsm_task, EVT_0);
	}
	if(evt & EVT_1){
		tiny_clr_event(&mass_task, EVT_1);
		tiny_set_event(&mass_fsm_task, EVT_1);
	}
	if(evt & EVT_2){
		tiny_clr_event(&mass_task, EVT_2);
		tiny_set_event(&mass_fsm_task, EVT_4);
	}

	//堵转处理
	if(ADC_GetSample(M1VAL_SAMPLE) > 140){
		fliter_cnt[0]++;
		if(fliter_cnt[0] >= 20){
			fliter_cnt[0] = 0;
			iNeck_3Pro.block_cnt++;
			if(iNeck_3Pro.mass_motor1_dir == MASS_FORWARD){
				iNeck_3Pro.mass_motor1_dir = MASS_REVERSE;
			}
			else{
				iNeck_3Pro.mass_motor1_dir = MASS_FORWARD;
			}
		}
	}else{
		fliter_cnt[0] = 0;
	}
	if(ADC_GetSample(M2VAL_SAMPLE) > 140){
		fliter_cnt[1]++;
		if(fliter_cnt[1] >= 20){
			fliter_cnt[1] = 0;
			iNeck_3Pro.block_cnt++;
			if(iNeck_3Pro.mass_motor2_dir == MASS_FORWARD){
				iNeck_3Pro.mass_motor2_dir = MASS_REVERSE;
			}
			else{
				iNeck_3Pro.mass_motor2_dir = MASS_FORWARD;
			}
		}
	}else{
		fliter_cnt[1] = 0;
	}
	if(ADC_GetSample(M3VAL_SAMPLE) > 200){
		fliter_cnt[2]++;
		if(fliter_cnt[2] >= 20){
			fliter_cnt[2] = 0;
			iNeck_3Pro.block_cnt++;
			tiny_set_event(&mass_fsm_task, EVT_5);
		}
	}else{
		fliter_cnt[2] = 0;
	}
	
	tiny_set_event(&mass_fsm_task, EVT_2);
}

/*
*EVT_0：send "start"
*EVT_1：send "stop"
*EVT_2: send "hold"
*EVT_3: send "change"
*EVT_4: send "free"
*EVT_5: send "block"
*/
static void mass_fsm_task_cb(void *para, uint32_t evt)
{
	if(evt & EVT_0){
		tiny_clr_event(&mass_fsm_task, EVT_0);
		statem_handle_event(&mass_fsm, &(struct event){MASS_EVT, (void *)"start"});
	}
	if(evt & EVT_1){
		tiny_clr_event(&mass_fsm_task, EVT_1);
		statem_handle_event(&mass_fsm, &(struct event){MASS_EVT, (void *)"stop"});
	}
	if(evt & EVT_2){
		tiny_clr_event(&mass_fsm_task, EVT_2);
		statem_handle_event(&mass_fsm, &(struct event){MASS_EVT, (void *)"hold"});
	}
	if(evt & EVT_3){
		tiny_clr_event(&mass_fsm_task, EVT_3);
		statem_handle_event(&mass_fsm, &(struct event){MASS_EVT, (void *)"change"});
	}
	if(evt & EVT_4){
		tiny_clr_event(&mass_fsm_task, EVT_4);
		statem_handle_event(&mass_fsm, &(struct event){MASS_EVT, (void *)"free"});
	}
	if(evt & EVT_5){
		tiny_clr_event(&mass_fsm_task, EVT_5);
		statem_handle_event(&mass_fsm, &(struct event){MASS_EVT, (void *)"block"});
	}
}

static void mass_process_cb(void *msg)
{
	uint32_t *evt = (uint32_t *)msg;

	tiny_set_event(&mass_task, *evt);
}

static void mode_change_cb(void *msg)
{
	if(mass_fsm.state_current == (&work_normal_hold)){
		tiny_set_event(&mass_fsm_task, EVT_3);
	}
}

static void mass_mode_init(void)
{
	tiny_timer_create(&mass_task, mass_task_cb, NULL);
	tiny_timer_start(&mass_task, TIMER_FOREVER, 50);
	tiny_task_create(&mass_fsm_task, mass_fsm_task_cb, NULL);

	Topic_Subscrib(MASS_TOPIC, mass_process_cb);
	Topic_Subscrib(CHANGE_TOPIC, mode_change_cb);

	statem_init(&mass_fsm, &mass_idle, &mass_error);
}
app_initcall(mass_mode_init);


