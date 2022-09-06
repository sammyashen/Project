#include "dev_buzzer.h"
#include "topic.h"
#include "include.h"
#include "drv_gpio.h"
#include <easyflash.h>



static BEEP_T g_tBeep;
#define BEEP_ENABLE()	bsp_SetTIMOutPWM_N(GPIOB, GPIO_PIN_14, TIM1, 2, g_tBeep.uiFreq, 5000)
#define BEEP_DISABLE()	bsp_SetTIMOutPWM_N(GPIOB, GPIO_PIN_14, TIM1, 2, 1500, 0)

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

	BEEP_ENABLE();	/* ��ʼ���� */		
}

static void beep_stop(void)
{
	g_tBeep.ucEnalbe = 0;

	BEEP_DISABLE();	/* ����������Ʊ�־����ֹͣ����������ֹͣ�����ж����ֿ��� */
}

void beep_process(void)
{
	if ((g_tBeep.ucEnalbe == 0) || (g_tBeep.usStopTime == 0))
	{
		return;
	}

	if (g_tBeep.ucState == 0)
	{
		if (g_tBeep.usStopTime > 0)	/* ��Ϸ��� */
		{
			if (++g_tBeep.usCount >= g_tBeep.usBeepTime)
			{
				BEEP_DISABLE();		/* ֹͣ���� */
				g_tBeep.usCount = 0;
				g_tBeep.ucState = 1;
			}
		}
		else
		{
			;	/* �����κδ����������� */
		}
	}
	else if (g_tBeep.ucState == 1)
	{
		if (++g_tBeep.usCount >= g_tBeep.usStopTime)
		{
			/* ��������ʱ��ֱ������stopֹͣΪֹ */
			if (g_tBeep.usCycle > 0)
			{
				if (++g_tBeep.usCycleCount >= g_tBeep.usCycle)
				{
					/* ѭ����������ֹͣ���� */
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

			BEEP_ENABLE();			/* ��ʼ���� */
		}
	}
}

rt_timer_t buzzer_sftimer = RT_NULL;
static void buzzer_sftimer_cb(void *para)
{
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();

	beep_stop();
	if(neck_5pro_dev_struct->dev_global.is_dev_on == RESET)	//�ػ����ȳ������������
	{
		if(neck_5pro_dev_struct->flash_para.is_save_recipe == 0x01)
		{
			if(neck_5pro_dev_struct->ble_pack.mass_mode == USER_MODE)
			{
				if(neck_5pro_dev_struct->ble_pack.is_manual == SET)
					neck_5pro_dev_struct->flash_para.place_recipe = PLACE_0;
			}
			if(ef_set_env_blob("user_mode_recipe", &neck_5pro_dev_struct->flash_para, sizeof(flash_para_t)) == EF_NO_ERR)//����ȴ��洢������ɲŵ���
			{
				LOG_I("save recipe successed.");
				PWR_OFF;
			}
		}
		else
		{
			PWR_OFF;
		}
	}
}

void buzzer_sftimer_init(void)
{
	buzzer_sftimer = rt_timer_create("buzzer_timer", buzzer_sftimer_cb, RT_NULL, 400, RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_ONE_SHOT);
}

static void buzzer_yowl_cb(void *msg)
{
	rt_uint32_t time;

	if(buzzer_sftimer != RT_NULL)	
	{
		beep_start(1500, 40, 40, 1);
		time = 400;
		rt_timer_control(buzzer_sftimer, RT_TIMER_CTRL_SET_TIME, &time);
		rt_timer_start(buzzer_sftimer);
	}
}

static void buzzer_mode_change_cb(void *msg)
{
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();

	beep_start(1500, 20, 20, neck_5pro_dev_struct->ble_pack.mass_mode);
}

static void buzzer_tone_cb(void *msg)
{
	beep_start(1500, 20, 20, 1);
}

//buzzer�豸������ⶩ��
static void buzzer_about_topic_subscrib(void)
{
	Topic_Subscrib(BUZZER_YOWL, buzzer_yowl_cb);
	Topic_Subscrib(DEV_MODE_CHANGE, buzzer_mode_change_cb);
	Topic_Subscrib(BUZZER_TONE, buzzer_tone_cb);
}
user_initcall(buzzer_about_topic_subscrib);



