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



//#define BEEP_HAVE_POWER		/* 定义此行表示有源蜂鸣器，直接通过GPIO驱动, 无需PWM */

static BEEP_T g_tBeep;		/* 定义蜂鸣器全局结构体变量 */

#ifdef	BEEP_HAVE_POWER		/* 有源蜂鸣器 */
	#define GPIO_RCC_BEEP   RCC_APB2_PERIPH_GPIOC
	#define GPIO_PORT_BEEP	GPIOC
	#define GPIO_PIN_BEEP	GPIO_PIN_11

	#define BEEP_ENABLE()	GPIO_PORT_BEEP->PBSC = GPIO_PIN_BEEP		/* 使能蜂鸣器鸣叫 */
	#define BEEP_DISABLE()	GPIO_PORT_BEEP->PBC = GPIO_PIN_BEEP			/* 禁止蜂鸣器鸣叫 */
#else		/* 无源蜂鸣器 */
	/* 1500表示频率1.5KHz，5000表示50.00%的占空比 */
//	#define BEEP_ENABLE()	bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_8, TIM1, 1, g_tBeep.uiFreq, 5000);

	/* 禁止蜂鸣器鸣叫 */
//	#define BEEP_DISABLE()	bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_8, TIM1, 1, 1500, 0);

	/* 普通IO模拟pwm输出：4KHz，50%的占空比，就是说以8KHz频率翻转IO状态 */
	#define GPIO_RCC_BEEP   RCC_APB2_PERIPH_GPIOC
	#define GPIO_PORT_BEEP	GPIOC
	#define GPIO_PIN_BEEP	GPIO_PIN_14

	#define BEEP_ENABLE()	GPIO_PORT_BEEP->PBC = GPIO_PIN_BEEP;TIM_Enable(TIM2, ENABLE);
	#define BEEP_DISABLE()	TIM_Enable(TIM2, DISABLE);GPIO_PORT_BEEP->PBSC = GPIO_PIN_BEEP;
#endif

/*
*@brief 	蜂鸣器模块注册函数
*@author 	shenhy
*@param[in] _ptBEEP_Model:蜂鸣器模块指针
*@retval	null
*/
void Register_BEEP_Model(T_BEEP_Model *_ptBEEP_Model)
{
	_ptBEEP_Model->beep_dev = &s_tBEEP_Dev;
}

/*
*@brief 	蜂鸣器驱动初始化函数
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
	/* 普通IO模拟pwm输出 */
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
*@brief 	蜂鸣器鸣叫启动函数
*@author 	shenhy
*@param[in] _uiFreq:蜂鸣器频率
*@param[in]	_usBeepTime:蜂鸣器鸣叫时间
*@param[in]	_usStopTime:蜂鸣器空闲时间
*@param[in]	_usCycle:蜂鸣器鸣叫循环次数
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

	BEEP_ENABLE();	/* 开始发声 */		
}

/*
*@brief 	蜂鸣器鸣叫停止函数
*@author 	shenhy
*@param[in] null
*@retval	null
*/
static void BEEP_Stop(void)
{
	g_tBeep.ucEnalbe = 0;

	BEEP_DISABLE();	/* 必须在清控制标志后再停止发声，避免停止后在中断中又开启 */
}

/*
*@brief 	蜂鸣器循环处理函数
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
		if (g_tBeep.usStopTime > 0)	/* 间断发声 */
		{
			if (++g_tBeep.usCount >= g_tBeep.usBeepTime)
			{
				BEEP_DISABLE();		/* 停止发声 */
				g_tBeep.usCount = 0;
				g_tBeep.ucState = 1;
			}
		}
		else
		{
			;	/* 不做任何处理，连续发声 */
		}
	}
	else if (g_tBeep.ucState == 1)
	{
		if (++g_tBeep.usCount >= g_tBeep.usStopTime)
		{
			/* 连续发声时，直到调用stop停止为止 */
			if (g_tBeep.usCycle > 0)
			{
				if (++g_tBeep.usCycleCount >= g_tBeep.usCycle)
				{
					/* 循环次数到，停止发声 */
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

			BEEP_ENABLE();			/* 开始发声 */
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



