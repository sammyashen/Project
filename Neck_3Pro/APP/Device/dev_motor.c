 #include "dev_motor.h"

 static task_t motor_task;
 static task_t motor_fsm_task;

 const static uint16_t motor_tra_pwm[2] = {0, 8000};
 const static uint16_t motor_pwm[4] = {0, 6000, 7000, 8000};
 static motor_t motor1, motor2, motor3;

 //电机1状态机及其状态定义
 static struct state_machine motor_1_fsm;
 static struct state motor_1_idle, motor_1_reverse, motor_1_forward, motor_1_stop, motor_1_error;
 static struct state motor_1f_idle, motor_1f_decrease, motor_1f_dir, motor_1f_increase;
 static struct state motor_1r_idle, motor_1r_decrease, motor_1r_dir, motor_1r_increase;
 static struct state motor_1s_idle, motor_1s_decrease, motor_1s_stop;
 //电机2状态机及其状态定义
 static struct state_machine motor_2_fsm;
 static struct state motor_2_idle, motor_2_reverse, motor_2_forward, motor_2_stop, motor_2_error;
 static struct state motor_2f_idle, motor_2f_decrease, motor_2f_dir, motor_2f_increase;
 static struct state motor_2r_idle, motor_2r_decrease, motor_2r_dir, motor_2r_increase;
 static struct state motor_2s_idle, motor_2s_decrease, motor_2s_stop;
 //电机3状态机及其状态定义
 static struct state_machine motor_3_fsm;
 static struct state motor_3_idle, motor_3_reverse, motor_3_forward, motor_3_stop, motor_3_error;
 static struct state motor_3f_idle, motor_3f_decrease, motor_3f_dir, motor_3f_increase;
 static struct state motor_3r_idle, motor_3r_decrease, motor_3r_dir, motor_3r_increase;
 static struct state motor_3s_idle, motor_3s_decrease, motor_3s_stop;

 static bool motor_evt_compare(void *key, struct event *event)
 {
	 if(event->type != MOTOR_EVT)	return false;

	 return (key == event->data);
 }
