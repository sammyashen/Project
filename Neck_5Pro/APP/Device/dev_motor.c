#include "dev_motor.h"
#include "topic.h"
#include "include.h"
#include "drv_gpio.h"

const uint16_t gc_mass_strength[4] = {0, 5000, 6000, 7000};
const uint16_t gc_tra_speed[4] = {0, 5000, 6000, 7000};

rt_timer_t mass_sftimer = RT_NULL, dir_sftimer = RT_NULL, strength_sftimer = RT_NULL;
rt_timer_t tra_sftimer = RT_NULL, block_sftimer = RT_NULL;
rt_timer_t work_sftimer = RT_NULL;

dev_motor_t dev_motor_struct; 

void get_motor_struct(void)
{
	LOG_I("mass_block:%d", dev_motor_struct.mass_block);
	LOG_I("mass_pwm:%d", dev_motor_struct.mass_speed_pwm);
	LOG_I("mass_target:%d", dev_motor_struct.mass_speed_target);
}
MSH_CMD_EXPORT(get_motor_struct, get_motor_struct);


static void mass_sftimer_cb(void *para)
{
	if(dev_motor_struct.mode_opt == STOP_MASS)
	{
		dev_motor_struct.mode_opt = CHECK_PLACE;
		if(mass_sftimer != RT_NULL)			rt_timer_stop(mass_sftimer);
	}
}

static void dir_sftimer_cb(void *para)
{
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();

	switch(neck_5pro_dev_struct->ble_pack.mass_mode)
	{
		case SOFT_MODE:
		case MID_MODE:
		case HARD_MODE:
			if(neck_5pro_dev_struct->ble_pack.mass_dir == MASS_FORWARD)
				neck_5pro_dev_struct->ble_pack.mass_dir = MASS_REVERSE;
			else
				neck_5pro_dev_struct->ble_pack.mass_dir = MASS_FORWARD;
		break;
	}
}

static void strength_sftimer_cb(void *para)
{
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();

	switch(neck_5pro_dev_struct->ble_pack.mass_mode)
	{
		case MID_MODE:
			if(neck_5pro_dev_struct->ble_pack.mass_strength == SOFT_STRENGTH)
				neck_5pro_dev_struct->ble_pack.mass_strength = MID_STRENGTH;
			else
				neck_5pro_dev_struct->ble_pack.mass_strength = SOFT_STRENGTH;
		break;
		
		case HARD_MODE:
			
		break;
	}
}

//移动至中间位置定时器回调函数
static void tra_sftimer_cb(void *para)
{
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();

	if(dev_motor_struct.mode_optnd == STOP_TARGET_PLACE)
	{
		if(tra_sftimer != RT_NULL)			rt_timer_stop(tra_sftimer);
		dev_motor_struct.mode_opt = START_MASS;
		dev_motor_struct.mode_optnd = GO_VRF_PLACE;
		//停止移动，设置按摩力度
		neck_5pro_dev_struct->dev_global.tra_speed = NONE_SPEED;
		neck_5pro_dev_struct->ble_pack.curr_place = MIDDLE;
		if(neck_5pro_dev_struct->ble_pack.mass_mode == SOFT_MODE)
		{
			neck_5pro_dev_struct->ble_pack.mass_strength = MID_STRENGTH;
			Topic_Pushlish(UPDATE_HEART_PACK, neck_5pro_dev_struct);
		}
	}
}

