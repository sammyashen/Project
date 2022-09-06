#include "mod_hmi.h"

//mod mq
rt_mq_t mod_hmi_mq = RT_NULL;

//led evtgroup
rt_event_t led_sta_evt = RT_NULL;
rt_event_t led_mode_evt = RT_NULL;
rt_event_t beezer_evt = RT_NULL;

/*
*@brief   HMI线程，处理LED、BEEZER、KEY逻辑
*@author  shy.breo
*@version V1.0.0
*@note	  外设使用前必须先初始化
*		  PUSH：消息队列存放HMI模块自身产生的消息
*		  GET：事件标志组、外部消息队列
*/
void HMIthread_entry(void *para)
{
	uint8_t _ucKeyCode = KEY_NONE;
	rt_uint32_t _ulRecvEvtMask = 0x00;
	static T_KEY_Model s_tKEY_Model;
	static T_BEEP_Model s_tBEEP_Model;

	/* LED Dev Configer */
	//上电SOFT模式灯常亮、颈部灯呼吸灯常亮、状态灯紫灯常亮
	g_tLED_Dev.LED_Config();
	g_tLED_Dev.LED_Mode_Run(TREAT_SOFT_MODE);
	g_tLED_Dev.LED_Neck_BrRun(SET);
	g_tLED_Dev.LED_Sta_Run(PURPLE_COLOR, 255);

	/* KEY Dev Configer */
	Register_KEY_Model(&s_tKEY_Model);
	s_tKEY_Model.Key_dev->Key_Init();

	/* BEEZER Dev Configer */
	Register_BEEP_Model(&s_tBEEP_Model);
	s_tBEEP_Model.beep_dev->BEEP_Config();

	/* MsgQueue Configer */
	mod_hmi_mq = rt_mq_create("mod_hmi_mq", 				//消息队列名称
								50,							//每条消息的大小
								5, 							//消息队列的消息容量
								RT_IPC_FLAG_FIFO);			//FIFO模式

	/* EvtGroup Configer */
	led_sta_evt = rt_event_create("led_sta_evt", RT_IPC_FLAG_FIFO);	
	led_mode_evt = rt_event_create("led_mode_evt", RT_IPC_FLAG_FIFO);	
	beezer_evt = rt_event_create("beezer_evt", RT_IPC_FLAG_FIFO);
	
	while(1)
	{
		//LED CTRL
		if(led_mode_evt != RT_NULL)
		{
			if(
				RT_EOK == rt_event_recv(led_mode_evt,
											IS_SW_MODE, 								//目标标志
											RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 	//逻辑或，收到标志后清楚标志
											RT_WAITING_NO,								//不等待
											RT_NULL)									//指向收到的事件，通常用以读出本次事件
			)
			{
				switch(g_tDevParam.TRAET_MODE)
				{
					case TREAT_SOFT_MODE:
						//SOFT LED ON;
						
					break;

					case TREAT_MED_MODE:
						//MED LED ON;
						
					break;

					case TREAT_HARD_MODE:
						//HARD LED ON;
						
					break;
				}
			}
		}

		if(led_sta_evt != RT_NULL)
		{
			if(
				RT_EOK == rt_event_recv(led_sta_evt, 
											IS_PWR_NORMAL | IS_CHARGE_HIGH_T | IS_CHARGE_ING |
											IS_CHARGE_DONE | IS_LOWPWR_NOPROT | IS_LOWPWR_PROT,
											RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
											RT_WAITING_NO,
											&_ulRecvEvtMask)
			)
			{
				switch(_ulRecvEvtMask)
				{
					case IS_PWR_NORMAL:
						//紫灯常亮
					break;

					case IS_CHARGE_HIGH_T:
						//红灯常亮
					break;

					case IS_CHARGE_ING:
						//红灯呼吸
					break;

					case IS_CHARGE_DONE:
						//绿灯常亮
					break;

					case IS_LOWPWR_NOPROT:
						//红灯常闪烁
					break;

					case IS_LOWPWR_PROT:
						//红灯闪烁3次后关机
					break;
					
				}
			}
		}

		//KEY CTRL
		_ucKeyCode = s_tKEY_Model.Key_dev->GetKeyVal();
		if(_ucKeyCode != KEY_NONE)
		{
			switch(_ucKeyCode)
			{
				case KEY_1_UP:	//S1
					rt_event_send(led_mode_evt, IS_SW_MODE);
					rt_event_send(beezer_evt, IS_SW_MODE);
					
					rt_mutex_take(DevPara_mutex, RT_WAITING_FOREVER);
					g_tDevParam.TRAET_MODE++;
					if(g_tDevParam.TRAET_MODE == 0x03)	g_tDevParam.TRAET_MODE = TREAT_SOFT_MODE;
					rt_mutex_release(DevPara_mutex);
				break;
			}
		}
		s_tKEY_Model.Key_dev->KeyScan();

		//BEEZER CTRL
		if(
			RT_EOK == rt_event_recv(beezer_evt,
										IS_DEV_ON | IS_DEV_OFF | IS_SW_MODE,
										RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
										RT_WAITING_NO,
										&_ulRecvEvtMask)
		)
		{
			switch(_ulRecvEvtMask)
			{
				case IS_DEV_ON:
					s_tBEEP_Model.beep_dev->BEEP_On(1000, 40, 20, 1);
				break;

				case IS_DEV_OFF:
					s_tBEEP_Model.beep_dev->BEEP_On(1000, 40, 20, 1);
				break;

				case IS_SW_MODE:
					switch(g_tDevParam.TRAET_MODE)
					{
						case TREAT_SOFT_MODE:
							s_tBEEP_Model.beep_dev->BEEP_On(1000, 20, 20, 3);
						break;

						case TREAT_MED_MODE:
							s_tBEEP_Model.beep_dev->BEEP_On(1000, 20, 20, 1);
						break;

						case TREAT_HARD_MODE:
							s_tBEEP_Model.beep_dev->BEEP_On(1000, 20, 20, 2);
						break;
					}
				break;
			}
		}
		s_tBEEP_Model.beep_dev->BEEP_Handler();

		uart_poll_dma_tx(DEV_UART1);
		rt_thread_mdelay(10);
	}
}