/***********************************************电机1*******************************************************************/
 static struct state motor_1_idle = {	
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'f', motor_evt_compare, NULL, &motor_1_forward},	
		 {MOTOR_EVT, (void *)'r', motor_evt_compare, NULL, &motor_1_reverse},
	 },
	 .transition_nums = 2,
 };

 static struct state motor_1_forward = {
 	 .state_entry = &motor_1f_idle,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'r', motor_evt_compare, NULL, &motor_1_reverse},	
		 {MOTOR_EVT, (void *)'s', motor_evt_compare, NULL, &motor_1_stop},
	 },
	 .transition_nums = 2,
 }; 

 static struct state motor_1f_idle = {		
 	 .state_parent = &motor_1_forward,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'h', motor_evt_compare, NULL, &motor_1f_decrease},	
	 },
	 .transition_nums = 1,
 }; 

 static void motor_1f_decrease_process(void *oldstate_data, struct event *event, void *state_new_data)
 {
	 if(motor1.pwm_curr >= 800)	motor1.pwm_curr -= 800;
 	 else{
 	 	motor1.pwm_curr = 0;
 	 	tiny_set_event(&motor_fsm_task, EVT_3);
 	 }
 	 bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_8, GPIO_AF2_TIM1, TIM1, TIM_CH_1, 10000, motor1.pwm_curr);
 }
 static struct state motor_1f_decrease = {		
 	 .state_parent = &motor_1_forward,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'h', motor_evt_compare, &motor_1f_decrease_process, &motor_1f_decrease},	
		 {MOTOR_EVT, (void *)'c', motor_evt_compare, NULL, &motor_1f_dir},
	 },
	 .transition_nums = 2,
 };  

 static void motor_1f_dir_process(void *oldstate_data, struct event *event, void *state_new_data)
 {
 	 MOTOR_1_FORWARD;
 	 tiny_set_event(&motor_fsm_task, EVT_3);
 }
 static struct state motor_1f_dir = {		
 	 .state_parent = &motor_1_forward,
	 .transitions = (struct transition[]){
	 	 {MOTOR_EVT, (void *)'h', motor_evt_compare, &motor_1f_dir_process, &motor_1f_dir},	
		 {MOTOR_EVT, (void *)'c', motor_evt_compare, NULL, &motor_1f_increase},
	 },
	 .transition_nums = 2,
 }; 

 static void motor_1f_increase_process(void *oldstate_data, struct event *event, void *state_new_data)
 {
 	 motor1.pwm_target = motor_pwm[iNeck_3Pro.mass_motor1_speed];
 	 //梯形控速
 	 if(motor1.pwm_curr < motor1.pwm_target){
 	 	 if((motor1.pwm_target - motor1.pwm_curr) > 800)	motor1.pwm_curr += 800;
 	 	 else												motor1.pwm_curr = motor1.pwm_target;
 	 }
 	 else if(motor1.pwm_curr > motor1.pwm_target){
 	 	 if((motor1.pwm_curr - motor1.pwm_target) > 800)	motor1.pwm_curr -= 800;
 	 	 else												motor1.pwm_curr = motor1.pwm_target;
 	 }
 	 bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_8, GPIO_AF2_TIM1, TIM1, TIM_CH_1, 10000, motor1.pwm_curr);
 }
 static struct state motor_1f_increase = {		
 	 .state_parent = &motor_1_forward,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'h', motor_evt_compare, &motor_1f_increase_process, &motor_1f_increase},	
	 },
	 .transition_nums = 1,
 };   

 static struct state motor_1_reverse = {
 	 .state_entry = &motor_1r_idle,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'f', motor_evt_compare, NULL, &motor_1_forward},	
		 {MOTOR_EVT, (void *)'s', motor_evt_compare, NULL, &motor_1_stop},
	 },
	 .transition_nums = 2,
 };  

 static struct state motor_1r_idle = {		
 	 .state_parent = &motor_1_reverse,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'h', motor_evt_compare, NULL, &motor_1r_decrease},	
	 },
	 .transition_nums = 1,
 }; 

 static void motor_1r_decrease_process(void *oldstate_data, struct event *event, void *state_new_data)
 {
 	 if(motor1.pwm_curr >= 800)	motor1.pwm_curr -= 800;
 	 else{
 	 	motor1.pwm_curr = 0;
 	 	tiny_set_event(&motor_fsm_task, EVT_3);
 	 }
 	 bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_8, GPIO_AF2_TIM1, TIM1, TIM_CH_1, 10000, motor1.pwm_curr);
 }
 static struct state motor_1r_decrease = {		
 	 .state_parent = &motor_1_reverse,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'h', motor_evt_compare, &motor_1r_decrease_process, &motor_1r_decrease},	
		 {MOTOR_EVT, (void *)'c', motor_evt_compare, NULL, &motor_1r_dir},
	 },
	 .transition_nums = 2,
 };  

 static void motor_1r_dir_process(void *oldstate_data, struct event *event, void *state_new_data)
 {
 	 MOTOR_1_REVERSE;
 	 tiny_set_event(&motor_fsm_task, EVT_3);
 }
 static struct state motor_1r_dir = {		
 	 .state_parent = &motor_1_reverse,
	 .transitions = (struct transition[]){
	 	 {MOTOR_EVT, (void *)'h', motor_evt_compare, &motor_1r_dir_process, &motor_1r_dir},	
		 {MOTOR_EVT, (void *)'c', motor_evt_compare, NULL, &motor_1r_increase},
	 },
	 .transition_nums = 2,
 }; 

 static void motor_1r_increase_process(void *oldstate_data, struct event *event, void *state_new_data)
 {
 	 motor1.pwm_target = motor_pwm[iNeck_3Pro.mass_motor1_speed];
 	 if(motor1.pwm_curr < motor1.pwm_target){
 	 	 if((motor1.pwm_target - motor1.pwm_curr) > 800)	motor1.pwm_curr += 800;
 	 	 else												motor1.pwm_curr = motor1.pwm_target;
 	 }
 	 else if(motor1.pwm_curr > motor1.pwm_target){
 	 	 if((motor1.pwm_curr - motor1.pwm_target) > 800)	motor1.pwm_curr -= 800;
 	 	 else												motor1.pwm_curr = motor1.pwm_target;
 	 }
 	 bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_8, GPIO_AF2_TIM1, TIM1, TIM_CH_1, 10000, motor1.pwm_curr);
 }
 static struct state motor_1r_increase = {		
 	 .state_parent = &motor_1_reverse,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'h', motor_evt_compare, &motor_1r_increase_process, &motor_1r_increase},	
	 },
	 .transition_nums = 1,
 };  

 static struct state motor_1_stop = {
 	 .state_entry = &motor_1s_idle,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'f', motor_evt_compare, NULL, &motor_1_forward},	
		 {MOTOR_EVT, (void *)'r', motor_evt_compare, NULL, &motor_1_reverse},
	 },
	 .transition_nums = 2,
 }; 

 static struct state motor_1s_idle = {		
 	 .state_parent = &motor_1_stop,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'h', motor_evt_compare, NULL, &motor_1s_decrease},	
	 },
	 .transition_nums = 1,
 };  

 static void motor_1s_decrease_process(void *oldstate_data, struct event *event, void *state_new_data)
 {
 	 if(motor1.pwm_curr >= 800)	motor1.pwm_curr -= 800;
 	 else{
 	 	motor1.pwm_curr = 0;
 	 	tiny_set_event(&motor_fsm_task, EVT_3);
 	 }
 	 bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_8, GPIO_AF2_TIM1, TIM1, TIM_CH_1, 10000, motor1.pwm_curr);
 }
 static struct state motor_1s_decrease = {		
 	 .state_parent = &motor_1_stop,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'h', motor_evt_compare, &motor_1s_decrease_process, &motor_1s_decrease},	
		 {MOTOR_EVT, (void *)'c', motor_evt_compare, NULL, &motor_1s_stop},
	 },
	 .transition_nums = 2,
 };  

 static struct state motor_1s_stop = {		
 	 .state_parent = &motor_1_stop,
 };
