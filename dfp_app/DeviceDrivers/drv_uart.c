#include "drv_uart.h"

static void uart3_gpio_init(void)
{
	GPIO_InitType GPIO_InitStructure;

	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Alternate = GPIO_AF5_USART3;
	GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Up;
    GPIO_InitStructure.Pin   = GPIO_PIN_11;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Alternate = GPIO_AF0_USART3;
    GPIO_InitStructure.Pin = GPIO_PIN_10;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
}

void uart3_init(void)
{
	USART_InitType USART_InitStructure;
	NVIC_InitType NVIC_InitStructure;
	
	uart3_gpio_init();
	
	RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA, ENABLE);
	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_USART3, ENABLE);

	USART_InitStructure.BaudRate            = 115200;
	USART_InitStructure.WordLength          = USART_WL_8B;
	USART_InitStructure.StopBits            = USART_STPB_1;
	USART_InitStructure.Parity              = USART_PE_NO;
	USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
	USART_InitStructure.Mode                = USART_MODE_RX | USART_MODE_TX;
	USART_Init(USART3, &USART_InitStructure);
	
	USART_ConfigInt(USART3, USART_INT_IDLEF, ENABLE);	
	USART_Enable(USART3, ENABLE);
	USART_EnableDMA(USART3, USART_DMAREQ_RX|USART_DMAREQ_TX, ENABLE);

	/* 串口中断 */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* DMA中断 */
	NVIC_InitStructure.NVIC_IRQChannel    = DMA_Channel6_IRQn; /* UART3 DMA_Rx*/   
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
}

void uart3_dmarx_config(uint8_t *mem_addr, uint32_t mem_size)
{
  	DMA_InitType DMA_InitStructure;

	DMA_DeInit(DMA_CH6); 
	DMA_EnableChannel(DMA_CH6, DISABLE);
	DMA_InitStructure.PeriphAddr 				= (uint32_t)&(USART3->DAT);
	DMA_InitStructure.MemAddr 					= (uint32_t)mem_addr; 
	DMA_InitStructure.Direction 				= DMA_DIR_PERIPH_SRC; 	
	DMA_InitStructure.BufSize 					= mem_size; 
	DMA_InitStructure.PeriphInc 				= DMA_PERIPH_INC_DISABLE; 
	DMA_InitStructure.DMA_MemoryInc 			= DMA_MEM_INC_ENABLE; 
	DMA_InitStructure.PeriphDataSize 			= DMA_PERIPH_DATA_SIZE_BYTE; 
	DMA_InitStructure.MemDataSize 				= DMA_MemoryDataSize_Byte;
	DMA_InitStructure.CircularMode 				= DMA_MODE_CIRCULAR; 
	DMA_InitStructure.Priority 					= DMA_PRIORITY_HIGH; 
	DMA_InitStructure.Mem2Mem 					= DMA_M2M_DISABLE; 
	DMA_Init(DMA_CH6, &DMA_InitStructure); 
	DMA_RequestRemap(DMA_REMAP_USART3_RX, DMA, DMA_CH6, ENABLE);
	DMA_ConfigInt(DMA_CH6, DMA_INT_TXC|DMA_INT_HTX|DMA_INT_ERR, ENABLE);
	DMA_ClearFlag(DMA_FLAG_TC6, DMA);
	DMA_ClearFlag(DMA_FLAG_HT6, DMA);
	DMA_EnableChannel(DMA_CH6, ENABLE);
}

uint16_t uart3_get_dmarx_buf_remain_size(void)
{
	return DMA_GetCurrDataCounter(DMA_CH6);	
}

static void uart2_gpio_init(void)
{
	GPIO_InitType GPIO_InitStructure;

	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Alternate = GPIO_AF4_USART2;
	GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Up;
    GPIO_InitStructure.Pin   = GPIO_PIN_2;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = GPIO_PIN_3;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
}

void uart2_init(void)
{
	USART_InitType USART_InitStructure;
	NVIC_InitType NVIC_InitStructure;

	uart2_gpio_init();

	RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA, ENABLE);
	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_USART2, ENABLE);

    USART_InitStructure.BaudRate            = 115200;
    USART_InitStructure.WordLength          = USART_WL_8B;
    USART_InitStructure.StopBits            = USART_STPB_1;
    USART_InitStructure.Parity              = USART_PE_NO;
    USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
    USART_InitStructure.Mode                = USART_MODE_RX | USART_MODE_TX;
	USART_Init(USART2, &USART_InitStructure);
	
	USART_ConfigInt(USART2, USART_INT_IDLEF, ENABLE);
    USART_Enable(USART2, ENABLE);
    USART_EnableDMA(USART2, USART_DMAREQ_RX|USART_DMAREQ_TX, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* DMA中断 */	
	NVIC_InitStructure.NVIC_IRQChannel    = DMA_Channel4_IRQn; /* UART2 DMA_Rx*/   
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
}

void uart2_dmarx_config(uint8_t *mem_addr, uint32_t mem_size)
{
  	DMA_InitType DMA_InitStructure;

	DMA_DeInit(DMA_CH4); 
	DMA_EnableChannel(DMA_CH4, DISABLE);
	DMA_InitStructure.PeriphAddr 				= (uint32_t)&(USART2->DAT);
	DMA_InitStructure.MemAddr 					= (uint32_t)mem_addr; 
	DMA_InitStructure.Direction 				= DMA_DIR_PERIPH_SRC; 	
	DMA_InitStructure.BufSize 					= mem_size; 
	DMA_InitStructure.PeriphInc 				= DMA_PERIPH_INC_DISABLE; 
	DMA_InitStructure.DMA_MemoryInc 			= DMA_MEM_INC_ENABLE; 
	DMA_InitStructure.PeriphDataSize 			= DMA_PERIPH_DATA_SIZE_BYTE; 
	DMA_InitStructure.MemDataSize 				= DMA_MemoryDataSize_Byte;
	DMA_InitStructure.CircularMode 				= DMA_MODE_CIRCULAR; 
	DMA_InitStructure.Priority 					= DMA_PRIORITY_HIGH; 
	DMA_InitStructure.Mem2Mem 					= DMA_M2M_DISABLE; 
	DMA_Init(DMA_CH4, &DMA_InitStructure); 
	DMA_RequestRemap(DMA_REMAP_USART2_RX, DMA, DMA_CH4, ENABLE);
	DMA_ConfigInt(DMA_CH4, DMA_INT_TXC|DMA_INT_HTX|DMA_INT_ERR, ENABLE);
	DMA_ClearFlag(DMA_FLAG_TC4, DMA);
	DMA_ClearFlag(DMA_FLAG_HT4, DMA);
	DMA_EnableChannel(DMA_CH4, ENABLE);
}

uint16_t uart2_get_dmarx_buf_remain_size(void)
{
	return DMA_GetCurrDataCounter(DMA_CH4);	
}


