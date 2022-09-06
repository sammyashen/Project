#include "mod_hmi.h"

//mod mq
rt_mq_t mod_hmi_mq = RT_NULL;

//led evtgroup
rt_event_t led_sta_evt = RT_NULL;
rt_event_t led_mode_evt = RT_NULL;
rt_event_t beezer_evt = RT_NULL;

/*
*@brief   HMI�̣߳�����LED��BEEZER��KEY�߼�
*@author  shy.breo
*@version V1.0.0
*@note	  ����ʹ��ǰ�����ȳ�ʼ��
*		  PUSH����Ϣ���д��HMIģ�������������Ϣ
*		  GET���¼���־�顢�ⲿ��Ϣ����
*/
void HMIthread_entry(void *para)
{
	uint8_t _ucKeyCode = KEY_NONE;
	rt_uint32_t _ulRecvEvtMask = 0x00;
	static T_KEY_Model s_tKEY_Model;
	static T_BEEP_Model s_tBEEP_Model;

	/* LED Dev Configer */
	//�ϵ�SOFTģʽ�Ƴ����������ƺ����Ƴ�����״̬���ϵƳ���
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
	mod_hmi_mq = rt_mq_create("mod_hmi_mq", 				//��Ϣ��������
								50,							//ÿ����Ϣ�Ĵ�С
								5, 							//��Ϣ���е���Ϣ����
								RT_IPC_FLAG_FIFO);			//FIFOģʽ

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
											IS_SW_MODE, 								//Ŀ���־
											RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 	//�߼����յ���־�������־
											RT_WAITING_NO,								//���ȴ�
											RT_NULL)									//ָ���յ����¼���ͨ�����Զ��������¼�
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
						//�ϵƳ���
					break;

					case IS_CHARGE_HIGH_T:
						//��Ƴ���
					break;

					case IS_CHARGE_ING:
						//��ƺ���
					break;

					case IS_CHARGE_DONE:
						//�̵Ƴ���
					break;

					case IS_LOWPWR_NOPROT:
						//��Ƴ���˸
					break;

					case IS_LOWPWR_PROT:
						//�����˸3�κ�ػ�
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







