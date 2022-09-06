#include "drv_pwm.h"

/*
*********************************************************************************************************
*	�� �� ��: bsp_GetRCCofGPIO
*	����˵��: ����GPIO �õ�RCC�Ĵ���
*	��    �Σ���
*	�� �� ֵ: GPIO����ʱ����
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
	else if (GPIOx == GPIOE)
	{
		rcc = RCC_APB2_PERIPH_GPIOE;
	}
	else if (GPIOx == GPIOF)
	{
		rcc = RCC_APB2_PERIPH_GPIOF;
	}
	else if (GPIOx == GPIOG)
	{
		rcc = RCC_APB2_PERIPH_GPIOG;
	}

	return rcc;
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_GetRCCofTIM
*	����˵��: ����TIM �õ�RCC�Ĵ���
*	��    �Σ���
*	�� �� ֵ: TIM����ʱ����
*********************************************************************************************************
*/
static uint32_t bsp_GetRCCofTIM(TIM_Module* TIMx)
{
	uint32_t rcc = 0;

	/*
		APB1 ��ʱ���� TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM7, TIM12, TIM13, TIM14
		APB2 ��ʱ���� TIM1, TIM8 ,TIM9, TIM10, TIM11
	*/
	if (TIMx == TIM1)
	{
		rcc = RCC_APB2_PERIPH_TIM1;
	}
	else if (TIMx == TIM8)
	{
		rcc = RCC_APB2_PERIPH_TIM8;
	}
//	else if (TIMx == TIM9)
//	{
//		rcc = RCC_APB2Periph_TIM9;
//	}
//	else if (TIMx == TIM10)
//	{
//		rcc = RCC_APB2Periph_TIM10;
//	}
//	else if (TIMx == TIM11)
//	{
//		rcc = RCC_APB2Periph_TIM11;
//	}
	/* ������ APB1ʱ�� */
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
//	else if (TIMx == TIM12)
//	{
//		rcc = RCC_APB1Periph_TIM12;
//	}
//	else if (TIMx == TIM13)
//	{
//		rcc = RCC_APB1Periph_TIM13;
//	}
//	else if (TIMx == TIM14)
//	{
//		rcc = RCC_APB1Periph_TIM14;
//	}

	return rcc;
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_ConfigTimGpio
*	����˵��: ����GPIO��TIMʱ�ӣ� GPIO���ӵ�TIM���ͨ��
*	��    ��: GPIOx
*			 GPIO_PinX
*			 TIMx
*			 _ucChannel
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void bsp_ConfigTimGpio(GPIO_Module* GPIOx, uint16_t GPIO_PinX, TIM_Module* TIMx, uint8_t _ucChannel)
{
	GPIO_InitType  GPIO_InitStructure;

	/* ʹ��GPIOʱ�� */
	RCC_EnableAPB2PeriphClk(bsp_GetRCCofGPIO(GPIOx), ENABLE);

  	/* ʹ��TIMʱ�� */
	if ((TIMx == TIM1) || (TIMx == TIM8))
	{
		RCC_EnableAPB2PeriphClk(bsp_GetRCCofTIM(TIMx), ENABLE);
	}
	else
	{
		RCC_EnableAPB1PeriphClk(bsp_GetRCCofTIM(TIMx), ENABLE);
	}
	RCC_EnableAPB1PeriphClk(RCC_APB2_PERIPH_AFIO, ENABLE);

	/* ����GPIO */
	GPIO_InitStructure.Pin = GPIO_PinX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		/* ���ù��� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitPeripheral(GPIOx, &GPIO_InitStructure); 
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_ConfigGpioOut
*	����˵��: ����GPIOΪ�����������Ҫ����PWM�����ռ�ձ�Ϊ0��100�������
*	��    ��: GPIOx
*			  GPIO_PinX
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void bsp_ConfigGpioOut(GPIO_Module* GPIOx, uint16_t GPIO_PinX)
{
	GPIO_InitType  GPIO_InitStructure;

	/* ʹ��GPIOʱ�� */
	RCC_EnableAPB2PeriphClk(bsp_GetRCCofGPIO(GPIOx), ENABLE);

	/* ����GPIO */
	GPIO_InitStructure.Pin = GPIO_PinX;					/* ������β� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	/* ��� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitPeripheral(GPIOx, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_SetTIMOutPWM
*	����˵��: �������������PWM�źŵ�Ƶ�ʺ�ռ�ձ�.  ��Ƶ��Ϊ0������ռ��Ϊ0ʱ���رն�ʱ����GPIO���0��
*			  ��Ƶ��Ϊ0��ռ�ձ�Ϊ100%ʱ��GPIO���1.
*	��    ��: _ulFreq : PWM�ź�Ƶ�ʣ���λHz  (ʵ�ʲ��ԣ�������Ƶ��Ϊ 168M / 4 = 42M��. 0 ��ʾ��ֹ���
*			  _ulDutyCycle : PWM�ź�ռ�ձȣ���λ�����֮һ����5000����ʾ50.00%��ռ�ձ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_SetTIMOutPWM(GPIO_Module* GPIOx, uint16_t GPIO_Pin, TIM_Module* TIMx, uint8_t _ucChannel,
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
//		TIM_Enable(TIMx, DISABLE);							/* �ر�PWM��� */
//		bsp_ConfigGpioOut(GPIOx, GPIO_Pin);					/* ����GPIOΪ������� */		
//		GPIO_WriteBit(GPIOx, GPIO_Pin, Bit_RESET);			/* PWM = 0 */		
//		return;

//		if(_ucChannel == 1)			TIM_ConfigOc1Preload(TIMx, TIM_OC_PRE_LOAD_DISABLE);
//		else if(_ucChannel == 2)	TIM_ConfigOc2Preload(TIMx, TIM_OC_PRE_LOAD_DISABLE);
//		else if(_ucChannel == 3)	TIM_ConfigOc3Preload(TIMx, TIM_OC_PRE_LOAD_DISABLE);
//		else if(_ucChannel == 4)	TIM_ConfigOc4Preload(TIMx, TIM_OC_PRE_LOAD_DISABLE);

		TIM_EnableCapCmpCh(TIMx, _ucChannel, TIM_CAP_CMP_DISABLE);
		bsp_ConfigGpioOut(GPIOx, GPIO_Pin);
		GPIO_WriteBit(GPIOx, GPIO_Pin, Bit_RESET);
		return;
	}
	else if (_ulDutyCycle == 10000)
	{
//		TIM_Enable(TIMx, DISABLE);							/* �ر�PWM��� */
//		bsp_ConfigGpioOut(GPIOx, GPIO_Pin);					/* ����GPIOΪ������� */		
//		GPIO_WriteBit(GPIOx, GPIO_Pin, Bit_SET);			/* PWM = 1 */	
//		return;

//		if(_ucChannel == 1)			TIM_ConfigOc1Preload(TIMx, TIM_OC_PRE_LOAD_DISABLE);
//		else if(_ucChannel == 2)	TIM_ConfigOc2Preload(TIMx, TIM_OC_PRE_LOAD_DISABLE);
//		else if(_ucChannel == 3)	TIM_ConfigOc3Preload(TIMx, TIM_OC_PRE_LOAD_DISABLE);
//		else if(_ucChannel == 4)	TIM_ConfigOc4Preload(TIMx, TIM_OC_PRE_LOAD_DISABLE);

		TIM_EnableCapCmpCh(TIMx, _ucChannel, TIM_CAP_CMP_DISABLE);
		bsp_ConfigGpioOut(GPIOx, GPIO_Pin);	
		GPIO_WriteBit(GPIOx, GPIO_Pin, Bit_SET);
		return;
	}
	
	RCC_GetClocksFreqValue(&RCC_ClockFreq);
	bsp_ConfigTimGpio(GPIOx, GPIO_Pin, TIMx, _ucChannel);	/* ʹ��GPIO��TIMʱ�ӣ�������TIMͨ����GPIO */
	
	if ((TIMx == TIM1) || (TIMx == TIM8))// || (TIMx == TIM9) || (TIMx == TIM10) || (TIMx == TIM11))
	{
		/* APB2 ��ʱ�� */
		uiTIMxCLK = RCC_ClockFreq.SysclkFreq;
	}
	else	/* APB1 ��ʱ�� */
	{
		uiTIMxCLK = RCC_ClockFreq.SysclkFreq;	
	}

	if (_ulFreq < 100)
	{
		usPrescaler = 10000 - 1;					
		usPeriod =  (uiTIMxCLK / 10000) / _ulFreq  - 1;		/* �Զ���װ��ֵ */
	}
	else if (_ulFreq < 3000)
	{
		usPrescaler = 100 - 1;								/* ��Ƶ�� = 100 */
		usPeriod =  (uiTIMxCLK / 100) / _ulFreq  - 1;		/* �Զ���װ��ֵ */
	}
	else													/* ����4K��Ƶ�ʣ������Ƶ */
	{
		usPrescaler = 0;									/* ��Ƶ�� = 1 */
		usPeriod = uiTIMxCLK / _ulFreq - 1;					/* �Զ���װ��ֵ */
	}

	/* Time base configuration */
	TIM_TimeBaseStructure.Period = usPeriod;
	TIM_TimeBaseStructure.Prescaler = usPrescaler;
	TIM_TimeBaseStructure.ClkDiv = 0;
	TIM_TimeBaseStructure.CntMode = TIM_CNT_MODE_DOWN;		//�����������	
	TIM_TimeBaseStructure.RepetCnt = 0;	
	TIM_InitTimeBase(TIMx, &TIM_TimeBaseStructure);
	
	TIM_OCInitStructure.OcMode = TIM_OCMODE_PWM1;
	TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
	TIM_OCInitStructure.Pulse = (_ulDutyCycle * usPeriod) / 10000;
	TIM_OCInitStructure.OcPolarity = TIM_OC_POLARITY_HIGH;
	
	TIM_OCInitStructure.OutputNState = TIM_OUTPUT_NSTATE_DISABLE;		/* only for TIM1 and TIM8. */	
	TIM_OCInitStructure.OcNPolarity = TIM_OCN_POLARITY_HIGH;			/* only for TIM1 and TIM8. */		
	TIM_OCInitStructure.OcIdleState = TIM_OC_IDLE_STATE_RESET;			/* only for TIM1 and TIM8. */
	TIM_OCInitStructure.OcNIdleState = TIM_OCN_IDLE_STATE_RESET;		/* only for TIM1 and TIM8. */
	
	if (_ucChannel == 1)
	{
		TIM_InitOc1(TIMx, &TIM_OCInitStructure);
		TIM_ConfigOc1Preload(TIMx, TIM_OC_PRE_LOAD_ENABLE);
	}
	else if (_ucChannel == 2)
	{
		TIM_InitOc2(TIMx, &TIM_OCInitStructure);
		TIM_ConfigOc2Preload(TIMx, TIM_OC_PRE_LOAD_ENABLE);
	}
	else if (_ucChannel == 3)
	{
		TIM_InitOc3(TIMx, &TIM_OCInitStructure);
		TIM_ConfigOc3Preload(TIMx, TIM_OC_PRE_LOAD_ENABLE);
	}
	else if (_ucChannel == 4)
	{
		TIM_InitOc4(TIMx, &TIM_OCInitStructure);
		TIM_ConfigOc4Preload(TIMx, TIM_OC_PRE_LOAD_ENABLE);
	}

	TIM_ConfigArPreload(TIMx, ENABLE);

	/* TIMx enable counter */
	TIM_Enable(TIMx, ENABLE);

	/* ������仰����TIM1��TIM8�Ǳ���ģ�����TIM2-TIM6�򲻱�Ҫ */
	if ((TIMx == TIM1) || (TIMx == TIM8))
	{
		TIM_EnableCtrlPwmOutputs(TIMx, ENABLE);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_SetTIMOutPWM_N
*	����˵��: ����TIM8_CH1N �ȷ������������PWM�źŵ�Ƶ�ʺ�ռ�ձ�.  ��Ƶ��Ϊ0������ռ��Ϊ0ʱ���رն�ʱ����GPIO���0��
*			  ��Ƶ��Ϊ0��ռ�ձ�Ϊ100%ʱ��GPIO���1.
*	��    ��: _ulFreq : PWM�ź�Ƶ�ʣ���λHz  (ʵ�ʲ��ԣ�������Ƶ��Ϊ 168M / 4 = 42M��. 0 ��ʾ��ֹ���
*			  _ulDutyCycle : PWM�ź�ռ�ձȣ���λ�����֮һ����5000����ʾ50.00%��ռ�ձ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_SetTIMOutPWM_N(GPIO_Module* GPIOx, uint16_t GPIO_Pin, TIM_Module* TIMx, uint8_t _ucChannel,
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
//		if(_ucChannel == 1)			TIM_ConfigOc1Preload(TIMx, TIM_OC_PRE_LOAD_DISABLE);
//		else if(_ucChannel == 2)	TIM_ConfigOc2Preload(TIMx, TIM_OC_PRE_LOAD_DISABLE);
//		else if(_ucChannel == 3)	TIM_ConfigOc3Preload(TIMx, TIM_OC_PRE_LOAD_DISABLE);
//		else if(_ucChannel == 4)	TIM_ConfigOc4Preload(TIMx, TIM_OC_PRE_LOAD_DISABLE);

		TIM_EnableCapCmpCh(TIMx, _ucChannel, TIM_CAP_CMP_N_DISABLE);
		bsp_ConfigGpioOut(GPIOx, GPIO_Pin);
		GPIO_WriteBit(GPIOx, GPIO_Pin, Bit_RESET);
		return;
	}
	else if (_ulDutyCycle == 10000)
	{
//		if(_ucChannel == 1)			TIM_ConfigOc1Preload(TIMx, TIM_OC_PRE_LOAD_DISABLE);
//		else if(_ucChannel == 2)	TIM_ConfigOc2Preload(TIMx, TIM_OC_PRE_LOAD_DISABLE);
//		else if(_ucChannel == 3)	TIM_ConfigOc3Preload(TIMx, TIM_OC_PRE_LOAD_DISABLE);
//		else if(_ucChannel == 4)	TIM_ConfigOc4Preload(TIMx, TIM_OC_PRE_LOAD_DISABLE);

		TIM_EnableCapCmpCh(TIMx, _ucChannel, TIM_CAP_CMP_N_DISABLE);
		bsp_ConfigGpioOut(GPIOx, GPIO_Pin);	
		GPIO_WriteBit(GPIOx, GPIO_Pin, Bit_SET);
		return;
	}
	
	RCC_GetClocksFreqValue(&RCC_ClockFreq);
	bsp_ConfigTimGpio(GPIOx, GPIO_Pin, TIMx, _ucChannel);	/* ʹ��GPIO��TIMʱ�ӣ�������TIMͨ����GPIO */
	
	if ((TIMx == TIM1) || (TIMx == TIM8))// || (TIMx == TIM9) || (TIMx == TIM10) || (TIMx == TIM11))
	{
		/* APB2 ��ʱ�� */
		uiTIMxCLK = RCC_ClockFreq.SysclkFreq;
	}
	else	/* APB1 ��ʱ�� */
	{
		uiTIMxCLK = RCC_ClockFreq.SysclkFreq;	
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
	TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_DISABLE;	
	TIM_OCInitStructure.Pulse = (_ulDutyCycle * usPeriod) / 10000;
	TIM_OCInitStructure.OcPolarity = TIM_OC_POLARITY_HIGH;
	
	TIM_OCInitStructure.OutputNState = TIM_OUTPUT_NSTATE_ENABLE;		
	TIM_OCInitStructure.OcNPolarity = TIM_OCN_POLARITY_HIGH;				
	TIM_OCInitStructure.OcIdleState = TIM_OC_IDLE_STATE_RESET;		
	TIM_OCInitStructure.OcNIdleState = TIM_OCN_IDLE_STATE_RESET;		
	
	if (_ucChannel == 1)
	{
		TIM_InitOc1(TIMx, &TIM_OCInitStructure);
		TIM_ConfigOc1Preload(TIMx, TIM_OC_PRE_LOAD_ENABLE);
	}
	else if (_ucChannel == 2)
	{
		TIM_InitOc2(TIMx, &TIM_OCInitStructure);
		TIM_ConfigOc2Preload(TIMx, TIM_OC_PRE_LOAD_ENABLE);
	}
	else if (_ucChannel == 3)
	{
		TIM_InitOc3(TIMx, &TIM_OCInitStructure);
		TIM_ConfigOc3Preload(TIMx, TIM_OC_PRE_LOAD_ENABLE);
	}
	else if (_ucChannel == 4)
	{
		TIM_InitOc4(TIMx, &TIM_OCInitStructure);
		TIM_ConfigOc4Preload(TIMx, TIM_OC_PRE_LOAD_ENABLE);
	}

	TIM_ConfigArPreload(TIMx, ENABLE);

	/* TIMx enable counter */
	TIM_Enable(TIMx, ENABLE);

	/* ������仰����TIM1��TIM8�Ǳ���ģ�����TIM2-TIM6�򲻱�Ҫ */
	if ((TIMx == TIM1) || (TIMx == TIM8))
	{
		TIM_EnableCtrlPwmOutputs(TIMx, ENABLE);
	}
}

/*
*********************************************************************************************************
*	 �� �� ��: bsp_SetTIMforInt
*	 ����˵��: ����TIM��NVIC�����ڼ򵥵Ķ�ʱ�ж�. ������ʱ�жϡ� �жϷ��������Ӧ�ó���ʵ�֡�
*	 ��	��: TIMx : ��ʱ��
*			   _ulFreq : ��ʱƵ�� ��Hz���� 0 ��ʾ�رա�
*			   _PreemptionPriority : �ж����ȼ�����
*			   _SubPriority : �����ȼ�
*	 �� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_SetTIMforInt(TIM_Module* TIMx, uint32_t _ulFreq, uint8_t _PreemptionPriority, uint8_t _SubPriority)
{
	TIM_TimeBaseInitType  	TIM_TimeBaseStructure;
	RCC_ClocksType 			RCC_ClockFreq;
	uint16_t usPeriod;
	uint16_t usPrescaler;
	uint32_t uiTIMxCLK;

	/* ʹ��TIMʱ�� */
	if ((TIMx == TIM1) || (TIMx == TIM8))
	{
		RCC_EnableAPB2PeriphClk(bsp_GetRCCofTIM(TIMx), ENABLE);
	}
	else
	{
		RCC_EnableAPB1PeriphClk(bsp_GetRCCofTIM(TIMx), ENABLE);
	}

	if (_ulFreq == 0)
	{
	 	TIM_Enable(TIMx, DISABLE);	 /* �رն�ʱ��� */

	 	/* �ر�TIM��ʱ�����ж� (Update) */
	 	{
		 	NVIC_InitType NVIC_InitStructure;	 /* �жϽṹ���� misc.h �ж��� */
		 	uint8_t irq = 0;	 

		 	if (TIMx == TIM1)
			 	irq = TIM1_UP_IRQn;
			else if (TIMx == TIM2)
				irq = TIM2_IRQn;
			else if (TIMx == TIM3)
				irq = TIM3_IRQn;
			else if (TIMx == TIM4)
				irq = TIM4_IRQn;
			else if (TIMx == TIM5)
				irq = TIM5_IRQn;
			else if (TIMx == TIM6)
				irq = TIM6_IRQn;

		 	NVIC_InitStructure.NVIC_IRQChannel = irq;
		 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = _PreemptionPriority;
		 	NVIC_InitStructure.NVIC_IRQChannelSubPriority = _SubPriority;
		 	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
		 	NVIC_Init(&NVIC_InitStructure);
	 	}	
	 	
	 	return;
	}

	RCC_GetClocksFreqValue(&RCC_ClockFreq);
	if ((TIMx == TIM1) || (TIMx == TIM8))
	{
		/* APB2 ��ʱ�� */
		uiTIMxCLK = RCC_ClockFreq.SysclkFreq;
	}
	else	 /* APB1 ��ʱ�� .  */
	{
	 	uiTIMxCLK = RCC_ClockFreq.SysclkFreq;	 
	}

	if (_ulFreq < 100)
	{
		 usPrescaler = 10000 - 1;					 /* ��Ƶ�� = 1000 */
		 usPeriod =  (uiTIMxCLK / 10000) / _ulFreq	- 1;	 /* �Զ���װ��ֵ */
	}
	else if (_ulFreq < 3000)
	{
	 	usPrescaler = 100 - 1; 				 /* ��Ƶ�� = 100 */
	 	usPeriod =  (uiTIMxCLK / 100) / _ulFreq  - 1;		 /* �Զ���װ��ֵ */
	}
	else	 /* ����4K��Ƶ�ʣ������Ƶ */
	{
	 	usPrescaler = 0;					 /* ��Ƶ�� = 1 */
	 	usPeriod = uiTIMxCLK / _ulFreq - 1; /* �Զ���װ��ֵ */
	}

	 /* Time base configuration */
	 TIM_TimeBaseStructure.Period = usPeriod;
	 TIM_TimeBaseStructure.Prescaler = usPrescaler;
	 TIM_TimeBaseStructure.ClkDiv = 0;
	 TIM_TimeBaseStructure.CntMode = TIM_CNT_MODE_UP;
	 TIM_TimeBaseStructure.RepetCnt = 0;

	 TIM_InitTimeBase(TIMx, &TIM_TimeBaseStructure);

	 TIM_ConfigArPreload(TIMx, ENABLE);

	 /* TIM Interrupts enable */
	 TIM_ConfigInt(TIMx, TIM_INT_UPDATE, ENABLE);

	 /* TIMx enable counter */
	 TIM_Enable(TIMx, ENABLE);

	 /* ����TIM��ʱ�����ж� (Update) */
	 {
		 NVIC_InitType NVIC_InitStructure;	 /* �жϽṹ���� misc.h �ж��� */
		 uint8_t irq = 0;	 

		 if (TIMx == TIM1)
			 irq = TIM1_UP_IRQn;
		 else if (TIMx == TIM2)
			 irq = TIM2_IRQn;
		 else if (TIMx == TIM3)
			 irq = TIM3_IRQn;
		 else if (TIMx == TIM4)
			 irq = TIM4_IRQn;
		 else if (TIMx == TIM5)
			 irq = TIM5_IRQn;
		 else if (TIMx == TIM6)
			 irq = TIM6_IRQn;

		 NVIC_InitStructure.NVIC_IRQChannel = irq;
		 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = _PreemptionPriority;
		 NVIC_InitStructure.NVIC_IRQChannelSubPriority = _SubPriority;
		 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		 NVIC_Init(&NVIC_InitStructure);
	 }
}