static void work_sftimer_cb(void *para)
{
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();
	static uint8_t aging_times = 0;
	static uint8_t block_timeout = 0;

	//堵转超时计数
	if(dev_motor_struct.mass_block != 0)
	{
		block_timeout++;
		if(block_timeout >= 10)
		{
			block_timeout = 0;
			if(dev_motor_struct.mass_block < 3)
				dev_motor_struct.mass_block = 0;
		}
	}

	neck_5pro_dev_struct->dev_global.mass_cnt++;
	if(neck_5pro_dev_struct->dev_global.mass_cnt >= neck_5pro_dev_struct->ble_pack.mass_time)
	{
		neck_5pro_dev_struct->dev_global.mass_cnt = 0;
		if(neck_5pro_dev_struct->dev_status == DEV_AGING)	//老化
		{
			if(neck_5pro_dev_struct->ble_pack.mass_strength != NONE_STRENGTH)
			{
				aging_times++;
//				if(aging_times >= 5)
//				{
//					aging_times = 0;
//					//关闭电机线程
//					if(rt_thread_find("motor") != RT_NULL)
//						rt_thread_delete(rt_thread_find("motor"));
//					while(TRA_SW2_DETECT != RESET)
//					{
//						MOTOR_TRA_REVERSE;
//						bsp_SetTIMOutPWM(GPIOB, GPIO_PIN_0, TIM3, 3, 20000, 6000);
//
//						rt_thread_mdelay(10);
//					}
//					neck_5pro_dev_struct->dev_global.is_dev_on = RESET;
//					neck_5pro_dev_struct->dev_global.is_dev_work = RESET;
//					Topic_Pushlish(DEV_NO_WORK, neck_5pro_dev_struct);
//					Topic_Pushlish(BUZZER_YOWL, neck_5pro_dev_struct);
//				}
//				else
//				{
//					neck_5pro_dev_struct->ble_pack.mass_strength = NONE_STRENGTH;
//					neck_5pro_dev_struct->dev_global.tra_speed = NONE_SPEED;
//					neck_5pro_dev_struct->ble_pack.heat_level = NONE_HEAT;
//					BOOST_DISABLE;
//				}

				if(aging_times >= 5)
				{
					aging_times = 5;
					neck_5pro_dev_struct->dev_global.is_aging_done = SET;
				}
				neck_5pro_dev_struct->ble_pack.mass_time = 300;
				neck_5pro_dev_struct->ble_pack.mass_strength = NONE_STRENGTH;
				neck_5pro_dev_struct->dev_global.tra_speed = NONE_SPEED;
				neck_5pro_dev_struct->ble_pack.heat_level = NONE_HEAT;
				BOOST_DISABLE;
			}
			else
			{
				if(neck_5pro_dev_struct->dev_global.is_aging_done == RESET)
				{
					neck_5pro_dev_struct->ble_pack.mass_time = 600;
					neck_5pro_dev_struct->ble_pack.mass_strength = MID_STRENGTH;
					neck_5pro_dev_struct->dev_global.tra_speed = MID_SPEED;
					neck_5pro_dev_struct->ble_pack.heat_level = WARM_HEAT;
					neck_5pro_dev_struct->ble_pack.mass_place = PLACE_5;
					BOOST_ENABLE;
				}
			}
			Topic_Pushlish(DEV_HEAT_SW, neck_5pro_dev_struct);
		}
		else
		{
			neck_5pro_dev_struct->dev_global.is_dev_on = RESET;
			neck_5pro_dev_struct->dev_global.is_dev_work = RESET;
			Topic_Pushlish(DEV_NO_WORK, neck_5pro_dev_struct);
			Topic_Pushlish(BUZZER_YOWL, neck_5pro_dev_struct);
		}
	}
	if(neck_5pro_dev_struct->ble_pack.mass_time > neck_5pro_dev_struct->dev_global.mass_cnt)
		neck_5pro_dev_struct->ble_pack.remain_time = neck_5pro_dev_struct->ble_pack.mass_time - neck_5pro_dev_struct->dev_global.mass_cnt;
	else
		neck_5pro_dev_struct->ble_pack.remain_time = 0;
}

