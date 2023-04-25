#include "drv_dac.h"

void dac_gpio_init(void)
{
	GPIO_InitType GPIO_InitStructure;
	
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
	GPIO_InitStructure.Pin       = GPIO_PIN_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Input;
	GPIO_InitStructure.GPIO_Pull = GPIO_No_Pull;
	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
}

void dac_tim4_init(void)
{
	TIM_TimeBaseInitType TIM_TimeBaseStructure;
	
	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM4, ENABLE);
	TIM_InitTimBaseStruct(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.Period    = 0xFFFF; 
    TIM_TimeBaseStructure.Prescaler = 0xF;
    TIM_TimeBaseStructure.ClkDiv    = 0x0;
    TIM_TimeBaseStructure.CntMode   = TIM_CNT_MODE_UP;
    TIM_InitTimeBase(TIM4, &TIM_TimeBaseStructure);
	
	TIM_SelectOutputTrig(TIM4, TIM_TRGO_SRC_UPDATE);
}

void dac_wave_init(void)
{
	DAC_InitType DAC_InitStructure;
	
	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_DAC, ENABLE);
	DAC_InitStructure.Trigger      = DAC_TRG_T4_TRGO;
    DAC_InitStructure.WaveGen      = DAC_WAVEGEN_NONE;
    DAC_InitStructure.BufferOutput = DAC_BUFFOUTPUT_DISABLE;
    DAC_Init(&DAC_InitStructure);
}

void dac_init(void)
{
	dac_gpio_init();
	dac_tim4_init();
	dac_wave_init();
}

void dac_tx(uint32_t *mem_addr, uint32_t mem_size)
{
	DMA_InitType DMA_InitStructure;
	
	RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA, ENABLE);
	TIM_Enable(TIM4, DISABLE);
	DAC_Enable(DISABLE);
    DMA_DeInit(DMA_CH2);
    DMA_InitStructure.PeriphAddr     = (uint32_t)&(DAC->DR12CH);
    DMA_InitStructure.MemAddr        = (uint32_t)mem_addr;
    DMA_InitStructure.Direction      = DMA_DIR_PERIPH_DST;
    DMA_InitStructure.BufSize        = mem_size;
    DMA_InitStructure.PeriphInc      = DMA_PERIPH_INC_DISABLE;
    DMA_InitStructure.DMA_MemoryInc  = DMA_MEM_INC_ENABLE;
    DMA_InitStructure.PeriphDataSize = DMA_PERIPH_DATA_SIZE_WORD;
    DMA_InitStructure.MemDataSize    = DMA_MemoryDataSize_Word;
    DMA_InitStructure.CircularMode   = DMA_MODE_CIRCULAR;
    DMA_InitStructure.Priority       = DMA_PRIORITY_HIGH;
    DMA_InitStructure.Mem2Mem        = DMA_M2M_DISABLE;

    DMA_Init(DMA_CH2, &DMA_InitStructure);
    DMA_RequestRemap(DMA_REMAP_DAC1,DMA,DMA_CH2,ENABLE);
	DMA_EnableChannel(DMA_CH2, ENABLE);
	
	DAC_Enable(ENABLE);
	DAC_DmaEnable(ENABLE);
	TIM_Enable(TIM4, ENABLE);
}
