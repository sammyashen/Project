#include "mod_treat.h"

rt_mq_t mod_treat_mq = RT_NULL;
rt_event_t mod_treat_evt = RT_NULL;


/*
*@brief   TREAT�̣߳�������ȡ���������߼�
*@author  shy.breo
*@version V1.0.0
*@note	  ����ʹ��ǰ�����ȳ�ʼ��
*/
void TREATthread_entry(void *para)
{
	static T_PWM_Model s_tPWM_Model;

	/* PWM Configer */
	Register_PWM_Model(&s_tPWM_Model);

	/* ADC Configer */
	ADC1_Init();
	ADC2_Init();

	/* UART Dev Configer */
	bsp_uart1_init();
	uart_device_init(DEV_UART1);

	/* PID Configer */

	/* Mutex Configer */
	DevPara_mutex = rt_mutex_create("devpara_mutex", RT_IPC_FLAG_FIFO);

	/* MsgQueue Configer */
	mod_treat_mq = rt_mq_create("mod_treat_mq", 			//��Ϣ��������
								50,							//ÿ����Ϣ�Ĵ�С
								5, 							//��Ϣ���е���Ϣ����
								RT_IPC_FLAG_FIFO);			//FIFOģʽ

	/* EvtGroup Configer */
	mod_treat_evt = rt_event_create("mod_treat_evt", RT_IPC_FLAG_FIFO);

	while(1)
	{
		rt_thread_mdelay(10);
	}
}


