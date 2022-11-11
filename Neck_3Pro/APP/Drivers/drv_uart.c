#include "drv_uart.h"

static void uart1_gpio_init(void)
{
	GPIO_InitType GPIO_InitStructure;
	
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Alternate = GPIO_AF0_USART1;
	GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Up;
    GPIO_InitStructure.Pin   = GPIO_PIN_7;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = GPIO_PIN_6;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
}

void uart1_init(void)
{
	USART_InitType USART_InitStructure;
	NVIC_InitType NVIC_InitStructure;
	
	uart1_gpio_init();
	
	/* 使能串口和DMA时钟 */
	RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA, ENABLE);
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
	NVIC_InitStructure.NVIC_IRQChannel    = DMA_Channel1_IRQn;  /* UART1 DMA_Tx*/     
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel    = DMA_Channel2_IRQn; /* UART1 DMA_Rx*/   
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
}

void uart1_dmatx_config(uint8_t *mem_addr, uint32_t mem_size)
{
  	DMA_InitType DMA_InitStructure;
	
	DMA_DeInit(DMA_CH1);
	DMA_EnableChannel(DMA_CH1, DISABLE);
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
	DMA_Init(DMA_CH1, &DMA_InitStructure);  
	DMA_RequestRemap(DMA_REMAP_USART1_TX, DMA, DMA_CH1, ENABLE);
	DMA_ConfigInt(DMA_CH1, DMA_INT_TXC|DMA_INT_ERR, ENABLE); 
	DMA_ClearFlag(DMA_FLAG_TC1, DMA);	/* 清除发送完成标识 */
	DMA_EnableChannel(DMA_CH1, ENABLE);
}

void uart1_dmarx_config(uint8_t *mem_addr, uint32_t mem_size)
{
  	DMA_InitType DMA_InitStructure;
	
	DMA_DeInit(DMA_CH2); 
	DMA_EnableChannel(DMA_CH2, DISABLE);
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
	DMA_Init(DMA_CH2, &DMA_InitStructure); 
	DMA_RequestRemap(DMA_REMAP_USART1_RX, DMA, DMA_CH2, ENABLE);
	DMA_ConfigInt(DMA_CH2, DMA_INT_TXC|DMA_INT_HTX|DMA_INT_ERR, ENABLE);/* 使能DMA半满、溢满、错误中断 */
	DMA_ClearFlag(DMA_FLAG_TC2, DMA);
	DMA_ClearFlag(DMA_FLAG_HT2, DMA);
	DMA_EnableChannel(DMA_CH2, ENABLE);
}

uint16_t uart1_get_dmarx_buf_remain_size(void)
{
	return DMA_GetCurrDataCounter(DMA_CH2);	
}

static void uart4_gpio_init(void)
{
    GPIO_InitType GPIO_InitStructure;

	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Alternate = GPIO_AF6_UART4;
	GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Up;
    GPIO_InitStructure.Pin   = GPIO_PIN_15;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = GPIO_PIN_14;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
}

void uart4_init(void)
{
	USART_InitType USART_InitStructure;
	NVIC_InitType NVIC_InitStructure;
	
	uart4_gpio_init();
	
	/* 使能串口和DMA时钟 */
	RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA, ENABLE);
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_UART4, ENABLE);

	USART_InitStructure.BaudRate            = 115200;
	USART_InitStructure.WordLength          = USART_WL_8B;
	USART_InitStructure.StopBits            = USART_STPB_1;
	USART_InitStructure.Parity              = USART_PE_NO;
	USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
	USART_InitStructure.Mode                = USART_MODE_RX | USART_MODE_TX;
	USART_Init(UART4, &USART_InitStructure);
	
	USART_ConfigInt(UART4, USART_INT_IDLEF, ENABLE);	/* 使能空闲中断 */
	USART_Enable(UART4, ENABLE);
	USART_EnableDMA(UART4, USART_DMAREQ_TX|USART_DMAREQ_RX, ENABLE); /* 使能DMA收发 */

	/* 串口中断 */
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* DMA中断 */
	NVIC_InitStructure.NVIC_IRQChannel    = DMA_Channel7_IRQn;  /* UART4 DMA_Tx*/     
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel    = DMA_Channel8_IRQn; /* UART4 DMA_Rx*/   
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
}

void uart4_dmatx_config(uint8_t *mem_addr, uint32_t mem_size)
{
  	DMA_InitType DMA_InitStructure;
	
	DMA_DeInit(DMA_CH7);
	DMA_EnableChannel(DMA_CH7, DISABLE);
	DMA_InitStructure.PeriphAddr 				= (uint32_t)&(UART4->DAT);
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
	DMA_Init(DMA_CH7, &DMA_InitStructure);  
	DMA_RequestRemap(DMA_REMAP_UART4_TX, DMA, DMA_CH7, ENABLE);
	DMA_ConfigInt(DMA_CH7, DMA_INT_TXC|DMA_INT_ERR, ENABLE); 
	DMA_ClearFlag(DMA_FLAG_TC7, DMA);	/* 清除发送完成标识 */
	DMA_EnableChannel(DMA_CH7, ENABLE); 
}

void uart4_dmarx_config(uint8_t *mem_addr, uint32_t mem_size)
{
  	DMA_InitType DMA_InitStructure;
	
	DMA_DeInit(DMA_CH8); 
	DMA_EnableChannel(DMA_CH8, DISABLE);
	DMA_InitStructure.PeriphAddr 				= (uint32_t)&(UART4->DAT);
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
	DMA_Init(DMA_CH8, &DMA_InitStructure); 
	DMA_RequestRemap(DMA_REMAP_UART4_RX, DMA, DMA_CH8, ENABLE);
	DMA_ConfigInt(DMA_CH8, DMA_INT_TXC|DMA_INT_HTX|DMA_INT_ERR, ENABLE);/* 使能DMA半满、溢满、错误中断 */
	DMA_ClearFlag(DMA_FLAG_TC8, DMA);
	DMA_ClearFlag(DMA_FLAG_HT8, DMA);
	DMA_EnableChannel(DMA_CH8, ENABLE); 
}

uint16_t uart4_get_dmarx_buf_remain_size(void)
{
	return DMA_GetCurrDataCounter(DMA_CH8);	
}