/***********************************************************************************************************************/

/***********************************************电机2*******************************************************************/
 static struct state motor_2_idle = {	
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'f', motor_evt_compare, NULL, &motor_2_forward},	
		 {MOTOR_EVT, (void *)'r', motor_evt_compare, NULL, &motor_2_reverse},
	 },
	 .transition_nums = 2,
 };

 static struct state motor_2_forward = {
 	 .state_entry = &motor_2f_idle,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'r', motor_evt_compare, NULL, &motor_2_reverse},	
		 {MOTOR_EVT, (void *)'s', motor_evt_compare, NULL, &motor_2_stop},
	 },
	 .transition_nums = 2,
 }; 

 static struct state motor_2f_idle = {		
 	 .state_parent = &motor_2_forward,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'h', motor_evt_compare, NULL, &motor_2f_decrease},	
	 },
	 .transition_nums = 1,
 }; 

 static void motor_2f_decrease_process(void *oldstate_data, struct event *event, void *state_new_data)
 {
 	 if(motor2.pwm_curr >= 800)	motor2.pwm_curr -= 800;
 	 else{
 	 	motor2.pwm_curr = 0;
 	 	tiny_set_event(&motor_fsm_task, EVT_8);
 	 }
 	 bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_9, GPIO_AF2_TIM1, TIM1, TIM_CH_2, 10000, motor2.pwm_curr);
 }
 static struct state motor_2f_decrease = {		
 	 .state_parent = &motor_2_forward,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'h', motor_evt_compare, &motor_2f_decrease_process, &motor_2f_decrease},	
		 {MOTOR_EVT, (void *)'c', motor_evt_compare, NULL, &motor_2f_dir},
	 },
	 .transition_nums = 2,
 };  

 static void motor_2f_dir_process(void *oldstate_data, struct event *event, void *state_new_data)
 {
 	 MOTOR_2_FORWARD;
 	 tiny_set_event(&motor_fsm_task, EVT_8);
 }
 static struct state motor_2f_dir = {		
 	 .state_parent = &motor_2_forward,
	 .transitions = (struct transition[]){
	 	 {MOTOR_EVT, (void *)'h', motor_evt_compare, &motor_2f_dir_process, &motor_2f_dir},	
		 {MOTOR_EVT, (void *)'c', motor_evt_compare, NULL, &motor_2f_increase},
	 },
	 .transition_nums = 2,
 }; 

 static void motor_2f_increase_process(void *oldstate_data, struct event *event, void *state_new_data)
 {
 	 motor2.pwm_target = motor_pwm[iNeck_3Pro.mass_motor2_speed];
 	 if(motor2.pwm_curr < motor2.pwm_target){
 	 	 if((motor2.pwm_target - motor2.pwm_curr) > 800)	motor2.pwm_curr += 800;
 	 	 else												motor2.pwm_curr = motor2.pwm_target;
 	 }
 	 else if(motor2.pwm_curr > motor2.pwm_target){
 	 	 if((motor2.pwm_curr - motor2.pwm_target) > 800)	motor2.pwm_curr -= 800;
 	 	 else												motor2.pwm_curr = motor2.pwm_target;
 	 }
 	 bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_9, GPIO_AF2_TIM1, TIM1, TIM_CH_2, 10000, motor2.pwm_curr);
 }
 static struct state motor_2f_increase = {		
 	 .state_parent = &motor_2_forward,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'h', motor_evt_compare, &motor_2f_increase_process, &motor_2f_increase},	
	 },
	 .transition_nums = 1,
 };   

 static struct state motor_2_reverse = {
 	 .state_entry = &motor_2r_idle,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'f', motor_evt_compare, NULL, &motor_2_forward},	
		 {MOTOR_EVT, (void *)'s', motor_evt_compare, NULL, &motor_2_stop},
	 },
	 .transition_nums = 2,
 };  

 static struct state motor_2r_idle = {		
 	 .state_parent = &motor_2_reverse,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'h', motor_evt_compare, NULL, &motor_2r_decrease},	
	 },
	 .transition_nums = 1,
 }; 

 static void motor_2r_decrease_process(void *oldstate_data, struct event *event, void *state_new_data)
 {
 	 if(motor2.pwm_curr >= 800)	motor2.pwm_curr -= 800;
 	 else{
 	 	motor2.pwm_curr = 0;
 	 	tiny_set_event(&motor_fsm_task, EVT_8);
 	 }
 	 bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_9, GPIO_AF2_TIM1, TIM1, TIM_CH_2, 10000, motor2.pwm_curr);
 }
 static struct state motor_2r_decrease = {		
 	 .state_parent = &motor_2_reverse,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'h', motor_evt_compare, &motor_2r_decrease_process, &motor_2r_decrease},	
		 {MOTOR_EVT, (void *)'c', motor_evt_compare, NULL, &motor_2r_dir},
	 },
	 .transition_nums = 2,
 };  

 static void motor_2r_dir_process(void *oldstate_data, struct event *event, void *state_new_data)
 {
 	 MOTOR_2_REVERSE;
 	 tiny_set_event(&motor_fsm_task, EVT_8);
 }
 static struct state motor_2r_dir = {		
 	 .state_parent = &motor_2_reverse,
	 .transitions = (struct transition[]){
	 	 {MOTOR_EVT, (void *)'h', motor_evt_compare, &motor_2r_dir_process, &motor_2r_dir},	
		 {MOTOR_EVT, (void *)'c', motor_evt_compare, NULL, &motor_2r_increase},
	 },
	 .transition_nums = 2,
 }; 

 static void motor_2r_increase_process(void *oldstate_data, struct event *event, void *state_new_data)
 {
 	 motor2.pwm_target = motor_pwm[iNeck_3Pro.mass_motor2_speed];
 	 if(motor2.pwm_curr < motor2.pwm_target){
 	 	 if((motor2.pwm_target - motor2.pwm_curr) > 800)	motor2.pwm_curr += 800;
 	 	 else												motor2.pwm_curr = motor2.pwm_target;
 	 }
 	 else if(motor2.pwm_curr > motor2.pwm_target){
 	 	 if((motor2.pwm_curr - motor2.pwm_target) > 800)	motor2.pwm_curr -= 800;
 	 	 else												motor2.pwm_curr = motor2.pwm_target;
 	 }
 	 bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_9, GPIO_AF2_TIM1, TIM1, TIM_CH_2, 10000, motor2.pwm_curr);
 }
 static struct state motor_2r_increase = {		
 	 .state_parent = &motor_2_reverse,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'h', motor_evt_compare, &motor_2r_increase_process, &motor_2r_increase},	
	 },
	 .transition_nums = 1,
 };  

 static struct state motor_2_stop = {
 	 .state_entry = &motor_2s_idle,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'f', motor_evt_compare, NULL, &motor_2_forward},	
		 {MOTOR_EVT, (void *)'r', motor_evt_compare, NULL, &motor_2_reverse},
	 },
	 .transition_nums = 2,
 }; 

 static struct state motor_2s_idle = {		
 	 .state_parent = &motor_2_stop,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'h', motor_evt_compare, NULL, &motor_2s_decrease},	
	 },
	 .transition_nums = 1,
 };  

 static void motor_2s_decrease_process(void *oldstate_data, struct event *event, void *state_new_data)
 {
 	 if(motor2.pwm_curr >= 800)	motor2.pwm_curr -= 800;
 	 else{
 	 	motor2.pwm_curr = 0;
 	 	tiny_set_event(&motor_fsm_task, EVT_8);
 	 }
 	 bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_9, GPIO_AF2_TIM1, TIM1, TIM_CH_2, 10000, motor2.pwm_curr);
 }
 static struct state motor_2s_decrease = {		
 	 .state_parent = &motor_2_stop,
	 .transitions = (struct transition[]){
		 {MOTOR_EVT, (void *)'h', motor_evt_compare, &motor_2s_decrease_process, &motor_2s_decrease},	
		 {MOTOR_EVT, (void *)'c', motor_evt_compare, NULL, &motor_2s_stop},
	 },
	 .transition_nums = 2,
 };  

 static struct state motor_2s_stop = {		
 	 .state_parent = &motor_2_stop,
 }; 
