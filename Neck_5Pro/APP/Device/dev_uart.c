#include "dev_uart.h"

/* 串口缓存 */
static uint8_t s_uart1_tx_buf[UART1_TX_BUF_SIZE];
static uint8_t s_uart1_rx_buf[UART1_RX_BUF_SIZE];
static uint8_t s_uart1_dmarx_buf[UART1_DMA_RX_BUF_SIZE];
static uint8_t s_uart1_dmatx_buf[UART1_DMA_TX_BUF_SIZE];

static uint8_t s_uart4_tx_buf[UART4_TX_BUF_SIZE];
static uint8_t s_uart4_rx_buf[UART4_RX_BUF_SIZE];
static uint8_t s_uart4_dmarx_buf[UART4_DMA_RX_BUF_SIZE];
static uint8_t s_uart4_dmatx_buf[UART4_DMA_TX_BUF_SIZE];

/* 串口设备实例 */
static uart_device_t s_uart_dev[2] = {0};
uint32_t s_UartTxRxCount[4] = {0};

/* fifo上锁函数 */
static void fifo_lock(void)
{
    rt_enter_critical();//__disable_irq();
}

/* fifo解锁函数 */
static void fifo_unlock(void)
{
    rt_exit_critical();//__enable_irq();
}

/**
 * @brief 串口设备初始化 
 * @param  
 * @retval 
 */
static void uart_device_init(uint8_t uart_id)
{
  	if (uart_id == 0)
	{
		uart1_init();
		/* 配置串口1收发fifo */
		fifo_register(&s_uart_dev[uart_id].tx_fifo, &s_uart1_tx_buf[0], 
                      sizeof(s_uart1_tx_buf), fifo_lock, fifo_unlock);
		fifo_register(&s_uart_dev[uart_id].rx_fifo, &s_uart1_rx_buf[0], 
                      sizeof(s_uart1_rx_buf), fifo_lock, fifo_unlock);
		
		/* 配置串口1 DMA收发buf */
		s_uart_dev[uart_id].dmarx_buf = &s_uart1_dmarx_buf[0];
		s_uart_dev[uart_id].dmarx_buf_size = sizeof(s_uart1_dmarx_buf);
		s_uart_dev[uart_id].dmatx_buf = &s_uart1_dmatx_buf[0];
		s_uart_dev[uart_id].dmatx_buf_size = sizeof(s_uart1_dmatx_buf);
		uart1_dmarx_config(s_uart_dev[uart_id].dmarx_buf, 
							   sizeof(s_uart1_dmarx_buf));/* 只需配置接收模式DMA，发送模式需发送数据时才配置 */
		s_uart_dev[uart_id].status  = 0;
	}
	else if (uart_id == 1)
	{
		uart4_init();
		/* 配置串口1收发fifo */
		fifo_register(&s_uart_dev[uart_id].tx_fifo, &s_uart4_tx_buf[0], 
                      sizeof(s_uart4_tx_buf), fifo_lock, fifo_unlock);
		fifo_register(&s_uart_dev[uart_id].rx_fifo, &s_uart4_rx_buf[0], 
                      sizeof(s_uart4_rx_buf), fifo_lock, fifo_unlock);
		
		/* 配置串口1 DMA收发buf */
		s_uart_dev[uart_id].dmarx_buf = &s_uart4_dmarx_buf[0];
		s_uart_dev[uart_id].dmarx_buf_size = sizeof(s_uart4_dmarx_buf);
		s_uart_dev[uart_id].dmatx_buf = &s_uart4_dmatx_buf[0];
		s_uart_dev[uart_id].dmatx_buf_size = sizeof(s_uart4_dmatx_buf);
		uart4_dmarx_config(s_uart_dev[uart_id].dmarx_buf, 
							   sizeof(s_uart4_dmarx_buf));/* 只需配置接收模式DMA，发送模式需发送数据时才配置 */
		s_uart_dev[uart_id].status  = 0;
	}
}

