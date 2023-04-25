#include "drv_adc.h"

static __IO uint16_t ADC1_SampleBuffer[3];		

static void ADC_PeriphConfig(void)
{
	GPIO_InitType GPIO_InitStructure;

	RCC_ConfigAdcHclk(RCC_ADCHCLK_DIV6);    
	RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA, ENABLE);
	RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC, ENABLE);
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Analog;
    GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;		
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
}

void ADC_RegConfig(void)
{
	uint8_t i;
	uint16_t temp = 0x0001;
	uint8_t rank = 1;
	ADC_InitType ADC_InitStructure;
	DMA_InitType DMA_InitStructure;

	ADC_PeriphConfig();
	ADC_DeInit(ADC);  
	/* ADC configuration */						
	ADC_InitStructure.MultiChEn = ENABLE;								
	ADC_InitStructure.ContinueConvEn = ENABLE;							
	ADC_InitStructure.ExtTrigSelect = ADC_EXT_TRIGCONV_NONE;
	ADC_InitStructure.DatAlign = ADC_DAT_ALIGN_R;	 				
	ADC_InitStructure.ChsNumber = ADC1_NbrOfChannel;		 				
	ADC_Init(ADC, &ADC_InitStructure);				

	/* ADC regular channel configuration */
	for (i = 0;i < 16;i++)
	{
		if (ADC1_SampleChannel & temp)
		{
			ADC_ConfigRegularChannel(ADC, i, rank++, ADC1_SampleTime);	
		}
		temp <<= 1;	
	}
	
	/* Enables or disables the temperature sensor and Vrefint channel */
	ADC_EnableTempSensorVrefint(ENABLE);
	
	/* Enable ADC */
	ADC_Enable(ADC, ENABLE);	
	/*Check ADC Ready*/
    while(ADC_GetFlagStatusNew(ADC,ADC_FLAG_RDY) == RESET);//等待ADC模块准备好，这句不可缺少!!! 											
	/* Start ADC calibaration */
	ADC_StartCalibration(ADC);													
	/* Check the end of ADC calibration */
	while(ADC_GetCalibrationStatus(ADC));	  		

	/* DMA1_channel1 configuration */
	DMA_DeInit(DMA_CH1);	   												
	DMA_InitStructure.PeriphAddr 				= ADC1_DAT_Address;					
	DMA_InitStructure.MemAddr 					= (u32)ADC1_SampleBuffer;				
	DMA_InitStructure.Direction 				= DMA_DIR_PERIPH_SRC;							
	DMA_InitStructure.BufSize 					= ADC1_SampleBufferSize;					
	DMA_InitStructure.PeriphInc 				= DMA_PERIPH_INC_DISABLE;		
	DMA_InitStructure.DMA_MemoryInc 			= DMA_MEM_INC_ENABLE;						
	DMA_InitStructure.PeriphDataSize 			= DMA_PERIPH_DATA_SIZE_HALFWORD;	
	DMA_InitStructure.MemDataSize 				= DMA_MemoryDataSize_HalfWord;	   		
	DMA_InitStructure.CircularMode 				= DMA_MODE_CIRCULAR;								
	DMA_InitStructure.Priority					= DMA_PRIORITY_HIGH;							
	DMA_InitStructure.Mem2Mem 					= DMA_M2M_DISABLE;			 					
	DMA_Init(DMA_CH1, &DMA_InitStructure);	   					
	DMA_RequestRemap(DMA_REMAP_ADC1, DMA, DMA_CH1, ENABLE);
	/* Enable DMA channel1 */
	DMA_EnableChannel(DMA_CH1, ENABLE);			                                    
	ADC_EnableDMA(ADC, ENABLE);                                                   
	ADC_EnableSoftwareStartConv(ADC, ENABLE);  
}

uint16_t ADC_GetSample(uint8_t type)
{
    uint16_t temp = 0;
    switch (type)
    {
		case OUT1_SAMPLE:
			temp = ADC1_SampleBuffer[1];
			break;

		case OUT2_SAMPLE:
			temp = ADC1_SampleBuffer[2];
			break;

		case VREF_SAMPLE:
			temp = ADC1_SampleBuffer[0];
			break;
			
        default:
            break;
    }

    return temp;
}