/***********************************************************************************************************************/

/***********************************************电机3*******************************************************************/
static struct state motor_3_idle = {   
	.transitions = (struct transition[]){
		{MOTOR_EVT, (void *)'f', motor_evt_compare, NULL, &motor_3_forward}, 
		{MOTOR_EVT, (void *)'r', motor_evt_compare, NULL, &motor_3_reverse},
	},
	.transition_nums = 2,
};

static struct state motor_3_forward = {
	.state_entry = &motor_3f_idle,
	.transitions = (struct transition[]){
		{MOTOR_EVT, (void *)'r', motor_evt_compare, NULL, &motor_3_reverse}, 
		{MOTOR_EVT, (void *)'s', motor_evt_compare, NULL, &motor_3_stop},
	},
	.transition_nums = 2,
}; 

static struct state motor_3f_idle = {	   
	.state_parent = &motor_3_forward,
	.transitions = (struct transition[]){
		{MOTOR_EVT, (void *)'h', motor_evt_compare, NULL, &motor_3f_decrease},   
	},
	.transition_nums = 1,
}; 

static void motor_3f_decrease_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	if(motor3.pwm_curr >= 1000)	motor3.pwm_curr -= 1000;
 	else{
 	 	motor3.pwm_curr = 0;
 	 	tiny_set_event(&motor_fsm_task, EVT_13);
 	}
 	bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_10, GPIO_AF2_TIM1, TIM1, TIM_CH_3, 10000, motor3.pwm_curr);
}
static struct state motor_3f_decrease = {	   
	.state_parent = &motor_3_forward,
	.transitions = (struct transition[]){
		{MOTOR_EVT, (void *)'h', motor_evt_compare, &motor_3f_decrease_process, &motor_3f_decrease}, 
		{MOTOR_EVT, (void *)'c', motor_evt_compare, NULL, &motor_3f_dir},
	},
	.transition_nums = 2,
};	