/**
 * @brief  串口发送数据接口，实际是写入发送fifo，发送由dma处理
 * @param  
 * @retval 
 */
uint16_t uart_write(uint8_t uart_id, const uint8_t *buf, uint16_t size)
{
	return fifo_write(&s_uart_dev[uart_id].tx_fifo, buf, size);
}

/**
 * @brief  串口读取数据接口，实际是从接收fifo读取
 * @param  
 * @retval 
 */
uint16_t uart_read(uint8_t uart_id, uint8_t *buf, uint16_t size)
{
	return fifo_read(&s_uart_dev[uart_id].rx_fifo, buf, size);
}

/**
 * @brief  串口dma接收完成中断处理
 * @param  
 * @retval 
 */
static void uart_dmarx_done_isr(uint8_t uart_id)
{
  	uint16_t recv_size;
	
	recv_size = s_uart_dev[uart_id].dmarx_buf_size - s_uart_dev[uart_id].last_dmarx_size;
    s_UartTxRxCount[uart_id*2+1] += recv_size;
	fifo_write(&s_uart_dev[uart_id].rx_fifo, 
				   (const uint8_t *)&(s_uart_dev[uart_id].dmarx_buf[s_uart_dev[uart_id].last_dmarx_size]), recv_size);

	s_uart_dev[uart_id].last_dmarx_size = 0;
}

/**
 * @brief  串口dma接收缓存大小一半数据中断处理
 * @param  
 * @retval 
 */
static void uart_dmarx_half_done_isr(uint8_t uart_id)
{
  	uint16_t recv_total_size;
  	uint16_t recv_size;
	
	if(uart_id == 0)
	{
	  	recv_total_size = s_uart_dev[uart_id].dmarx_buf_size - uart1_get_dmarx_buf_remain_size();
	}
	else if(uart_id == 1)
	{
		recv_total_size = s_uart_dev[uart_id].dmarx_buf_size - uart4_get_dmarx_buf_remain_size();
	}
	recv_size = recv_total_size - s_uart_dev[uart_id].last_dmarx_size;
	s_UartTxRxCount[uart_id*2+1] += recv_size;
    
	fifo_write(&s_uart_dev[uart_id].rx_fifo, 
				   (const uint8_t *)&(s_uart_dev[uart_id].dmarx_buf[s_uart_dev[uart_id].last_dmarx_size]), recv_size);
	s_uart_dev[uart_id].last_dmarx_size = recv_total_size;
}

/**
 * @brief  串口空闲中断处理
 * @param  
 * @retval 
 */
static void uart_dmarx_idle_isr(uint8_t uart_id)
{
  	uint16_t recv_total_size;
  	uint16_t recv_size;
	
	if(uart_id == 0)
	{
	  	recv_total_size = s_uart_dev[uart_id].dmarx_buf_size - uart1_get_dmarx_buf_remain_size();
	}
	else if(uart_id == 1)
	{
		recv_total_size = s_uart_dev[uart_id].dmarx_buf_size - uart4_get_dmarx_buf_remain_size();
	}
	recv_size = recv_total_size - s_uart_dev[uart_id].last_dmarx_size;
	s_UartTxRxCount[uart_id*2+1] += recv_size;
	fifo_write(&s_uart_dev[uart_id].rx_fifo, 
				   (const uint8_t *)&(s_uart_dev[uart_id].dmarx_buf[s_uart_dev[uart_id].last_dmarx_size]), recv_size);
	s_uart_dev[uart_id].last_dmarx_size = recv_total_size;
}

/**
 * @brief  串口dma发送完成中断处理
 * @param  
 * @retval 
 */
static void uart_dmatx_done_isr(uint8_t uart_id)
{
 	s_uart_dev[uart_id].status = 0;	/* DMA发送空闲 */
}

/**
 * @brief  循环从串口发送fifo读出数据，放置于dma发送缓存，并启动dma传输
 * @param  
 * @retval 
 */
