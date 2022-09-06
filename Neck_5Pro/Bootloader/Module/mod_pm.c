#include "mod_pm.h"

rt_mq_t mod_pm_mq = RT_NULL;
rt_event_t mod_pm_evt = RT_NULL;


/*
*@brief   电池管理线程
*@author  shy.breo
*@version V1.0.0
*@note	  外设使用前必须先初始化
*/
void PMthread_entry(void *para)
{
	/* ADC Configer */
	//TREAT线程已初始化，此处无需再次初始化

	/* Power Manager Unit Configer */
	MP_DEV_Configure();

	/* MsgQueue Configer */
	mod_pm_mq = rt_mq_create("mod_pm_mq", 					//消息队列名称
								50,							//每条消息的大小
								5, 							//消息队列的消息容量
								RT_IPC_FLAG_FIFO);			//FIFO模式

	/* EvtGroup Configer */
	mod_pm_evt = rt_event_create("mod_pm_evt", RT_IPC_FLAG_FIFO);

	while(1)
	{
		
	
		rt_thread_mdelay(50);
	}
}