static void motor_3f_dir_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	MOTOR_3_FORWARD;
	tiny_set_event(&motor_fsm_task, EVT_13);
}
static struct state motor_3f_dir = {	   
	.state_parent = &motor_3_forward,
	.transitions = (struct transition[]){
		{MOTOR_EVT, (void *)'h', motor_evt_compare, &motor_3f_dir_process, &motor_3f_dir},   
		{MOTOR_EVT, (void *)'c', motor_evt_compare, NULL, &motor_3f_increase},
	},
	.transition_nums = 2,
}; 

static void motor_3f_increase_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	motor3.pwm_target = motor_tra_pwm[iNeck_3Pro.mass_motor3_speed];
	if(motor3.pwm_curr < motor3.pwm_target){
		if((motor3.pwm_target - motor3.pwm_curr) > 2000)    motor3.pwm_curr += 2000;
		else											   motor3.pwm_curr = motor3.pwm_target;
	}
	else if(motor3.pwm_curr > motor3.pwm_target){
		if((motor3.pwm_curr - motor3.pwm_target) > 2000)    motor3.pwm_curr -= 2000;
		else											   motor3.pwm_curr = motor3.pwm_target;
	}
 	bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_10, GPIO_AF2_TIM1, TIM1, TIM_CH_3, 10000, motor3.pwm_curr);
}
static struct state motor_3f_increase = {	   
	.state_parent = &motor_3_forward,
	.transitions = (struct transition[]){
		{MOTOR_EVT, (void *)'h', motor_evt_compare, &motor_3f_increase_process, &motor_3f_increase}, 
	},
	.transition_nums = 1,
};		