kartun_t dev_kartun;
static void Motorthread_entry(void *para)
{
	static uint8_t fg2_cnt = 0, fg1_cnt = 0;
	static uint8_t outside_tra_cnt;
	uint32_t time = 25000;
	rt_uint32_t tra_time;
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();
	
	while(1)
	{
		//操作序列处理
		switch(dev_motor_struct.mode_opt)
		{
			case CHECK_PLACE_PRE:
				if(neck_5pro_dev_struct->ble_pack.is_manual == SET)			break;
				neck_5pro_dev_struct->dev_global.tra_speed = MID_SPEED;
				if(TRA_SW1_DETECT == SET && TRA_SW2_DETECT == RESET)
					dev_motor_struct.mode_opt = CHECK_PLACE;
				else
					neck_5pro_dev_struct->dev_global.tra_dir = TRA_REVERSE;
			break;
		
			case CHECK_PLACE:
				switch(neck_5pro_dev_struct->ble_pack.mass_mode)
				{
					case SOFT_MODE:
						if(neck_5pro_dev_struct->ble_pack.is_manual == SET)			break;
						time = 25000;
						neck_5pro_dev_struct->dev_global.tra_speed = MID_SPEED;
						if(TRA_SW1_DETECT == RESET && TRA_SW2_DETECT == SET)
						{
							dev_motor_struct.mode_opt = GO_MIDDLE;
//							dev_motor_struct.mode_optnd = GO_VRF_PLACE;
							dev_motor_struct.mode_optnd = START_TARGET_PLACE;
						}
						else if(TRA_SW1_DETECT == SET && TRA_SW2_DETECT == RESET)
							dev_motor_struct.mode_opt = GO_INSIDE;
						else
						{
							if(neck_5pro_dev_struct->ble_pack.curr_place == INSIDE)
							{
								dev_motor_struct.mode_opt = GO_MIDDLE;
								dev_motor_struct.mode_optnd = GO_VRF_PLACE;
							}	
							else
								dev_motor_struct.mode_opt = GO_OUTSIDE;
						}
					break;

					case MID_MODE:
						if(neck_5pro_dev_struct->ble_pack.is_manual == SET)			break;
						time = 25000;
						neck_5pro_dev_struct->dev_global.tra_speed = MID_SPEED;
						if(TRA_SW1_DETECT == RESET && TRA_SW2_DETECT == SET)
							dev_motor_struct.mode_opt = GO_OUTSIDE;
						else
							dev_motor_struct.mode_opt = GO_INSIDE;
					break;

					case HARD_MODE:
						if(neck_5pro_dev_struct->ble_pack.is_manual == SET)			break;
						time = 35000;
						neck_5pro_dev_struct->dev_global.tra_speed = MID_SPEED;
						if(TRA_SW1_DETECT == RESET && TRA_SW2_DETECT == SET)
							dev_motor_struct.mode_opt = GO_OUTSIDE;
						else
							dev_motor_struct.mode_opt = GO_INSIDE;
					break;

					case USER_MODE:
						if(neck_5pro_dev_struct->ble_pack.is_manual == SET)			break;
						time = 25000;
						neck_5pro_dev_struct->dev_global.tra_speed = MID_SPEED;
						switch(neck_5pro_dev_struct->ble_pack.mass_place)
						{
							case PLACE_0:
							break;

							case PLACE_1:
								dev_motor_struct.mode_opt = GO_OUTSIDE;
							break;

							case PLACE_2:
								if(TRA_SW1_DETECT == SET && TRA_SW2_DETECT == SET)
								{
									dev_motor_struct.mode_opt = START_MASS;
									dev_motor_struct.mode_optnd = GO_VRF_PLACE;
									neck_5pro_dev_struct->dev_global.tra_speed = NONE_SPEED;
									neck_5pro_dev_struct->ble_pack.curr_place = MIDDLE;
									Topic_Pushlish(UPDATE_HEART_PACK, neck_5pro_dev_struct);
								}
								else
								{
									dev_motor_struct.mode_opt = GO_MIDDLE;
									dev_motor_struct.mode_optnd = GO_VRF_PLACE;
								}
							break;

							case PLACE_3:
								if(TRA_SW2_DETECT != RESET)
									dev_motor_struct.mode_opt = GO_OUTSIDE;
								else
								{
									dev_motor_struct.mode_opt = GO_MIDDLE;
									dev_motor_struct.mode_optnd = GO_VRF_PLACE;
								}
							break;

							case PLACE_4:
								dev_motor_struct.mode_opt = GO_INSIDE;
							break;

							case PLACE_5:
								if(TRA_SW2_DETECT != RESET)
									dev_motor_struct.mode_opt = GO_OUTSIDE;
								else
									dev_motor_struct.mode_opt = GO_INSIDE;
							break;

							case PLACE_6:
								if(TRA_SW1_DETECT != RESET)
									dev_motor_struct.mode_opt = GO_INSIDE;
								else
								{
									dev_motor_struct.mode_opt = GO_MIDDLE;
//									dev_motor_struct.mode_optnd = GO_VRF_PLACE;
									dev_motor_struct.mode_optnd = START_TARGET_PLACE;
								}
							break;

							case PLACE_7:
								if(TRA_SW1_DETECT == RESET && TRA_SW2_DETECT == SET)
								{
									dev_motor_struct.mode_opt = GO_MIDDLE;
//									dev_motor_struct.mode_optnd = GO_VRF_PLACE;
									dev_motor_struct.mode_optnd = START_TARGET_PLACE;
								}
								else if(TRA_SW1_DETECT == SET && TRA_SW2_DETECT == RESET)
									dev_motor_struct.mode_opt = GO_INSIDE;
								else
								{
									if(neck_5pro_dev_struct->ble_pack.curr_place == INSIDE)
									{
										dev_motor_struct.mode_opt = GO_MIDDLE;
										dev_motor_struct.mode_optnd = GO_VRF_PLACE;
									}	
									else
										dev_motor_struct.mode_opt = GO_OUTSIDE;
								}
							break;
						}
					break;
				}
			break;

			case GO_INSIDE:
				if(TRA_SW1_DETECT != RESET)
				{
					neck_5pro_dev_struct->dev_global.tra_dir = TRA_FORWARD;
					//自适应脖围
					if(dev_motor_struct.tra_speed_pwm == dev_motor_struct.tra_speed_target)
					{
						if(g_usFG2Freq < FG2_THRESHOLD)
						{
							if(++fg2_cnt >= FG2_FILTER)
							{
								fg2_cnt = 0;
								dev_motor_struct.mode_opt = START_MASS;
								neck_5pro_dev_struct->dev_global.tra_speed = NONE_SPEED;
								if(neck_5pro_dev_struct->ble_pack.mass_mode == SOFT_MODE)
									neck_5pro_dev_struct->ble_pack.mass_strength = SOFT_STRENGTH;
								neck_5pro_dev_struct->ble_pack.curr_place = INSIDE;
								Topic_Pushlish(UPDATE_HEART_PACK, neck_5pro_dev_struct);

								LOG_W("here,FG2:%d", g_usFG2Freq);
							}
						}
						else
							fg2_cnt = 0;
					}
				}
				else
				{
					dev_motor_struct.mode_opt = START_MASS;
					//停止移动，设置按摩力度
					neck_5pro_dev_struct->dev_global.tra_speed = NONE_SPEED;
					if(neck_5pro_dev_struct->ble_pack.mass_mode == SOFT_MODE)
						neck_5pro_dev_struct->ble_pack.mass_strength = SOFT_STRENGTH;
					neck_5pro_dev_struct->ble_pack.curr_place = INSIDE;
					Topic_Pushlish(UPDATE_HEART_PACK, neck_5pro_dev_struct);
				}
			break;

			case GO_OUTSIDE:
				if(TRA_SW2_DETECT != RESET)
					neck_5pro_dev_struct->dev_global.tra_dir = TRA_REVERSE;
				else
				{
					dev_motor_struct.mode_opt = START_MASS;
					//停止移动
					neck_5pro_dev_struct->dev_global.tra_speed = NONE_SPEED;
					if(neck_5pro_dev_struct->ble_pack.mass_mode == SOFT_MODE)
						neck_5pro_dev_struct->ble_pack.mass_strength = HARD_STRENGTH;
					neck_5pro_dev_struct->ble_pack.curr_place = OUTSIDE;
					Topic_Pushlish(UPDATE_HEART_PACK, neck_5pro_dev_struct);
				}
			break;

			case GO_MIDDLE:
				switch(dev_motor_struct.mode_optnd)
				{
					case GO_VRF_PLACE:
						if(TRA_SW2_DETECT != RESET)
							neck_5pro_dev_struct->dev_global.tra_dir = TRA_REVERSE;
						else
						{
							dev_motor_struct.mode_optnd = START_TARGET_PLACE;
						}
					break;

					case START_TARGET_PLACE:
						if(tra_sftimer != RT_NULL)	
						{
							tra_time = 3500;
							rt_timer_control(tra_sftimer, RT_TIMER_CTRL_SET_TIME, &tra_time);
							rt_timer_start(tra_sftimer);
							if(TRA_SW1_DETECT != RESET)
								neck_5pro_dev_struct->dev_global.tra_dir = TRA_FORWARD;
							else
								neck_5pro_dev_struct->dev_global.tra_dir = TRA_REVERSE;
							dev_motor_struct.mode_optnd = STOP_TARGET_PLACE;
						}
					break;

					case STOP_TARGET_PLACE:
						//自适应脖围
						if(dev_motor_struct.tra_speed_pwm == dev_motor_struct.tra_speed_target)
						{
							if(g_usFG2Freq < FG2_THRESHOLD)
							{
								if(++fg2_cnt >= FG2_FILTER)
								{
									fg2_cnt = 0;
									if(tra_sftimer != RT_NULL)			rt_timer_stop(tra_sftimer);
									dev_motor_struct.mode_opt = START_MASS;
									dev_motor_struct.mode_optnd = GO_VRF_PLACE;
									//停止移动，设置按摩力度
									neck_5pro_dev_struct->dev_global.tra_speed = NONE_SPEED;
									if(neck_5pro_dev_struct->ble_pack.mass_mode == SOFT_MODE)
										neck_5pro_dev_struct->ble_pack.mass_strength = MID_STRENGTH;
									neck_5pro_dev_struct->ble_pack.curr_place = MIDDLE;
									Topic_Pushlish(UPDATE_HEART_PACK, neck_5pro_dev_struct);

									LOG_W("here,FG2:%d", g_usFG2Freq);
								}
							}
							else
								fg2_cnt = 0;
						}
					break;
				}
			break;

			case START_MASS:
				if(mass_sftimer != RT_NULL)		
				{
					rt_timer_control(mass_sftimer, RT_TIMER_CTRL_SET_TIME, &time);
					rt_timer_start(mass_sftimer);
					dev_motor_struct.mode_opt = STOP_MASS;
				}
			break;
		}

		//堵转处理
		if(neck_5pro_dev_struct->dev_global.is_dev_work == SET && neck_5pro_dev_struct->dev_status != DEV_AGING)//仅正常工作时生效
		{
			if(dev_motor_struct.mass_speed_pwm == dev_motor_struct.mass_speed_target)
			{
				if(g_usFG1Freq < FG1_THRESHOLD)
				{
					if(++fg1_cnt >= FG1_FILTER)
					{
						fg1_cnt = 0;
						dev_motor_struct.mass_block++;
						//反转
						LOG_E("block cnt++,FG1:%d,FG2:%d", g_usFG1Freq,g_usFG2Freq);
						if(neck_5pro_dev_struct->ble_pack.mass_dir == MASS_FORWARD)
							neck_5pro_dev_struct->ble_pack.mass_dir = MASS_REVERSE;
						else
							neck_5pro_dev_struct->ble_pack.mass_dir = MASS_FORWARD;
						//往外移动一点
						outside_tra_cnt = 100;
						while(1)
						{
							outside_tra_cnt--;
							if(outside_tra_cnt > 0)
							{
								MOTOR_TRA_REVERSE;
								bsp_SetTIMOutPWM(GPIOB, GPIO_PIN_0, TIM3, 3, 20000, 6000);
								if(TRA_SW2_DETECT == RESET)
									break;
							}
							else
								break;

							rt_thread_mdelay(10);
						}
							
						if(dev_motor_struct.mass_block >= 0x03)	
						{
							//发布关机主题
							LOG_E("motor block..");
							neck_5pro_dev_struct->dev_global.is_dev_on = RESET;
//							Topic_Pushlish(DEV_NO_WORK, neck_5pro_dev_struct);
						}
					}
				}
				else if(g_usFG1Freq > (FG1_THRESHOLD+30))
				{
					fg1_cnt = 0;
				}
			}
		}	
		
		//速度渐变
		dev_motor_struct.mass_speed_target = gc_mass_strength[neck_5pro_dev_struct->ble_pack.mass_strength];
		if(dev_motor_struct.mass_speed_pwm < dev_motor_struct.mass_speed_target)
		{
			if((dev_motor_struct.mass_speed_target - dev_motor_struct.mass_speed_pwm) > 100)	
				dev_motor_struct.mass_speed_pwm += 100;
			else												
				dev_motor_struct.mass_speed_pwm = dev_motor_struct.mass_speed_target;
		}
		else if(dev_motor_struct.mass_speed_pwm > dev_motor_struct.mass_speed_target)
		{
			if((dev_motor_struct.mass_speed_pwm - dev_motor_struct.mass_speed_target) > 100)	
				dev_motor_struct.mass_speed_pwm -= 100;
			else												
				dev_motor_struct.mass_speed_pwm = dev_motor_struct.mass_speed_target;
		}
		bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_7, TIM3, 2, 20000, (10000 - dev_motor_struct.mass_speed_pwm));

		dev_motor_struct.tra_speed_target = gc_tra_speed[neck_5pro_dev_struct->dev_global.tra_speed];
		//定点、手动移动处理
		if(neck_5pro_dev_struct->ble_pack.is_manual == SET)
		{
			if(neck_5pro_dev_struct->dev_global.manual_tra_start == SET)
				dev_motor_struct.tra_speed_target = gc_tra_speed[MID_SPEED];
			else
				dev_motor_struct.tra_speed_target = gc_tra_speed[NONE_SPEED];
		}

		if(dev_motor_struct.tra_speed_pwm < dev_motor_struct.tra_speed_target)
		{
			if((dev_motor_struct.tra_speed_target - dev_motor_struct.tra_speed_pwm) > 1000)	
				dev_motor_struct.tra_speed_pwm += 1000;
			else												
				dev_motor_struct.tra_speed_pwm = dev_motor_struct.tra_speed_target;
		}
		else if(dev_motor_struct.tra_speed_pwm > dev_motor_struct.tra_speed_target)
		{
			if((dev_motor_struct.tra_speed_pwm - dev_motor_struct.tra_speed_target) > 1000)	
				dev_motor_struct.tra_speed_pwm -= 1000;
			else												
				dev_motor_struct.tra_speed_pwm = dev_motor_struct.tra_speed_target;
		}
		bsp_SetTIMOutPWM(GPIOB, GPIO_PIN_0, TIM3, 3, 20000, (10000 - dev_motor_struct.tra_speed_pwm));

		//方向处理
		if(neck_5pro_dev_struct->ble_pack.mass_dir == MASS_FORWARD)
			MOTOR_MASS_FORWARD;
		else
			MOTOR_MASS_REVERSE;

		if(neck_5pro_dev_struct->dev_global.tra_dir == TRA_FORWARD)
		{
			if(TRA_SW1_DETECT == RESET)
			{
				neck_5pro_dev_struct->dev_global.tra_dir = TRA_REVERSE;
				MOTOR_TRA_REVERSE;
			}
			else
				MOTOR_TRA_FORWARD;
		}
		else
		{
			if(TRA_SW2_DETECT == RESET)
			{
				neck_5pro_dev_struct->dev_global.tra_dir = TRA_FORWARD;
				MOTOR_TRA_FORWARD;
			}
			else
				MOTOR_TRA_REVERSE;
		}

		rt_thread_mdelay(10);
	}
}

