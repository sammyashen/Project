#include "mod_pm.h"

rt_mq_t mod_pm_mq = RT_NULL;
rt_event_t mod_pm_evt = RT_NULL;


/*
*@brief   ��ع����߳�
*@author  shy.breo
*@version V1.0.0
*@note	  ����ʹ��ǰ�����ȳ�ʼ��
*/
void PMthread_entry(void *para)
{
	/* ADC Configer */
	//TREAT�߳��ѳ�ʼ�����˴������ٴγ�ʼ��

	/* Power Manager Unit Configer */
	MP_DEV_Configure();

	/* MsgQueue Configer */
	mod_pm_mq = rt_mq_create("mod_pm_mq", 					//��Ϣ��������
								50,							//ÿ����Ϣ�Ĵ�С
								5, 							//��Ϣ���е���Ϣ����
								RT_IPC_FLAG_FIFO);			//FIFOģʽ

	/* EvtGroup Configer */
	mod_pm_evt = rt_event_create("mod_pm_evt", RT_IPC_FLAG_FIFO);

	while(1)
	{
		
	
		rt_thread_mdelay(50);
	}
}