void uart_poll_dma_tx(uint8_t uart_id)
{
  	uint16_t size = 0;
	
	if (0x01 == s_uart_dev[uart_id].status)
    {
        return;
    }
	size = fifo_read(&s_uart_dev[uart_id].tx_fifo, s_uart_dev[uart_id].dmatx_buf,
					 s_uart_dev[uart_id].dmatx_buf_size);
	if (size != 0)
	{	
        s_UartTxRxCount[uart_id*2+0] += size;
	  	if (uart_id == 0)
		{
            s_uart_dev[uart_id].status = 0x01;	/* DMA发送状态 */
		  	uart1_dmatx_config(s_uart_dev[uart_id].dmatx_buf, size);
		}
		else if(uart_id == 1)
		{
			s_uart_dev[uart_id].status = 0x01;	/* DMA发送状态 */
		  	uart4_dmatx_config(s_uart_dev[uart_id].dmatx_buf, size);
		}
	}
}

/**
 * @brief  串口1中断处理
 * @param  
 * @retval 
 */
void USART1_IRQHandler(void)
{
	if(USART_GetIntStatus(USART1, USART_INT_IDLEF) != RESET)
	{
		uart_dmarx_idle_isr(DEV_UART1);
		/* read the data to clear receive idle interrupt flag */
		USART_ReceiveData(USART1);
	}
}

/**
  * @brief	DMA发送中断处理
  */
 void DMA1_Channel4_IRQHandler(void)
{
	if(DMA_GetIntStatus(DMA1_INT_TXC4, DMA1))
	{
		uart_dmatx_done_isr(DEV_UART1);
		DMA_ClearFlag(DMA1_FLAG_TC4, DMA1);
        DMA_EnableChannel(DMA1_CH4, DISABLE);
	}
}

 /**
  * @brief	DMA接收中断处理
  */
 void DMA1_Channel5_IRQHandler(void)
 {
	 if(DMA_GetIntStatus(DMA1_INT_TXC5, DMA1))
	 {
		 uart_dmarx_done_isr(DEV_UART1);
		 DMA_ClearFlag(DMA1_FLAG_TC5, DMA1);
	 }
	 if(DMA_GetIntStatus(DMA1_INT_HTX5, DMA1))
	 {
		 uart_dmarx_half_done_isr(DEV_UART1);
		 DMA_ClearFlag(DMA1_FLAG_HT5, DMA1);
	 }
 }

 /**
 * @brief	串口4中断处理
 */
void UART4_IRQHandler(void)
{
	if(USART_GetIntStatus(UART4, USART_INT_IDLEF) != RESET)
	{
		uart_dmarx_idle_isr(DEV_UART4);
		/* read the data to clear receive idle interrupt flag */
		USART_ReceiveData(UART4);
	}
}

/**
 * @brief	DMA发送中断处理
 */
 void DMA2_Channel5_IRQHandler(void)
{
	if(DMA_GetIntStatus(DMA2_INT_TXC5, DMA2))
	{
		uart_dmatx_done_isr(DEV_UART4);
		DMA_ClearFlag(DMA2_FLAG_TC5, DMA2);
        DMA_EnableChannel(DMA2_CH5, DISABLE);
	}
}

/**
 * @brief	DMA接收中断处理
 */
 void DMA2_Channel3_IRQHandler(void)
 {
	 if(DMA_GetIntStatus(DMA2_INT_TXC3, DMA2))
	 {
		 uart_dmarx_done_isr(DEV_UART4);
		 DMA_ClearFlag(DMA2_FLAG_TC3, DMA2);
	 }
	 if(DMA_GetIntStatus(DMA2_INT_HTX3, DMA2))
	 {
		 uart_dmarx_half_done_isr(DEV_UART4);
		 DMA_ClearFlag(DMA2_FLAG_HT3, DMA2);
	 }
 }

static void uart_device_register(void)
{
	uart_device_init(DEV_UART1);
	uart_device_init(DEV_UART4);
}

debug_initcall(uart_device_register);