void get_fg(void)
{
	LOG_I("FG1:%d", g_usFG1Freq);
	LOG_I("FG2:%d", g_usFG2Freq);
}
MSH_CMD_EXPORT(get_fg, get_fg);

static void motor_dev_work_cb(void *msg)
{
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();
	
	motor_create_thread();
	neck_5pro_dev_struct->ble_pack.mass_mode = SOFT_MODE;
	neck_5pro_dev_struct->ble_pack.mass_dir = MASS_FORWARD;
	neck_5pro_dev_struct->ble_pack.mass_strength = SOFT_STRENGTH;
	neck_5pro_dev_struct->dev_global.tra_dir = TRA_FORWARD;
	neck_5pro_dev_struct->dev_global.tra_speed = MID_SPEED;
	dev_motor_struct.mode_opt = CHECK_PLACE_PRE;
	if(work_sftimer != RT_NULL)		rt_timer_start(work_sftimer);
	if(dir_sftimer != RT_NULL)		rt_timer_start(dir_sftimer);
	BOOST_ENABLE;
}

static void motor_dev_no_work_cb(void *msg)
{
	BOOST_DISABLE;
	if(rt_thread_find("motor") != RT_NULL)
		rt_thread_delete(rt_thread_find("motor"));
	if(work_sftimer != RT_NULL)		rt_timer_stop(work_sftimer);
}

