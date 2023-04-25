#include "drv_pwm.h"

/*
*********************************************************************************************************
*	函 数 名: bsp_GetRCCofGPIO
*	功能说明: 根据GPIO 得到RCC寄存器
*	形    参：无
*	返 回 值: GPIO外设时钟名
*********************************************************************************************************
*/
static uint32_t bsp_GetRCCofGPIO(GPIO_Module* GPIOx)
{
	uint32_t rcc = 0;

	if (GPIOx == GPIOA)
	{
		rcc = RCC_APB2_PERIPH_GPIOA;
	}
	else if (GPIOx == GPIOB)
	{
		rcc = RCC_APB2_PERIPH_GPIOB;
	}
	else if (GPIOx == GPIOC)
	{
		rcc = RCC_APB2_PERIPH_GPIOC;
	}
	else if (GPIOx == GPIOD)
	{
		rcc = RCC_APB2_PERIPH_GPIOD;
	}

	return rcc;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_GetRCCofTIM
*	功能说明: 根据TIM 得到RCC寄存器
*	形    参：无
*	返 回 值: TIM外设时钟名
*********************************************************************************************************
*/
static uint32_t bsp_GetRCCofTIM(TIM_Module* TIMx)
{
	uint32_t rcc = 0;

	/*
		APB1 定时器有 TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM7, TIM12, TIM13, TIM14
		APB2 定时器有 TIM1, TIM8 ,TIM9, TIM10, TIM11
	*/
	if (TIMx == TIM1)
	{
		rcc = RCC_APB2_PERIPH_TIM1;
	}
	else if (TIMx == TIM8)
	{
		rcc = RCC_APB2_PERIPH_TIM8;
	}
	/* 下面是 APB1时钟 */
	else if (TIMx == TIM2)
	{
		rcc = RCC_APB1_PERIPH_TIM2;
	}
	else if (TIMx == TIM3)
	{
		rcc = RCC_APB1_PERIPH_TIM3;
	}
	else if (TIMx == TIM4)
	{
		rcc = RCC_APB1_PERIPH_TIM4;
	}
	else if (TIMx == TIM5)
	{
		rcc = RCC_APB1_PERIPH_TIM5;
	}
	else if (TIMx == TIM6)
	{
		rcc = RCC_APB1_PERIPH_TIM6;
	}
	else if (TIMx == TIM7)
	{
		rcc = RCC_APB1_PERIPH_TIM7;
	}
	else if (TIMx == TIM9)
	{
		rcc = RCC_APB1_PERIPH_TIM9;
	}

	return rcc;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_ConfigTimGpio
*	功能说明: 配置GPIO和TIM时钟， GPIO连接到TIM输出通道
*	形    参: GPIOx
*			 GPIO_PinX
*			 GPIO_ALTx
*			 TIMx
*			 _ucChannel
*	返 回 值: 无
*********************************************************************************************************
*/
static void bsp_ConfigTimGpio(GPIO_Module* GPIOx, uint16_t GPIO_PinX, uint32_t GPIO_ALTx, TIM_Module* TIMx)
{
	GPIO_InitType  GPIO_InitStructure;

	/* 使能GPIO时钟 */
	RCC_EnableAPB2PeriphClk(bsp_GetRCCofGPIO(GPIOx), ENABLE);

  	/* 使能TIM时钟 */
	if ((TIMx == TIM1) || (TIMx == TIM8))
	{
		RCC_EnableAPB2PeriphClk(bsp_GetRCCofTIM(TIMx), ENABLE);
	}
	else
	{
		RCC_EnableAPB1PeriphClk(bsp_GetRCCofTIM(TIMx), ENABLE);
	}
	
	/* 配置GPIO */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Alternate = GPIO_ALTx;
	GPIO_InitStructure.GPIO_Current = GPIO_DC_4mA;
    GPIO_InitStructure.Pin   = GPIO_PinX;
	GPIO_InitPeripheral(GPIOx, &GPIO_InitStructure); 
}

/*
*********************************************************************************************************
*	函 数 名: bsp_ConfigGpioOut
*	功能说明: 配置GPIO为推挽输出。主要用于PWM输出，占空比为0和100的情况。
*	形    参: GPIOx
*			  GPIO_PinX
*	返 回 值: 无
*********************************************************************************************************
*/
static void bsp_ConfigGpioOut(GPIO_Module* GPIOx, uint16_t GPIO_PinX)
{
	GPIO_InitType  GPIO_InitStructure;

	/* 使能GPIO时钟 */
	RCC_EnableAPB2PeriphClk(bsp_GetRCCofGPIO(GPIOx), ENABLE);

	/* 配置GPIO */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Current = GPIO_DC_4mA;
	GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Down;
	GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
	GPIO_InitStructure.Pin = GPIO_PinX;					/* 带入的形参 */
	GPIO_InitPeripheral(GPIOx, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
*	函 数 名: bsp_SetTIMOutPWM
*	功能说明: 设置引脚输出的PWM信号的频率和占空比.  当频率为0，并且占空为0时，关闭定时器，GPIO输出0；
*			  当频率为0，占空比为100%时，GPIO输出1.
*	形    参: _ulFreq : PWM信号频率，单位Hz  (实际测试，最大输出频率为 168M / 4 = 42M）. 0 表示禁止输出
*			  _ulDutyCycle : PWM信号占空比，单位：万分之一。如5000，表示50.00%的占空比
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_SetTIMOutPWM(GPIO_Module* GPIOx, uint16_t GPIO_Pin, uint32_t GPIO_ALTx, TIM_Module* TIMx, uint8_t _ucChannel,
	 uint32_t _ulFreq, uint32_t _ulDutyCycle)
{
	TIM_TimeBaseInitType  	TIM_TimeBaseStructure;
	OCInitType				TIM_OCInitStructure;
	RCC_ClocksType			RCC_ClockFreq;
	uint16_t usPeriod;
	uint16_t usPrescaler;
	uint32_t uiTIMxCLK;

	if (_ulDutyCycle == 0)
	{		
		TIM_EnableCapCmpCh(TIMx, _ucChannel, TIM_CAP_CMP_DISABLE);
		bsp_ConfigGpioOut(GPIOx, GPIO_Pin);
		GPIO_WriteBit(GPIOx, GPIO_Pin, Bit_RESET);
		return;
	}
	else if (_ulDutyCycle == 10000)
	{
		TIM_EnableCapCmpCh(TIMx, _ucChannel, TIM_CAP_CMP_DISABLE);
		bsp_ConfigGpioOut(GPIOx, GPIO_Pin);	
		GPIO_WriteBit(GPIOx, GPIO_Pin, Bit_SET);
		return;
	}
	
	RCC_GetClocksFreqValue(&RCC_ClockFreq);
	bsp_ConfigTimGpio(GPIOx, GPIO_Pin, GPIO_ALTx, TIMx);	
	
	if ((TIMx == TIM1) || (TIMx == TIM8))
	{
		/* APB2 定时器 */
		uiTIMxCLK = RCC_ClockFreq.SysclkFreq;
	}
	else	/* APB1 定时器 */
	{
		uiTIMxCLK = RCC_ClockFreq.SysclkFreq / 2;	
	}

	if (_ulFreq < 100)
	{
		usPrescaler = 10000 - 1;					
		usPeriod =  (uiTIMxCLK / 10000) / _ulFreq  - 1;		/* 自动重装的值 */
	}
	else if (_ulFreq < 3000)
	{
		usPrescaler = 100 - 1;								/* 分频比 = 100 */
		usPeriod =  (uiTIMxCLK / 100) / _ulFreq  - 1;		/* 自动重装的值 */
	}
	else													/* 大于4K的频率，无需分频 */
	{
		usPrescaler = 0;									/* 分频比 = 1 */
		usPeriod = uiTIMxCLK / _ulFreq - 1;					/* 自动重装的值 */
	}

	/* Time base configuration */
	TIM_TimeBaseStructure.Period = usPeriod;
	TIM_TimeBaseStructure.Prescaler = usPrescaler;
	TIM_TimeBaseStructure.ClkDiv = 0;
	TIM_TimeBaseStructure.CntMode = TIM_CNT_MODE_DOWN;		//消除脉冲叠加	
	TIM_TimeBaseStructure.RepetCnt = 0;	
	TIM_InitTimeBase(TIMx, &TIM_TimeBaseStructure);
	
	TIM_OCInitStructure.OcMode = TIM_OCMODE_PWM1;
	TIM_OCInitStructure.Pulse = (_ulDutyCycle * usPeriod) / 10000;
	
	TIM_OCInitStructure.OcPolarity = TIM_OC_POLARITY_HIGH;
	TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
	TIM_OCInitStructure.OcIdleState = TIM_OC_IDLE_STATE_RESET;

	TIM_OCInitStructure.OcNPolarity = TIM_OCN_POLARITY_HIGH;
	TIM_OCInitStructure.OutputNState = TIM_OUTPUT_NSTATE_DISABLE;		/* only for TIM1 and TIM8. */		
	TIM_OCInitStructure.OcNIdleState = TIM_OCN_IDLE_STATE_RESET;		/* only for TIM1 and TIM8. */
	
	if (_ucChannel == TIM_CH_1)
	{
		TIM_InitOc1(TIMx, &TIM_OCInitStructure);
		TIM_ConfigOc1Preload(TIMx, TIM_OC_PRE_LOAD_ENABLE);
	}
	else if (_ucChannel == TIM_CH_2)
	{
		TIM_InitOc2(TIMx, &TIM_OCInitStructure);
		TIM_ConfigOc2Preload(TIMx, TIM_OC_PRE_LOAD_ENABLE);
	}
	else if (_ucChannel == TIM_CH_3)
	{
		TIM_InitOc3(TIMx, &TIM_OCInitStructure);
		TIM_ConfigOc3Preload(TIMx, TIM_OC_PRE_LOAD_ENABLE);
	}
	else if (_ucChannel == TIM_CH_4)
	{
		TIM_InitOc4(TIMx, &TIM_OCInitStructure);
		TIM_ConfigOc4Preload(TIMx, TIM_OC_PRE_LOAD_ENABLE);
	}

	TIM_ConfigArPreload(TIMx, ENABLE);

	/* TIMx enable counter */
	TIM_Enable(TIMx, ENABLE);

	/* 下面这句话对于TIM1和TIM8是必须的，对于TIM2-TIM6则不必要 */
	if ((TIMx == TIM1) || (TIMx == TIM8))
	{
		TIM_EnableCtrlPwmOutputs(TIMx, ENABLE);
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_SetTIMOutPWM_N
*	功能说明: 设置TIM8_CH1N 等反相引脚输出的PWM信号的频率和占空比.  当频率为0，并且占空为0时，关闭定时器，GPIO输出0；
*			  当频率为0，占空比为100%时，GPIO输出1.
*	形    参: _ulFreq : PWM信号频率，单位Hz  (实际测试，最大输出频率为 168M / 4 = 42M）. 0 表示禁止输出
*			  _ulDutyCycle : PWM信号占空比，单位：万分之一。如5000，表示50.00%的占空比
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_SetTIMOutPWM_N(GPIO_Module* GPIOx, uint16_t GPIO_Pin, uint32_t GPIO_ALTx, TIM_Module* TIMx, uint8_t _ucChannel,
	 uint32_t _ulFreq, uint32_t _ulDutyCycle)
{
	TIM_TimeBaseInitType  	TIM_TimeBaseStructure;
	OCInitType  			TIM_OCInitStructure;
	RCC_ClocksType 			RCC_ClockFreq;
	uint16_t usPeriod;
	uint16_t usPrescaler;
	uint32_t uiTIMxCLK;

	if (_ulDutyCycle == 0)
	{		
		TIM_EnableCapCmpCh(TIMx, _ucChannel, TIM_CAP_CMP_N_DISABLE);
		bsp_ConfigGpioOut(GPIOx, GPIO_Pin);
		GPIO_WriteBit(GPIOx, GPIO_Pin, Bit_RESET);
		return;
	}
	else if (_ulDutyCycle == 10000)
	{
		TIM_EnableCapCmpCh(TIMx, _ucChannel, TIM_CAP_CMP_N_DISABLE);
		bsp_ConfigGpioOut(GPIOx, GPIO_Pin);	
		GPIO_WriteBit(GPIOx, GPIO_Pin, Bit_SET);
		return;
	}
	
	RCC_GetClocksFreqValue(&RCC_ClockFreq);
	bsp_ConfigTimGpio(GPIOx, GPIO_Pin, GPIO_ALTx, TIMx);	
	
	if ((TIMx == TIM1) || (TIMx == TIM8))
	{
		/* APB2 定时器 */
		uiTIMxCLK = RCC_ClockFreq.SysclkFreq;
	}
	else	/* APB1 定时器 */
	{
		uiTIMxCLK = RCC_ClockFreq.SysclkFreq / 2;	
	}

	if (_ulFreq < 100)
	{
		usPrescaler = 10000 - 1;								
		usPeriod =  (uiTIMxCLK / 10000) / _ulFreq  - 1;		
	}
	else if (_ulFreq < 3000)
	{
		usPrescaler = 100 - 1;					
		usPeriod =  (uiTIMxCLK / 100) / _ulFreq  - 1;		
	}
	else	
	{
		usPrescaler = 0;					
		usPeriod = uiTIMxCLK / _ulFreq - 1;	
	}

	/* Time base configuration */
	TIM_TimeBaseStructure.Period = usPeriod;
	TIM_TimeBaseStructure.Prescaler = usPrescaler;
	TIM_TimeBaseStructure.ClkDiv = 0;
	TIM_TimeBaseStructure.CntMode = TIM_CNT_MODE_DOWN;
	TIM_TimeBaseStructure.RepetCnt = 0;	
	TIM_InitTimeBase(TIMx, &TIM_TimeBaseStructure);	
	
	TIM_OCInitStructure.OcMode = TIM_OCMODE_PWM1;
	TIM_OCInitStructure.Pulse = (_ulDutyCycle * usPeriod) / 10000;
	
	TIM_OCInitStructure.OcPolarity = TIM_OC_POLARITY_HIGH;
	TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_DISABLE;	
	TIM_OCInitStructure.OcIdleState = TIM_OC_IDLE_STATE_RESET;
			
	TIM_OCInitStructure.OcNPolarity = TIM_OCN_POLARITY_HIGH;	
	TIM_OCInitStructure.OutputNState = TIM_OUTPUT_NSTATE_ENABLE;
	TIM_OCInitStructure.OcNIdleState = TIM_OCN_IDLE_STATE_RESET;		
	
	if (_ucChannel == TIM_CH_1)
	{
		TIM_InitOc1(TIMx, &TIM_OCInitStructure);
		TIM_ConfigOc1Preload(TIMx, TIM_OC_PRE_LOAD_ENABLE);
	}
	else if (_ucChannel == TIM_CH_2)
	{
		TIM_InitOc2(TIMx, &TIM_OCInitStructure);
		TIM_ConfigOc2Preload(TIMx, TIM_OC_PRE_LOAD_ENABLE);
	}
	else if (_ucChannel == TIM_CH_3)
	{
		TIM_InitOc3(TIMx, &TIM_OCInitStructure);
		TIM_ConfigOc3Preload(TIMx, TIM_OC_PRE_LOAD_ENABLE);
	}
	else if (_ucChannel == TIM_CH_4)
	{
		TIM_InitOc4(TIMx, &TIM_OCInitStructure);
		TIM_ConfigOc4Preload(TIMx, TIM_OC_PRE_LOAD_ENABLE);
	}

	TIM_ConfigArPreload(TIMx, ENABLE);

	/* TIMx enable counter */
	TIM_Enable(TIMx, ENABLE);

	/* 下面这句话对于TIM1和TIM8是必须的，对于TIM2-TIM6则不必要 */
	if ((TIMx == TIM1) || (TIMx == TIM8))
	{
		TIM_EnableCtrlPwmOutputs(TIMx, ENABLE);
	}
}




