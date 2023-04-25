#include "drv_hw_spi.h"

uint8_t spi1_4wire_trans_byte(uint8_t byte)
{
	uint8_t ret = 0;

	while (SPI_I2S_GetStatus(SPI1, SPI_I2S_TE_FLAG) == RESET);
	SPI_I2S_TransmitData(SPI1, byte);
	while (SPI_I2S_GetStatus(SPI1, SPI_I2S_RNE_FLAG) == RESET);
	ret = SPI_I2S_ReceiveData(SPI1);

	return ret;
}

void spi_hw_init(void)
{
	SPI_InitType SPI_InitStructure;
	GPIO_InitType GPIO_InitStructure;
	NVIC_InitType NVIC_InitStructure;
	EXTI_InitType EXTI_InitStructure;
	
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB | RCC_APB2_PERIPH_AFIO, ENABLE);
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_SPI1 | RCC_APB2_PERIPH_SPI2, ENABLE);
	RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA, ENABLE);
	
	//SPI1 SCK
	GPIO_InitStructure.Pin = GPIO_PIN_3;
	GPIO_InitStructure.GPIO_Alternate = GPIO_AF1_SPI1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
	//SPI1 MOSI
	GPIO_InitStructure.Pin = GPIO_PIN_5;
	GPIO_InitStructure.GPIO_Alternate = GPIO_AF0_SPI1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
	//SPI1 MISO
	GPIO_ConfigPinRemap(GPIOB_PORT_SOURCE, GPIO_PIN_SOURCE4, GPIO_AF1_SPI1);
	//SPI1 NSS
	GPIO_InitStructure.Pin = GPIO_PIN_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Current = GPIO_DC_4mA;
	GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
	SPI1_CS_DISABLE;
	
	//SPI2 SCK,MOSI
	GPIO_InitStructure.Pin = GPIO_PIN_13 | GPIO_PIN_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Input;
	GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_High;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
	//SPI2 MISO
	GPIO_InitStructure.Pin = GPIO_PIN_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
	//SPI2 NSS
	//SPI2 INT
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
	GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin        = GPIO_PIN_11;
    GPIO_InitStructure.GPIO_Pull  = GPIO_Pull_Down;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Input;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
    GPIO_ConfigEXTILine(GPIOA_PORT_SOURCE, GPIO_PIN_SOURCE11);

    EXTI_InitStructure.EXTI_Line    = EXTI_LINE11;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitPeripheral(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel                   = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

	//SPI1 init
	SPI_InitStructure.DataDirection = SPI_DIR_DOUBLELINE_FULLDUPLEX;
    SPI_InitStructure.SpiMode       = SPI_MODE_MASTER;
    SPI_InitStructure.DataLen       = SPI_DATA_SIZE_8BITS;
    SPI_InitStructure.CLKPOL        = SPI_CLKPOL_LOW;
    SPI_InitStructure.CLKPHA        = SPI_CLKPHA_FIRST_EDGE;
    SPI_InitStructure.NSS           = SPI_NSS_SOFT;
    SPI_InitStructure.BaudRatePres  = SPI_BR_PRESCALER_4;
    SPI_InitStructure.FirstBit      = SPI_FB_MSB;
    SPI_InitStructure.CRCPoly       = 7;
    SPI_Init(SPI1, &SPI_InitStructure);
	//SPI2 init
	SPI_InitStructure.SpiMode 		= SPI_MODE_SLAVE;
	SPI_InitStructure.NSS           = SPI_NSS_HARD;
    SPI_Init(SPI2, &SPI_InitStructure);
	
	SPI_I2S_EnableDma(SPI2, SPI_I2S_DMA_TX, ENABLE);
	SPI_I2S_EnableDma(SPI2, SPI_I2S_DMA_RX, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel    = DMA_Channel7_IRQn; /* SPI2 DMA_Rx*/   
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);

  	NVIC_InitStructure.NVIC_IRQChannel    = DMA_Channel8_IRQn; /* SPI2 DMA_Tx*/   
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
	
	SPI_Enable(SPI1, ENABLE);
	SPI_Enable(SPI2, ENABLE);
}

void spi2_dma_rx_config(uint8_t *mem_addr, uint32_t mem_size)
{
	DMA_InitType DMA_InitStructure;

	DMA_DeInit(DMA_CH7); 
	DMA_EnableChannel(DMA_CH7, DISABLE);
	DMA_InitStructure.PeriphAddr 				= (uint32_t)&(SPI2->DAT);
	DMA_InitStructure.MemAddr 					= (uint32_t)mem_addr; 
	DMA_InitStructure.Direction 				= DMA_DIR_PERIPH_SRC; 	
	DMA_InitStructure.BufSize 					= mem_size; 
	DMA_InitStructure.PeriphInc 				= DMA_PERIPH_INC_DISABLE; 
	DMA_InitStructure.DMA_MemoryInc 			= DMA_MEM_INC_ENABLE; 
	DMA_InitStructure.PeriphDataSize 			= DMA_PERIPH_DATA_SIZE_BYTE; 
	DMA_InitStructure.MemDataSize 				= DMA_MemoryDataSize_Byte;
	DMA_InitStructure.CircularMode 				= DMA_MODE_NORMAL; 
	DMA_InitStructure.Priority 					= DMA_PRIORITY_VERY_HIGH; 
	DMA_InitStructure.Mem2Mem 					= DMA_M2M_DISABLE; 
	DMA_Init(DMA_CH7, &DMA_InitStructure); 
	DMA_RequestRemap(DMA_REMAP_SPI2_RX, DMA, DMA_CH7, ENABLE);
	DMA_ConfigInt(DMA_CH7, DMA_INT_TXC, ENABLE);
	DMA_ClearFlag(DMA_FLAG_TC7, DMA);
	DMA_EnableChannel(DMA_CH7, ENABLE);
}

void spi2_dma_tx_config(uint8_t *mem_addr, uint32_t mem_size)
{
  	DMA_InitType DMA_InitStructure;
	
	DMA_DeInit(DMA_CH8);
	DMA_EnableChannel(DMA_CH8, DISABLE);
	DMA_InitStructure.PeriphAddr 				= (uint32_t)&(SPI2->DAT);
	DMA_InitStructure.MemAddr 					= (uint32_t)mem_addr; 
	DMA_InitStructure.Direction 				= DMA_DIR_PERIPH_DST; 	
	DMA_InitStructure.BufSize 					= mem_size; 
	DMA_InitStructure.PeriphInc 				= DMA_PERIPH_INC_DISABLE; 
	DMA_InitStructure.DMA_MemoryInc 			= DMA_MEM_INC_ENABLE; 
	DMA_InitStructure.PeriphDataSize 			= DMA_PERIPH_DATA_SIZE_BYTE; 
	DMA_InitStructure.MemDataSize 				= DMA_MemoryDataSize_Byte;
	DMA_InitStructure.CircularMode 				= DMA_MODE_NORMAL; 
	DMA_InitStructure.Priority 					= DMA_PRIORITY_VERY_HIGH; 
	DMA_InitStructure.Mem2Mem 					= DMA_M2M_DISABLE; 
	DMA_Init(DMA_CH8, &DMA_InitStructure);  
	DMA_RequestRemap(DMA_REMAP_SPI2_TX, DMA, DMA_CH8, ENABLE);
	DMA_ConfigInt(DMA_CH8, DMA_INT_TXC, ENABLE);
	DMA_ClearFlag(DMA_FLAG_TC8, DMA);
	DMA_EnableChannel(DMA_CH8, ENABLE);
}