static void motor_mode_change_cb(void *msg)
{
	rt_uint32_t time;
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();

	dev_motor_struct.mode_opt = CHECK_PLACE_PRE;
	neck_5pro_dev_struct->dev_global.tra_speed = MID_SPEED;
	switch(neck_5pro_dev_struct->ble_pack.mass_mode)
	{
		case SOFT_MODE:
			time = 120000;
			rt_timer_control(dir_sftimer, RT_TIMER_CTRL_SET_TIME, &time);
			rt_timer_start(dir_sftimer);
		break;

		case MID_MODE:
			time = 60000;
			rt_timer_control(dir_sftimer, RT_TIMER_CTRL_SET_TIME, &time);
			rt_timer_start(dir_sftimer);
			time = 120000;
			rt_timer_control(strength_sftimer, RT_TIMER_CTRL_SET_TIME, &time);
			rt_timer_start(strength_sftimer);
		break;

		case HARD_MODE:
			time = 60000;
			rt_timer_control(dir_sftimer, RT_TIMER_CTRL_SET_TIME, &time);
			rt_timer_start(dir_sftimer);
		break;

		default:
		break;
	}
}

static void motor_manual_sw_cb(void *msg)
{
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();

	dev_motor_struct.mode_opt = CHECK_PLACE_PRE;
	if(neck_5pro_dev_struct->ble_pack.is_manual == RESET)
		neck_5pro_dev_struct->dev_global.tra_speed = MID_SPEED;
	else
		neck_5pro_dev_struct->dev_global.tra_speed = NONE_SPEED;
}

