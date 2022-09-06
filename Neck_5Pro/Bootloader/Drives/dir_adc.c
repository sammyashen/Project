#include "dri_adc.h"

__IO uint16_t ADC1_SampleBuffer[10];						 

static void ADC1_PeriphConfig(void)
{
	GPIO_InitType GPIO_InitStructure;

	RCC_ConfigAdcHclk(RCC_ADCHCLK_DIV6);    //ADC Clock = sysclk / 6.
	
	RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA1, ENABLE);
	RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC1, ENABLE);
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_3;		
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;		
    GPIO_InitPeripheral(GPIOC, &GPIO_InitStructure);
}

static void ADC1_RegConfig(void)
{
	uint8_t i;
	uint32_t temp = 0x0001;
	uint8_t rank = 1;
	ADC_InitType ADC_InitStructure;
	DMA_InitType DMA_InitStructure;

	ADC_DeInit(ADC1);  
	/* ADC configuration */
	ADC_InitStructure.WorkMode = ADC_WORKMODE_INDEPENDENT;							
	ADC_InitStructure.MultiChEn = ENABLE;								
	ADC_InitStructure.ContinueConvEn = ENABLE;							
	ADC_InitStructure.ExtTrigSelect = ADC_EXT_TRIGCONV_NONE;
	ADC_InitStructure.DatAlign = ADC_DAT_ALIGN_R;	 				
	ADC_InitStructure.ChsNumber = ADC1_NbrOfChannel + 2;		 				
	ADC_Init(ADC1, &ADC_InitStructure);				

	/* ADC regular channel configuration */
	for (i = 0;i < 16;i++)
	{
		if (ADC1_SampleChannel & temp)
		{
			ADC_ConfigRegularChannel(ADC1, i, rank++, ADC1_SampleTime);	
		}
		temp <<= 1;	
	}

	ADC_ConfigRegularChannel(ADC1, ADC_CH_16, 6, ADC1_SampleTime);	
	ADC_ConfigRegularChannel(ADC1, ADC_CH_INT_VREF, 7, ADC_SAMP_TIME_239CYCLES5);
	//待测通道电压 = 1.20V * (待测通道AD值/内部基准通道AD值)
	
	/* Enables or disables the temperature sensor and Vrefint channel */
	ADC_EnableTempSensorVrefint(ENABLE);
	
	/* Enable ADC */
	ADC_Enable(ADC1, ENABLE);	
	/*Check ADC Ready*/
    while(ADC_GetFlagStatusNew(ADC1,ADC_FLAG_RDY) == RESET);//等待ADC模块准备好，这句不可缺少!!!
	/* Enable ADC reset calibaration register */   
//	ADC_ResetCalibration(ADC1);		 											
	/* Check the end of ADC reset calibration register */
//	while(ADC_GetResetCalibrationStatus(ADC1));	   							
	/* Start ADC calibaration */
	ADC_StartCalibration(ADC1);													
	/* Check the end of ADC calibration */
	while(ADC_GetCalibrationStatus(ADC1));	  		

	/* DMA1_channel1 configuration */
	DMA_DeInit(DMA1_CH1);	   												
	DMA_InitStructure.PeriphAddr 				= (u32)&ADC1->DAT;//ADC1_DAT_Address;					
	DMA_InitStructure.MemAddr 					= (u32)ADC1_SampleBuffer;				
	DMA_InitStructure.Direction 				= DMA_DIR_PERIPH_SRC;							
	DMA_InitStructure.BufSize 					= ADC1_SampleBufferSize + 2;					
	DMA_InitStructure.PeriphInc 				= DMA_PERIPH_INC_DISABLE;		
	DMA_InitStructure.DMA_MemoryInc 			= DMA_MEM_INC_ENABLE;						
	DMA_InitStructure.PeriphDataSize 			= DMA_PERIPH_DATA_SIZE_HALFWORD;	
	DMA_InitStructure.MemDataSize 				= DMA_MemoryDataSize_HalfWord;	   		
	DMA_InitStructure.CircularMode 				= DMA_MODE_CIRCULAR;								
	DMA_InitStructure.Priority					= DMA_PRIORITY_HIGH;							
	DMA_InitStructure.Mem2Mem 					= DMA_M2M_DISABLE;			 					
	DMA_Init(DMA1_CH1, &DMA_InitStructure);	   							
//	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
	/* Enable DMA channel1 */
	DMA_EnableChannel(DMA1_CH1, ENABLE);			                                    
	ADC_EnableDMA(ADC1, ENABLE);                                                   
	ADC_EnableSoftwareStartConv(ADC1, ENABLE);                                     
}

static void ADC2_PeriphConfig(void)
{
	GPIO_InitType GPIO_InitStructure;

	RCC_ConfigAdcHclk(RCC_ADCHCLK_DIV6);    //ADC Clock = sysclk / 6.
	
	RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC2, ENABLE);
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.Pin = GPIO_PIN_5;		
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
}

void ADC2_RegConfig(void)
{
	ADC_InitType ADC_InitStructure;

	 /* ADC configuration ------------------------------------------------------*/
    ADC_InitStructure.WorkMode       = ADC_WORKMODE_INDEPENDENT;
    ADC_InitStructure.MultiChEn      = DISABLE;
    ADC_InitStructure.ContinueConvEn = DISABLE;
    ADC_InitStructure.ExtTrigSelect  = ADC_EXT_TRIGCONV_NONE;
    ADC_InitStructure.DatAlign       = ADC_DAT_ALIGN_R;
    ADC_InitStructure.ChsNumber      = 1;
    ADC_Init(ADC2, &ADC_InitStructure);

    /* Enable ADC */
    ADC_Enable(ADC2, ENABLE);
	/*Check ADC Ready*/
    while(ADC_GetFlagStatusNew(ADC2,ADC_FLAG_RDY) == RESET);
	/* Enable ADC reset calibaration register */   
//	ADC_ResetCalibration(ADC2);		 											
	/* Check the end of ADC reset calibration register */
//	while(ADC_GetResetCalibrationStatus(ADC2));	
    /* Start ADC calibration */
    ADC_StartCalibration(ADC2);
    /* Check the end of ADC calibration */
    while (ADC_GetCalibrationStatus(ADC2));
}


void ADC1_Init(void)
{
    ADC1_PeriphConfig();
    ADC1_RegConfig();
}

void ADC2_Init(void)
{
	ADC2_PeriphConfig();
    ADC2_RegConfig();
}

uint16_t ADC2_GetData(void)
{
	uint16_t dat;
	
	ADC_ConfigRegularChannel(ADC2, ADC2_Channel_02_PA5, 1, ADC_SAMP_TIME_13CYCLES5);
	/* Start ADC Software Conversion */
	ADC_EnableSoftwareStartConv(ADC2, ENABLE);
	while(ADC_GetFlagStatus(ADC2, ADC_FLAG_ENDC)==0);
	ADC_ClearFlag(ADC2, ADC_FLAG_ENDC);
	ADC_ClearFlag(ADC2, ADC_FLAG_STR);
	dat=ADC_GetDat(ADC2);
	
	return dat;
}


uint16_t ADC_GetSample(uint8_t type)
{
    uint16_t temp = 0;
    switch (type)
    {
        case PA1_SAMPLE:
            temp = ADC1_SampleBuffer[0];
            break;
		
        case PA2_SAMPLE:
            temp = ADC1_SampleBuffer[1];
            break;
		
        case PA3_SAMPLE:
            temp = ADC1_SampleBuffer[2];
			break;
		
		case PA5_SAMPLE:
			temp =	ADC2_GetData();
			break;
			
        default:
            break;
    }
    return temp;
}


