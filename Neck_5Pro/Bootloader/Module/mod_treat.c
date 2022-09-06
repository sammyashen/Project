#include "mod_treat.h"

rt_mq_t mod_treat_mq = RT_NULL;
rt_event_t mod_treat_evt = RT_NULL;


/*
*@brief   TREAT线程，处理加热、电机控制逻辑
*@author  shy.breo
*@version V1.0.0
*@note	  外设使用前必须先初始化
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
	mod_treat_mq = rt_mq_create("mod_treat_mq", 			//消息队列名称
								50,							//每条消息的大小
								5, 							//消息队列的消息容量
								RT_IPC_FLAG_FIFO);			//FIFO模式

	/* EvtGroup Configer */
	mod_treat_evt = rt_event_create("mod_treat_evt", RT_IPC_FLAG_FIFO);

	while(1)
	{
		rt_thread_mdelay(10);
	}
}