static void motor_about_topic_subscrib(void)
{
	Topic_Subscrib(DEV_WORK, motor_dev_work_cb);
	Topic_Subscrib(DEV_NO_WORK, motor_dev_no_work_cb);
	Topic_Subscrib(DEV_MODE_CHANGE, motor_mode_change_cb);
	Topic_Subscrib(DEV_BLE_CMD, motor_mode_change_cb);
	Topic_Subscrib(DEV_MANUAL_SW, motor_manual_sw_cb);
}
user_initcall(motor_about_topic_subscrib);

void motor_sftimer_init(void)
{
	mass_sftimer = rt_timer_create("mass_timer", mass_sftimer_cb, RT_NULL, 25000, RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_PERIODIC);
	dir_sftimer = rt_timer_create("dir_timer", dir_sftimer_cb, RT_NULL, 120000, RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_PERIODIC);
	strength_sftimer = rt_timer_create("strength_sftimer", strength_sftimer_cb, RT_NULL, 60000, RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_PERIODIC);
	tra_sftimer = rt_timer_create("tra_sftimer", tra_sftimer_cb, RT_NULL, 3500, RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_PERIODIC);
	work_sftimer = rt_timer_create("work_sftimer", work_sftimer_cb, RT_NULL, 1000, RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_PERIODIC);
}

void motor_create_thread(void)
{
	rt_thread_t rt_Motorthread_id = RT_NULL;
	
	rt_Motorthread_id = rt_thread_create("motor",
								Motorthread_entry,
								RT_NULL,
								512,		//stack size
								16,			//prioity
								10);		//timeslice
	if(rt_Motorthread_id != RT_NULL)
		rt_thread_startup(rt_Motorthread_id);
}