static struct state motor_3_reverse = {
	.state_entry = &motor_3r_idle,
	.transitions = (struct transition[]){
		{MOTOR_EVT, (void *)'f', motor_evt_compare, NULL, &motor_3_forward}, 
		{MOTOR_EVT, (void *)'s', motor_evt_compare, NULL, &motor_3_stop},
	},
	.transition_nums = 2,
};	

static struct state motor_3r_idle = {	   
	.state_parent = &motor_3_reverse,
	.transitions = (struct transition[]){
		{MOTOR_EVT, (void *)'h', motor_evt_compare, NULL, &motor_3r_decrease},   
	},
	.transition_nums = 1,
}; 

static void motor_3r_decrease_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	if(motor3.pwm_curr >= 1000)	motor3.pwm_curr -= 1000;
 	else{
 	 	motor3.pwm_curr = 0;
 	 	tiny_set_event(&motor_fsm_task, EVT_13);
 	}
 	bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_10, GPIO_AF2_TIM1, TIM1, TIM_CH_3, 10000, motor3.pwm_curr);
}
static struct state motor_3r_decrease = {	   
	.state_parent = &motor_3_reverse,
	.transitions = (struct transition[]){
		{MOTOR_EVT, (void *)'h', motor_evt_compare, &motor_3r_decrease_process, &motor_3r_decrease}, 
		{MOTOR_EVT, (void *)'c', motor_evt_compare, NULL, &motor_3r_dir},
	},
	.transition_nums = 2,
};	

static void motor_3r_dir_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	MOTOR_3_REVERSE;
	tiny_set_event(&motor_fsm_task, EVT_13);
}
static struct state motor_3r_dir = {	   
	.state_parent = &motor_3_reverse,
	.transitions = (struct transition[]){
		{MOTOR_EVT, (void *)'h', motor_evt_compare, &motor_3r_dir_process, &motor_3r_dir},   
		{MOTOR_EVT, (void *)'c', motor_evt_compare, NULL, &motor_3r_increase},
	},
	.transition_nums = 2,
}; 

static void motor_3r_increase_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	motor3.pwm_target = motor_tra_pwm[iNeck_3Pro.mass_motor3_speed];
	if(motor3.pwm_curr < motor3.pwm_target){
		if((motor3.pwm_target - motor3.pwm_curr) > 2000)    motor3.pwm_curr += 2000;
		else											   motor3.pwm_curr = motor3.pwm_target;
	}
	else if(motor3.pwm_curr > motor3.pwm_target){
		if((motor3.pwm_curr - motor3.pwm_target) > 2000)    motor3.pwm_curr -= 2000;
		else											   motor3.pwm_curr = motor3.pwm_target;
	}
 	bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_10, GPIO_AF2_TIM1, TIM1, TIM_CH_3, 10000, motor3.pwm_curr);
}
static struct state motor_3r_increase = {	   
	.state_parent = &motor_3_reverse,
	.transitions = (struct transition[]){
		{MOTOR_EVT, (void *)'h', motor_evt_compare, &motor_3r_increase_process, &motor_3r_increase}, 
	},
	.transition_nums = 1,
};	

static struct state motor_3_stop = {
	.state_entry = &motor_3s_idle,
	.transitions = (struct transition[]){
		{MOTOR_EVT, (void *)'f', motor_evt_compare, NULL, &motor_3_forward}, 
		{MOTOR_EVT, (void *)'r', motor_evt_compare, NULL, &motor_3_reverse},
	},
	.transition_nums = 2,
}; 

