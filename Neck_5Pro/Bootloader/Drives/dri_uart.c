#include "dri_uart.h"

/**
 * @brief  uart1 gpio配置
 * @param  
 * @retval 
 */
static void bsp_uart1_gpio_init(void)
{
    GPIO_InitType GPIO_InitStructure;
	
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.Pin   = GPIO_PIN_10;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.Pin = GPIO_PIN_9;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
}


/**
 * @brief  uart1初始化配置
 * @param  
 * @retval 
 */
void bsp_uart1_init(void)
{
	USART_InitType USART_InitStructure;
	NVIC_InitType NVIC_InitStructure;
	
	bsp_uart1_gpio_init();
	
	/* 使能串口和DMA时钟 */
	RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA1, ENABLE);
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_USART1, ENABLE);

	USART_InitStructure.BaudRate            = 115200;
	USART_InitStructure.WordLength          = USART_WL_8B;
	USART_InitStructure.StopBits            = USART_STPB_1;
	USART_InitStructure.Parity              = USART_PE_NO;
	USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
	USART_InitStructure.Mode                = USART_MODE_RX | USART_MODE_TX;
	USART_Init(USART1, &USART_InitStructure);
	
	USART_ConfigInt(USART1, USART_INT_IDLEF, ENABLE);	/* 使能空闲中断 */
	USART_Enable(USART1, ENABLE);
	USART_EnableDMA(USART1, USART_DMAREQ_TX|USART_DMAREQ_RX, ENABLE); /* 使能DMA收发 */

	/* 串口中断 */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* DMA中断 */
	NVIC_InitStructure.NVIC_IRQChannel    = DMA1_Channel4_IRQn;  /* UART1 DMA1Tx*/     
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel    = DMA1_Channel5_IRQn; /* UART1 DMA1Rx*/   
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
}


/**
 * @brief  uart1 dma发送通道配置
 * @param  
 * @retval 
 */
void bsp_uart1_dmatx_config(uint8_t *mem_addr, uint32_t mem_size)
{
  	DMA_InitType DMA_InitStructure;
	
	DMA_DeInit(DMA1_CH4);
	DMA_EnableChannel(DMA1_CH4, DISABLE);
	DMA_InitStructure.PeriphAddr 				= (uint32_t)&(USART1->DAT);
	DMA_InitStructure.MemAddr 					= (uint32_t)mem_addr; 
	DMA_InitStructure.Direction 				= DMA_DIR_PERIPH_DST; 	/* 传输方向:内存->外设 */
	DMA_InitStructure.BufSize 					= mem_size; 
	DMA_InitStructure.PeriphInc 				= DMA_PERIPH_INC_DISABLE; 
	DMA_InitStructure.DMA_MemoryInc 			= DMA_MEM_INC_ENABLE; 
	DMA_InitStructure.PeriphDataSize 			= DMA_PERIPH_DATA_SIZE_BYTE; 
	DMA_InitStructure.MemDataSize 				= DMA_MemoryDataSize_Byte;
	DMA_InitStructure.CircularMode 				= DMA_MODE_NORMAL; 
	DMA_InitStructure.Priority 					= DMA_PRIORITY_HIGH; 
	DMA_InitStructure.Mem2Mem 					= DMA_M2M_DISABLE; 
	DMA_Init(DMA1_CH4, &DMA_InitStructure);  
	DMA_ConfigInt(DMA1_CH4, DMA_INT_TXC|DMA_INT_ERR, ENABLE); 
	DMA_ClearFlag(DMA1_INT_TXC4, DMA1);	/* 清除发送完成标识 */
	DMA_EnableChannel(DMA1_CH4, ENABLE); 
}

/**
 * @brief  uart1 dma接收通道配置
 * @param  
 * @retval 
 */
void bsp_uart1_dmarx_config(uint8_t *mem_addr, uint32_t mem_size)
{
  	DMA_InitType DMA_InitStructure;
	
	DMA_DeInit(DMA1_CH5); 
	DMA_EnableChannel(DMA1_CH5, DISABLE);
	DMA_InitStructure.PeriphAddr 				= (uint32_t)&(USART1->DAT);
	DMA_InitStructure.MemAddr 					= (uint32_t)mem_addr; 
	DMA_InitStructure.Direction 				= DMA_DIR_PERIPH_SRC; 	/* 传输方向:外设->内存 */
	DMA_InitStructure.BufSize 					= mem_size; 
	DMA_InitStructure.PeriphInc 				= DMA_PERIPH_INC_DISABLE; 
	DMA_InitStructure.DMA_MemoryInc 			= DMA_MEM_INC_ENABLE; 
	DMA_InitStructure.PeriphDataSize 			= DMA_PERIPH_DATA_SIZE_BYTE; 
	DMA_InitStructure.MemDataSize 				= DMA_MemoryDataSize_Byte;
	DMA_InitStructure.CircularMode 				= DMA_MODE_CIRCULAR; 
	DMA_InitStructure.Priority 					= DMA_PRIORITY_VERY_HIGH; 
	DMA_InitStructure.Mem2Mem 					= DMA_M2M_DISABLE; 
	DMA_Init(DMA1_CH5, &DMA_InitStructure); 
	DMA_ConfigInt(DMA1_CH5, DMA_INT_TXC|DMA_INT_HTX|DMA_INT_ERR, ENABLE);/* 使能DMA半满、溢满、错误中断 */
	DMA_ClearFlag(DMA1_INT_TXC5, DMA1);
	DMA_ClearFlag(DMA1_INT_HTX5, DMA1);
	DMA_EnableChannel(DMA1_CH5, ENABLE); 
}

/**
 * @brief  获取DMA接收buf剩余空间
 * @param  
 * @retval 
 */
uint16_t bsp_uart1_get_dmarx_buf_remain_size(void)
{
	return DMA_GetCurrDataCounter(DMA1_CH5);	
}


