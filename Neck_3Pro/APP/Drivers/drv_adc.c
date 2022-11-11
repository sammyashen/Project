#include "drv_adc.h"
#include "tiny_os.h"

__IO uint16_t ADC1_SampleBuffer[10];		

static void ADC_PeriphConfig(void)
{
	GPIO_InitType GPIO_InitStructure;

	RCC_ConfigAdcHclk(RCC_ADCHCLK_DIV6);    
	RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA, ENABLE);
	RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC, ENABLE);
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Analog;
    GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;		
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1;		
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
}

static void ADC_RegConfig(void)
{
	uint8_t i;
	uint16_t temp = 0x0001;
	uint8_t rank = 1;
	ADC_InitType ADC_InitStructure;
	DMA_InitType DMA_InitStructure;

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
	/* Enable ADC reset calibaration register */   
//	ADC_ResetCalibration(ADC1);		 											
	/* Check the end of ADC reset calibration register */
//	while(ADC_GetResetCalibrationStatus(ADC1));	  
	/* enable Adc calibration bypass. */
//	ADC_SetBypassCalibration(ADC1, DISABLE);
	/* Start ADC calibaration */
	ADC_StartCalibration(ADC);													
	/* Check the end of ADC calibration */
	while(ADC_GetCalibrationStatus(ADC));	  		

	/* DMA1_channel1 configuration */
	DMA_DeInit(DMA_CH3);	   												
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
	DMA_Init(DMA_CH3, &DMA_InitStructure);	   					
	DMA_RequestRemap(DMA_REMAP_ADC1, DMA, DMA_CH3, ENABLE);
//	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
	/* Enable DMA channel1 */
	DMA_EnableChannel(DMA_CH3, ENABLE);			                                    
	ADC_EnableDMA(ADC, ENABLE);                                                   
	ADC_EnableSoftwareStartConv(ADC, ENABLE);                                     
}

volatile static avgf_t avg_filter_para[9] = 
{
	{2048, 2048, {0}, 0, 0, 0.2},				//ntc
	{4095, 4095, {0}, 0, 0, 0.3},				//bat1val
	{4095, 4095, {0}, 0, 0, 0.3},				//bat2val
	{2048, 2048, {0}, 0, 0, 0.2},				//bat1temp
	{2048, 2048, {0}, 0, 0, 0.2},				//bat2temp
	{2048, 2048, {0}, 0, 0, 0.8},				//m1val
	{2048, 2048, {0}, 0, 0, 0.8},				//m2val
	{2048, 2048, {0}, 0, 0, 0.8},				//m3val
	{1489, 1489, {0}, 0, 0, 0.5},				//vref
};
static void adc_task_cb(void *para, uint32_t evt)
{
	uint16_t temp;

	for(uint8_t i=0;i<9;i++){
		if(avg_filter_para[i].cnt >= 10){
			avg_filter_para[i].cnt = 0;
			//排序
			for(uint8_t m=0;m<9;m++){
				for(uint8_t n=0;n<(9-m);n++){
					if(avg_filter_para[i].buff[n] > avg_filter_para[i].buff[n+1]){
						temp = avg_filter_para[i].buff[n];
						avg_filter_para[i].buff[n] = avg_filter_para[i].buff[n+1];
						avg_filter_para[i].buff[n+1] = temp;
					}
				}
			}
			//算术平均
			avg_filter_para[i].sum = 0;
			for(uint8_t j=1;j<9;j++){
				avg_filter_para[i].sum += avg_filter_para[i].buff[j];
			}
			avg_filter_para[i].avg = avg_filter_para[i].sum >> 3;
			//一阶低通
			avg_filter_para[i].avg = (uint16_t)(avg_filter_para[i].alpha * (float)avg_filter_para[i].avg + 
						(1 - avg_filter_para[i].alpha) * (float)avg_filter_para[i].avg_1);
			avg_filter_para[i].avg_1 = avg_filter_para[i].avg;
		}else{
			if(i == 0)			avg_filter_para[i].buff[avg_filter_para[i].cnt++] = ADC1_SampleBuffer[6];
			else if(i == 1)		avg_filter_para[i].buff[avg_filter_para[i].cnt++] = ADC1_SampleBuffer[1];
			else if(i == 2)		avg_filter_para[i].buff[avg_filter_para[i].cnt++] = ADC1_SampleBuffer[2];
			else if(i == 3)		avg_filter_para[i].buff[avg_filter_para[i].cnt++] = ADC1_SampleBuffer[8];
			else if(i == 4)		avg_filter_para[i].buff[avg_filter_para[i].cnt++] = ADC1_SampleBuffer[7];
			else if(i == 5)		avg_filter_para[i].buff[avg_filter_para[i].cnt++] = ADC1_SampleBuffer[3];
			else if(i == 6)		avg_filter_para[i].buff[avg_filter_para[i].cnt++] = ADC1_SampleBuffer[4];
			else if(i == 7)		avg_filter_para[i].buff[avg_filter_para[i].cnt++] = ADC1_SampleBuffer[5];
			else if(i == 8)		avg_filter_para[i].buff[avg_filter_para[i].cnt++] = ADC1_SampleBuffer[0];
		}
	}
}

static void adc_init(void)
{
    ADC_PeriphConfig();
    ADC_RegConfig();
}
device_initcall(adc_init);

static void adc_app_init(void)
{
	static task_t adc_task;

	tiny_timer_create(&adc_task, adc_task_cb, NULL);
	tiny_timer_start(&adc_task, TIMER_FOREVER, 10);
}
app_initcall(adc_app_init);

uint16_t ADC_GetSample(uint8_t type)
{
    uint16_t temp = 0;
    switch (type)
    {
        case NTC_SAMPLE:
            temp = avg_filter_para[0].avg;
            break;
            
        case BAT1VAL_SAMPLE:
			temp = avg_filter_para[1].avg;
			break;
		
		case BAT2VAL_SAMPLE:
			temp = avg_filter_para[2].avg;
			break;

		case BAT1TEMP_SAMPLE:
			temp = avg_filter_para[3].avg;
			break;

		case BAT2TEMP_SAMPLE:
			temp = avg_filter_para[4].avg;
			break;

		case M1VAL_SAMPLE:
			temp = avg_filter_para[5].avg;
			break;

		case M2VAL_SAMPLE:
			temp = avg_filter_para[6].avg;
			break;

		case M3VAL_SAMPLE:
			temp = avg_filter_para[7].avg;
			break;

		case VREF_SAMPLE:
			temp = avg_filter_para[8].avg;
			break;
			
        default:
            break;
    }

    temp = (uint16_t)((float)temp * (1489 / (float)avg_filter_para[8].avg));
    return temp;
}