static struct state motor_3s_idle = {	   
	.state_parent = &motor_3_stop,
	.transitions = (struct transition[]){
		{MOTOR_EVT, (void *)'h', motor_evt_compare, NULL, &motor_3s_decrease},   
	},
	.transition_nums = 1,
};	

static void motor_3s_decrease_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	if(motor3.pwm_curr >= 1000)	motor3.pwm_curr -= 1000;
 	else{
 	 	motor3.pwm_curr = 0;
 	 	tiny_set_event(&motor_fsm_task, EVT_13);
 	}
 	bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_10, GPIO_AF2_TIM1, TIM1, TIM_CH_3, 10000, motor3.pwm_curr);
}
static struct state motor_3s_decrease = {	   
	.state_parent = &motor_3_stop,
	.transitions = (struct transition[]){
		{MOTOR_EVT, (void *)'h', motor_evt_compare, &motor_3s_decrease_process, &motor_3s_decrease}, 
		{MOTOR_EVT, (void *)'c', motor_evt_compare, NULL, &motor_3s_stop},
	},
	.transition_nums = 2,
};	

static struct state motor_3s_stop = {	   
	.state_parent = &motor_3_stop,
}; 
/***********************************************************************************************************************/

 /*
 *EVT_0：电机1正转
 *EVT_1：电机1停止
 *EVT_2：电机1反转
 *EVT_3：电机2正转
 *EVT_4：电机2停止
 *EVT_5：电机2反转
 *EVT_6：电机3正转
 *EVT_7：电机3停止
 *EVT_8：电机3反转
 */
 static void motor_task_cb(void *para, uint32_t evt)
 {
 	//M1
	if(evt & EVT_0){
		tiny_clr_event(&motor_task, EVT_0);
		tiny_set_event(&motor_fsm_task, EVT_0);
	}
	if(evt & EVT_1){
		tiny_clr_event(&motor_task, EVT_1);
		tiny_set_event(&motor_fsm_task, EVT_2);
	}
	if(evt & EVT_2){
		tiny_clr_event(&motor_task, EVT_2);
		tiny_set_event(&motor_fsm_task, EVT_1);
	}
	tiny_set_event(&motor_fsm_task, EVT_4);
	//M2
	if(evt & EVT_3){
		tiny_clr_event(&motor_task, EVT_3);
		tiny_set_event(&motor_fsm_task, EVT_5);
	}
	if(evt & EVT_4){
		tiny_clr_event(&motor_task, EVT_4);
		tiny_set_event(&motor_fsm_task, EVT_7);
	}
	if(evt & EVT_5){
		tiny_clr_event(&motor_task, EVT_5);
		tiny_set_event(&motor_fsm_task, EVT_6);
	}
	tiny_set_event(&motor_fsm_task, EVT_9);
	//M3
	if(evt & EVT_6){
		tiny_clr_event(&motor_task, EVT_6);
		tiny_set_event(&motor_fsm_task, EVT_10);
	}
	if(evt & EVT_7){
		tiny_clr_event(&motor_task, EVT_7);
		tiny_set_event(&motor_fsm_task, EVT_12);
	}
	if(evt & EVT_8){
		tiny_clr_event(&motor_task, EVT_8);
		tiny_set_event(&motor_fsm_task, EVT_11);
	}
	tiny_set_event(&motor_fsm_task, EVT_14);
 }

 /*
 *EVT_0：M1切换f状态
 *EVT_1：M1切换r状态
 *EVT_2：M1切换s状态
 *EVT_3：M1切换c状态
 *EVT_4：M1切换h状态
 *EVT_5：M2切换f状态
 *EVT_6：M2切换r状态
 *EVT_7：M2切换s状态
 *EVT_8：M2切换c状态
 *EVT_9：M2切换h状态
 *EVT_10：M3切换f状态
 *EVT_11：M3切换r状态
 *EVT_12：M3切换s状态
 *EVT_13：M3切换c状态
 *EVT_14：M3切换h状态
 */
 static void motor_fsm_task_cb(void *para, uint32_t evt)
 {
 	//M1 fsm
 	if(evt & EVT_0){
		tiny_clr_event(&motor_fsm_task, EVT_0);
		statem_handle_event(&motor_1_fsm, &(struct event){MOTOR_EVT, (void *)'f'});
	}
	if(evt & EVT_1){
		tiny_clr_event(&motor_fsm_task, EVT_1);
		statem_handle_event(&motor_1_fsm, &(struct event){MOTOR_EVT, (void *)'r'});
	}
	if(evt & EVT_2){
		tiny_clr_event(&motor_fsm_task, EVT_2);
		statem_handle_event(&motor_1_fsm, &(struct event){MOTOR_EVT, (void *)'s'});
	}
	if(evt & EVT_3){
		tiny_clr_event(&motor_fsm_task, EVT_3);
		statem_handle_event(&motor_1_fsm, &(struct event){MOTOR_EVT, (void *)'c'});
	}
	if(evt & EVT_4){
		tiny_clr_event(&motor_fsm_task, EVT_4);
		statem_handle_event(&motor_1_fsm, &(struct event){MOTOR_EVT, (void *)'h'});
	}
	//M2 fsm
	if(evt & EVT_5){
		tiny_clr_event(&motor_fsm_task, EVT_5);
		statem_handle_event(&motor_2_fsm, &(struct event){MOTOR_EVT, (void *)'f'});
	}
	if(evt & EVT_6){
		tiny_clr_event(&motor_fsm_task, EVT_6);
		statem_handle_event(&motor_2_fsm, &(struct event){MOTOR_EVT, (void *)'r'});
	}
	if(evt & EVT_7){
		tiny_clr_event(&motor_fsm_task, EVT_7);
		statem_handle_event(&motor_2_fsm, &(struct event){MOTOR_EVT, (void *)'s'});
	}
	if(evt & EVT_8){
		tiny_clr_event(&motor_fsm_task, EVT_8);
		statem_handle_event(&motor_2_fsm, &(struct event){MOTOR_EVT, (void *)'c'});
	}
	if(evt & EVT_9){
		tiny_clr_event(&motor_fsm_task, EVT_9);
		statem_handle_event(&motor_2_fsm, &(struct event){MOTOR_EVT, (void *)'h'});
	}
	//M3 fsm
	if(evt & EVT_10){
		tiny_clr_event(&motor_fsm_task, EVT_10);
		statem_handle_event(&motor_3_fsm, &(struct event){MOTOR_EVT, (void *)'f'});
	}
	if(evt & EVT_11){
		tiny_clr_event(&motor_fsm_task, EVT_11);
		statem_handle_event(&motor_3_fsm, &(struct event){MOTOR_EVT, (void *)'r'});
	}
	if(evt & EVT_12){
		tiny_clr_event(&motor_fsm_task, EVT_12);
		statem_handle_event(&motor_3_fsm, &(struct event){MOTOR_EVT, (void *)'s'});
	}
	if(evt & EVT_13){
		tiny_clr_event(&motor_fsm_task, EVT_13);
		statem_handle_event(&motor_3_fsm, &(struct event){MOTOR_EVT, (void *)'c'});
	}
	if(evt & EVT_14){
		tiny_clr_event(&motor_fsm_task, EVT_14);
		statem_handle_event(&motor_3_fsm, &(struct event){MOTOR_EVT, (void *)'h'});
	}
 }

 static void motor_process_cb(void *msg)
 {
	 uint32_t *evt = (uint32_t *)msg;

	 tiny_set_event(&motor_task, *evt);
 }
 
 static void motor_device_init(void)
 {
	 tiny_timer_create(&motor_task, motor_task_cb, NULL);
	 tiny_timer_start(&motor_task, TIMER_FOREVER, 50);
	 tiny_task_create(&motor_fsm_task, motor_fsm_task_cb, NULL);
 
	 Topic_Subscrib(MOTOR_TOPIC, motor_process_cb);

	 statem_init(&motor_1_fsm, &motor_1_idle, &motor_1_error);
	 statem_init(&motor_2_fsm, &motor_2_idle, &motor_2_error);
	 statem_init(&motor_3_fsm, &motor_3_idle, &motor_3_error);
 }
 app_initcall(motor_device_init);


