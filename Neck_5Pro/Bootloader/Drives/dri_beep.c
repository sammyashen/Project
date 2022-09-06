#include "dri_beep.h"

static void BEEP_InitHard(void);
static void BEEP_Stop(void);
static void BEEP_Start(uint32_t _uiFreq, uint16_t _usBeepTime, uint16_t _usStopTime, uint16_t _usCycle);
static void BEEP_Pro(void);

static T_BEEP_Device s_tBEEP_Dev = {
	.BEEP_Config = BEEP_InitHard,
	.BEEP_On = BEEP_Start,
	.BEEP_Off = BEEP_Stop,
	.BEEP_Handler = BEEP_Pro
};



//#define BEEP_HAVE_POWER		/* ������б�ʾ��Դ��������ֱ��ͨ��GPIO����, ����PWM */

static BEEP_T g_tBeep;		/* ���������ȫ�ֽṹ����� */

#ifdef	BEEP_HAVE_POWER		/* ��Դ������ */
	#define GPIO_RCC_BEEP   RCC_APB2_PERIPH_GPIOC
	#define GPIO_PORT_BEEP	GPIOC
	#define GPIO_PIN_BEEP	GPIO_PIN_11

	#define BEEP_ENABLE()	GPIO_PORT_BEEP->PBSC = GPIO_PIN_BEEP		/* ʹ�ܷ��������� */
	#define BEEP_DISABLE()	GPIO_PORT_BEEP->PBC = GPIO_PIN_BEEP			/* ��ֹ���������� */
#else		/* ��Դ������ */
	/* 1500��ʾƵ��1.5KHz��5000��ʾ50.00%��ռ�ձ� */
//	#define BEEP_ENABLE()	bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_8, TIM1, 1, g_tBeep.uiFreq, 5000);

	/* ��ֹ���������� */
//	#define BEEP_DISABLE()	bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_8, TIM1, 1, 1500, 0);

	/* ��ͨIOģ��pwm�����4KHz��50%��ռ�ձȣ�����˵��8KHzƵ�ʷ�תIO״̬ */
	#define GPIO_RCC_BEEP   RCC_APB2_PERIPH_GPIOC
	#define GPIO_PORT_BEEP	GPIOC
	#define GPIO_PIN_BEEP	GPIO_PIN_14

	#define BEEP_ENABLE()	GPIO_PORT_BEEP->PBC = GPIO_PIN_BEEP;TIM_Enable(TIM2, ENABLE);
	#define BEEP_DISABLE()	TIM_Enable(TIM2, DISABLE);GPIO_PORT_BEEP->PBSC = GPIO_PIN_BEEP;
#endif

/*
*@brief 	������ģ��ע�ắ��
*@author 	shenhy
*@param[in] _ptBEEP_Model:������ģ��ָ��
*@retval	null
*/
void Register_BEEP_Model(T_BEEP_Model *_ptBEEP_Model)
{
	_ptBEEP_Model->beep_dev = &s_tBEEP_Dev;
}

/*
*@brief 	������������ʼ������
*@author 	shenhy
*@param[in] null
*@retval	null
*/
static void BEEP_InitHard(void)
{
#ifdef	BEEP_HAVE_POWER		
	GPIO_InitType GPIO_InitStructure;

	RCC_EnableAPB2PeriphClk(GPIO_RCC_BEEP, ENABLE);

	BEEP_DISABLE();

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
	GPIO_InitStructure.Pin = GPIO_PIN_BEEP;
	GPIO_InitPeripheral(GPIO_PORT_BEEP, &GPIO_InitStructure);
#else
//	;
	/* ��ͨIOģ��pwm��� */
	GPIO_InitType 			GPIO_InitStructure;
	TIM_TimeBaseInitType  	TIM_TimeBaseStructure;
	NVIC_InitType 			NVIC_InitStructure;

	RCC_EnableAPB2PeriphClk(GPIO_RCC_BEEP, ENABLE);
	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM2, ENABLE);
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
	GPIO_InitStructure.Pin = GPIO_PIN_BEEP;
	GPIO_InitPeripheral(GPIO_PORT_BEEP, &GPIO_InitStructure);
	
	TIM_TimeBaseStructure.Period = 90 - 1;		
	TIM_TimeBaseStructure.Prescaler = 100 - 1;
	TIM_TimeBaseStructure.ClkDiv = TIM_CLK_DIV1;				
	TIM_TimeBaseStructure.CntMode = TIM_CNT_MODE_UP;
	TIM_TimeBaseStructure.RepetCnt = 0;	
	TIM_InitTimeBase(TIM2, &TIM_TimeBaseStructure);
	TIM_ClearFlag(TIM2, TIM_FLAG_UPDATE);
	TIM_ConfigInt(TIM2, TIM_INT_UPDATE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	BEEP_DISABLE();
	
#endif
}

/*
*@brief 	������������������
*@author 	shenhy
*@param[in] _uiFreq:������Ƶ��
*@param[in]	_usBeepTime:����������ʱ��
*@param[in]	_usStopTime:����������ʱ��
*@param[in]	_usCycle:����������ѭ������
*@retval	null
*/
static void BEEP_Start(uint32_t _uiFreq, uint16_t _usBeepTime, uint16_t _usStopTime, uint16_t _usCycle)
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

/*
*@brief 	����������ֹͣ����
*@author 	shenhy
*@param[in] null
*@retval	null
*/
static void BEEP_Stop(void)
{
	g_tBeep.ucEnalbe = 0;

	BEEP_DISABLE();	/* ����������Ʊ�־����ֹͣ����������ֹͣ�����ж����ֿ��� */
}

/*
*@brief 	������ѭ��������
*@author 	shenhy
*@param[in] null
*@retval	null
*/
static void BEEP_Pro(void)
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

void TIM2_IRQHandler(void)
{
	if(TIM_GetIntStatus(TIM2, TIM_INT_UPDATE) != RESET)
	{
		GPIO_PORT_BEEP->POD ^= GPIO_PIN_BEEP;
		TIM_ClrIntPendingBit(TIM2, TIM_INT_UPDATE);
	}
}



