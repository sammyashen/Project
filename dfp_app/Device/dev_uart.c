#include "dev_uart.h"
#include "rtthread.h"

/* 串口缓存 */
static uint8_t s_uart3_rx_buf[UART3_RX_BUF_SIZE];
static uint8_t s_uart3_dmarx_buf[UART3_DMA_RX_BUF_SIZE];

static uint8_t s_uart2_rx_buf[UART2_RX_BUF_SIZE];
static uint8_t s_uart2_dmarx_buf[UART2_DMA_RX_BUF_SIZE];

/* 串口设备实例 */
static uart_device_t s_uart_dev[2] = {0};
uint32_t s_UartTxRxCount[2*2] = {0};

/* fifo上锁函数 */
static void fifo_lock(void)
{
    rt_enter_critical();
}

/* fifo解锁函数 */
static void fifo_unlock(void)
{
    rt_exit_critical();
}

/**
 * @brief 串口设备初始化 
 * @param  
 * @retval 
 */
static void uart_device_init(uint8_t uart_id)
{
	if (uart_id == DEV_UART3)
	{
		uart3_init();
		/* 配置串口3收发fifo */
		fifo_register(&s_uart_dev[uart_id].rx_fifo, &s_uart3_rx_buf[0], 
                      sizeof(s_uart3_rx_buf), fifo_lock, fifo_unlock);
		
		/* 配置串口3 DMA收发buf */
		s_uart_dev[uart_id].dmarx_buf = &s_uart3_dmarx_buf[0];
		s_uart_dev[uart_id].dmarx_buf_size = sizeof(s_uart3_dmarx_buf);
		uart3_dmarx_config(s_uart_dev[uart_id].dmarx_buf, 
							   sizeof(s_uart3_dmarx_buf));/* 只需配置接收模式DMA，发送模式需发送数据时才配置 */
		s_uart_dev[uart_id].status  = 0;
	}
	else if (uart_id == DEV_UART2)
	{
		uart2_init();
		/* 配置串口2收发fifo */
		fifo_register(&s_uart_dev[uart_id].rx_fifo, &s_uart2_rx_buf[0], 
                      sizeof(s_uart2_rx_buf), fifo_lock, fifo_unlock);
		
		/* 配置串口2 DMA收发buf */
		s_uart_dev[uart_id].dmarx_buf = &s_uart2_dmarx_buf[0];
		s_uart_dev[uart_id].dmarx_buf_size = sizeof(s_uart2_dmarx_buf);
		uart2_dmarx_config(s_uart_dev[uart_id].dmarx_buf, 
							   sizeof(s_uart2_dmarx_buf));/* 只需配置接收模式DMA，发送模式需发送数据时才配置 */
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
//	return fifo_write(&s_uart_dev[uart_id].tx_fifo, buf, size);
	if(uart_id == DEV_UART3){
		for(uint16_t i=0;i<size;i++)
		{
			USART_SendData(USART3, *(buf+i));
    		while(USART_GetFlagStatus(USART3, USART_FLAG_TXDE) == RESET);
		}

		return size;
	}else if(uart_id == DEV_UART2){
		for(uint16_t i=0;i<size;i++)
		{
			USART_SendData(USART2, *(buf+i));
    		while(USART_GetFlagStatus(USART2, USART_FLAG_TXDE) == RESET);
		}

		return size;
	}else{
		return 0;
	}
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
 * @brief  串口dma接收缓存大小一半中断处理
 * @param  
 * @retval 
 */
static void uart_dmarx_half_done_isr(uint8_t uart_id)
{
  	uint16_t recv_total_size;
  	uint16_t recv_size;
	
	if(uart_id == DEV_UART3)
	{
		recv_total_size = s_uart_dev[uart_id].dmarx_buf_size - uart3_get_dmarx_buf_remain_size();
	}
	else if(uart_id == DEV_UART2)
	{
		recv_total_size = s_uart_dev[uart_id].dmarx_buf_size - uart2_get_dmarx_buf_remain_size();
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

	if(uart_id == DEV_UART3)
	{
	  	recv_total_size = s_uart_dev[uart_id].dmarx_buf_size - uart3_get_dmarx_buf_remain_size();
	}
	else if(uart_id == DEV_UART2)
	{
		recv_total_size = s_uart_dev[uart_id].dmarx_buf_size - uart2_get_dmarx_buf_remain_size();
	}
	recv_size = recv_total_size - s_uart_dev[uart_id].last_dmarx_size;
	s_UartTxRxCount[uart_id*2+1] += recv_size;
	fifo_write(&s_uart_dev[uart_id].rx_fifo, 
				   (const uint8_t *)&(s_uart_dev[uart_id].dmarx_buf[s_uart_dev[uart_id].last_dmarx_size]), recv_size);
	s_uart_dev[uart_id].last_dmarx_size = recv_total_size;
}

void USART3_IRQHandler(void)
{
	if(USART_GetIntStatus(USART3, USART_INT_IDLEF) != RESET)
	{
		uart_dmarx_idle_isr(DEV_UART3);
		/* read the data to clear receive idle interrupt flag */
		USART_ReceiveData(USART3);
	}
}

void DMA_Channel6_IRQHandler(void)
{
	if(DMA_GetIntStatus(DMA_INT_TXC6, DMA))
	{
		uart_dmarx_done_isr(DEV_UART3);
		DMA_ClrIntPendingBit(DMA_INT_TXC6, DMA);
	}
	if(DMA_GetIntStatus(DMA_INT_HTX6, DMA))
	{
		uart_dmarx_half_done_isr(DEV_UART3);
		DMA_ClrIntPendingBit(DMA_INT_HTX6, DMA);
	}
}

void USART2_IRQHandler(void)
{
	if(USART_GetIntStatus(USART2, USART_INT_IDLEF) != RESET)
	{
		uart_dmarx_idle_isr(DEV_UART2);
		/* read the data to clear receive idle interrupt flag */
		USART_ReceiveData(USART2);
	}
}

void DMA_Channel4_IRQHandler(void)
{
	if(DMA_GetIntStatus(DMA_INT_TXC4, DMA))
	{
		uart_dmarx_done_isr(DEV_UART2);
		DMA_ClrIntPendingBit(DMA_INT_TXC4, DMA);
	}
	if(DMA_GetIntStatus(DMA_INT_HTX4, DMA))
	{
		uart_dmarx_half_done_isr(DEV_UART2);
		DMA_ClrIntPendingBit(DMA_INT_HTX4, DMA);
	}
}

static int uart_dev_register(void)
{
	uart_device_init(DEV_UART3);
	return 0;
}
INIT_PREV_EXPORT(uart_dev_register);

//一定要先初始化ADC，再初始化uart2，貌似N32L40x库函数有BUG！！！
extern void $Super$$ADC_RegConfig(void);
void $Sub$$ADC_RegConfig(void)
{
	$Super$$ADC_RegConfig();
	uart_device_init(DEV_UART2);
}

