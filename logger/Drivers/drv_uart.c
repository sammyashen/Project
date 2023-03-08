#include "drv_uart.h"

static void uart1_gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void uart1_init(void)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	uart1_gpio_init();
	
	/* 使能串口和DMA时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	USART_InitStructure.USART_BaudRate					= 2000000;
	USART_InitStructure.USART_WordLength 				= USART_WordLength_8b;
	USART_InitStructure.USART_StopBits					= USART_StopBits_1;
	USART_InitStructure.USART_Parity					= USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl		= USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode						= USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
	USART_Cmd(USART1, ENABLE);
	USART_DMACmd(USART1, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);

	/* 串口中断 */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* DMA中断 */
	NVIC_InitStructure.NVIC_IRQChannel    = DMA2_Stream7_IRQn;  /* UART1 DMA_Tx*/     
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel    = DMA2_Stream5_IRQn; /* UART1 DMA_Rx*/   
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
}

void uart1_dmatx_config(uint8_t *mem_addr, uint32_t mem_size)
{
  	DMA_InitTypeDef DMA_InitStructure;
	
	DMA_DeInit(DMA2_Stream7);
	DMA_Cmd(DMA2_Stream7, DISABLE);
	DMA_InitStructure.DMA_Channel				= DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr	= (uint32_t)&(USART1->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr		= (uint32_t)mem_addr;
	DMA_InitStructure.DMA_DIR					= DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize			= mem_size;
	DMA_InitStructure.DMA_PeripheralInc			= DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc				= DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize	= DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize		= DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode					= DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority				= DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode				= DMA_FIFOMode_Enable;
	DMA_InitStructure.DMA_FIFOThreshold			= DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_PeripheralBurst		= DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_MemoryBurst			= DMA_MemoryBurst_Single;
	DMA_Init(DMA2_Stream7, &DMA_InitStructure);  
	DMA_ITConfig(DMA2_Stream7, DMA_IT_TC | DMA_IT_TE, ENABLE);
	DMA_ClearFlag(DMA2_Stream7, DMA_FLAG_TCIF7);
	DMA_Cmd(DMA2_Stream7, ENABLE);
}

void uart1_dmarx_config(uint8_t *mem_addr, uint32_t mem_size)
{
  	DMA_InitTypeDef DMA_InitStructure;

  	DMA_DeInit(DMA2_Stream5);
	DMA_Cmd(DMA2_Stream5, DISABLE);
	DMA_InitStructure.DMA_Channel				= DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr	= (uint32_t)&(USART1->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr		= (uint32_t)mem_addr;
	DMA_InitStructure.DMA_DIR					= DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize			= mem_size;
	DMA_InitStructure.DMA_PeripheralInc			= DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc				= DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize	= DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize		= DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode					= DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority				= DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_FIFOMode				= DMA_FIFOMode_Enable;
	DMA_InitStructure.DMA_FIFOThreshold			= DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_PeripheralBurst		= DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_MemoryBurst			= DMA_MemoryBurst_Single;
	DMA_Init(DMA2_Stream5, &DMA_InitStructure);  
	DMA_ITConfig(DMA2_Stream5, DMA_IT_TC | DMA_IT_HT | DMA_IT_TE, ENABLE);
	DMA_ClearFlag(DMA2_Stream5, DMA_FLAG_TCIF5);
	DMA_ClearFlag(DMA2_Stream5, DMA_FLAG_HTIF5);
	DMA_Cmd(DMA2_Stream5, ENABLE);
}

uint16_t uart1_get_dmarx_buf_remain_size(void)
{
	return DMA_GetCurrDataCounter(DMA2_Stream5);	
}

static void uart6_gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void uart6_init(void)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	uart6_gpio_init();
	
	/* 使能串口和DMA时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

	USART_InitStructure.USART_BaudRate					= 115200;
	USART_InitStructure.USART_WordLength 				= USART_WordLength_8b;
	USART_InitStructure.USART_StopBits					= USART_StopBits_1;
	USART_InitStructure.USART_Parity					= USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl		= USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode						= USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART6, &USART_InitStructure);

	USART_ITConfig(USART6, USART_IT_IDLE, ENABLE);
	USART_Cmd(USART6, ENABLE);
	USART_DMACmd(USART6, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);

	/* 串口中断 */
	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* DMA中断 */
	NVIC_InitStructure.NVIC_IRQChannel    = DMA2_Stream6_IRQn;  /* UART6 DMA_Tx*/     
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel    = DMA2_Stream2_IRQn; /* UART6 DMA_Rx*/   
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
}

void uart6_dmatx_config(uint8_t *mem_addr, uint32_t mem_size)
{
  	DMA_InitTypeDef DMA_InitStructure;
	
	DMA_DeInit(DMA2_Stream6);
	DMA_Cmd(DMA2_Stream6, DISABLE);
	DMA_InitStructure.DMA_Channel				= DMA_Channel_5;
	DMA_InitStructure.DMA_PeripheralBaseAddr	= (uint32_t)&(USART6->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr		= (uint32_t)mem_addr;
	DMA_InitStructure.DMA_DIR					= DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize			= mem_size;
	DMA_InitStructure.DMA_PeripheralInc			= DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc				= DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize	= DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize		= DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode					= DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority				= DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode				= DMA_FIFOMode_Enable;
	DMA_InitStructure.DMA_FIFOThreshold			= DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_PeripheralBurst		= DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_MemoryBurst			= DMA_MemoryBurst_Single;
	DMA_Init(DMA2_Stream6, &DMA_InitStructure);  
	DMA_ITConfig(DMA2_Stream6, DMA_IT_TC | DMA_IT_TE, ENABLE);
	DMA_ClearFlag(DMA2_Stream6, DMA_FLAG_TCIF6);
	DMA_Cmd(DMA2_Stream6, ENABLE);
}

void uart6_dmarx_config(uint8_t *mem_addr, uint32_t mem_size)
{
  	DMA_InitTypeDef DMA_InitStructure;

  	DMA_DeInit(DMA2_Stream2);
	DMA_Cmd(DMA2_Stream2, DISABLE);
	DMA_InitStructure.DMA_Channel				= DMA_Channel_5;
	DMA_InitStructure.DMA_PeripheralBaseAddr	= (uint32_t)&(USART6->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr		= (uint32_t)mem_addr;
	DMA_InitStructure.DMA_DIR					= DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize			= mem_size;
	DMA_InitStructure.DMA_PeripheralInc			= DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc				= DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize	= DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize		= DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode					= DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority				= DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_FIFOMode				= DMA_FIFOMode_Enable;
	DMA_InitStructure.DMA_FIFOThreshold			= DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_PeripheralBurst		= DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_MemoryBurst			= DMA_MemoryBurst_Single;
	DMA_Init(DMA2_Stream2, &DMA_InitStructure);  
	DMA_ITConfig(DMA2_Stream2, DMA_IT_TC | DMA_IT_HT | DMA_IT_TE, ENABLE);
	DMA_ClearFlag(DMA2_Stream2, DMA_FLAG_TCIF2);
	DMA_ClearFlag(DMA2_Stream2, DMA_FLAG_HTIF2);
	DMA_Cmd(DMA2_Stream2, ENABLE);
}

uint16_t uart6_get_dmarx_buf_remain_size(void)
{
	return DMA_GetCurrDataCounter(DMA2_Stream2);	
}



