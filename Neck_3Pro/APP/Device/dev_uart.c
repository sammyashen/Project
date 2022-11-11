#include "dev_uart.h"

/* ���ڻ��� */
static uint8_t s_uart1_tx_buf[UART1_TX_BUF_SIZE];
static uint8_t s_uart1_rx_buf[UART1_RX_BUF_SIZE];
static uint8_t s_uart1_dmarx_buf[UART1_DMA_RX_BUF_SIZE];
static uint8_t s_uart1_dmatx_buf[UART1_DMA_TX_BUF_SIZE];

static uint8_t s_uart4_tx_buf[UART4_TX_BUF_SIZE];
static uint8_t s_uart4_rx_buf[UART4_RX_BUF_SIZE];
static uint8_t s_uart4_dmarx_buf[UART4_DMA_RX_BUF_SIZE];
static uint8_t s_uart4_dmatx_buf[UART4_DMA_TX_BUF_SIZE];

/* �����豸ʵ�� */
static uart_device_t s_uart_dev[2] = {0};
uint32_t s_UartTxRxCount[4] = {0};

/* fifo�������� */
//static void fifo_lock(void)
//{
//    
//}

/* fifo�������� */
//static void fifo_unlock(void)
//{
//    
//}

/**
 * @brief �����豸��ʼ�� 
 * @param  
 * @retval 
 */
static void uart_device_init(uint8_t uart_id)
{
  	if (uart_id == 0)
	{
		uart1_init();
		/* ���ô���1�շ�fifo */
		fifo_register(&s_uart_dev[uart_id].tx_fifo, &s_uart1_tx_buf[0], 
                      sizeof(s_uart1_tx_buf), NULL, NULL);
		fifo_register(&s_uart_dev[uart_id].rx_fifo, &s_uart1_rx_buf[0], 
                      sizeof(s_uart1_rx_buf), NULL, NULL);
		
		/* ���ô���1 DMA�շ�buf */
		s_uart_dev[uart_id].dmarx_buf = &s_uart1_dmarx_buf[0];
		s_uart_dev[uart_id].dmarx_buf_size = sizeof(s_uart1_dmarx_buf);
		s_uart_dev[uart_id].dmatx_buf = &s_uart1_dmatx_buf[0];
		s_uart_dev[uart_id].dmatx_buf_size = sizeof(s_uart1_dmatx_buf);
		uart1_dmarx_config(s_uart_dev[uart_id].dmarx_buf, 
							   sizeof(s_uart1_dmarx_buf));/* ֻ�����ý���ģʽDMA������ģʽ�跢������ʱ������ */
		s_uart_dev[uart_id].status  = 0;
	}
	else if (uart_id == 1)
	{
		uart4_init();
		/* ���ô���1�շ�fifo */
		fifo_register(&s_uart_dev[uart_id].tx_fifo, &s_uart4_tx_buf[0], 
                      sizeof(s_uart4_tx_buf), NULL, NULL);
		fifo_register(&s_uart_dev[uart_id].rx_fifo, &s_uart4_rx_buf[0], 
                      sizeof(s_uart4_rx_buf), NULL, NULL);
		
		/* ���ô���1 DMA�շ�buf */
		s_uart_dev[uart_id].dmarx_buf = &s_uart4_dmarx_buf[0];
		s_uart_dev[uart_id].dmarx_buf_size = sizeof(s_uart4_dmarx_buf);
		s_uart_dev[uart_id].dmatx_buf = &s_uart4_dmatx_buf[0];
		s_uart_dev[uart_id].dmatx_buf_size = sizeof(s_uart4_dmatx_buf);
		uart4_dmarx_config(s_uart_dev[uart_id].dmarx_buf, 
							   sizeof(s_uart4_dmarx_buf));/* ֻ�����ý���ģʽDMA������ģʽ�跢������ʱ������ */
		s_uart_dev[uart_id].status  = 0;
	}
}

/**
 * @brief  ���ڷ������ݽӿڣ�ʵ����д�뷢��fifo��������dma����
 * @param  
 * @retval 
 */
uint16_t uart_write(uint8_t uart_id, const uint8_t *buf, uint16_t size)
{
	return fifo_write(&s_uart_dev[uart_id].tx_fifo, buf, size);
}

/**
 * @brief  ���ڶ�ȡ���ݽӿڣ�ʵ���Ǵӽ���fifo��ȡ
 * @param  
 * @retval 
 */
uint16_t uart_read(uint8_t uart_id, uint8_t *buf, uint16_t size)
{
	return fifo_read(&s_uart_dev[uart_id].rx_fifo, buf, size);
}

/**
 * @brief  ����dma��������жϴ���
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
 * @brief  ����dma���ջ����Сһ�������жϴ���
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
 * @brief  ���ڿ����жϴ���
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
 * @brief  ����dma��������жϴ���
 * @param  
 * @retval 
 */
static void uart_dmatx_done_isr(uint8_t uart_id)
{
 	s_uart_dev[uart_id].status = 0;	/* DMA���Ϳ��� */
}

/**
 * @brief  ѭ���Ӵ��ڷ���fifo�������ݣ�������dma���ͻ��棬������dma����
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
            s_uart_dev[uart_id].status = 0x01;	/* DMA����״̬ */
		  	uart1_dmatx_config(s_uart_dev[uart_id].dmatx_buf, size);
		}
		else if(uart_id == 1)
		{
			s_uart_dev[uart_id].status = 0x01;	/* DMA����״̬ */
		  	uart4_dmatx_config(s_uart_dev[uart_id].dmatx_buf, size);
		}
	}
}

/**
 * @brief  ����1�жϴ���
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
  * @brief	DMA�����жϴ���
  */
 void DMA_Channel1_IRQHandler(void)
{
	if(DMA_GetIntStatus(DMA_INT_TXC1, DMA))
	{
		uart_dmatx_done_isr(DEV_UART1);
//		DMA_ClearFlag(DMA_FLAG_TC1, DMA);
		DMA_ClrIntPendingBit(DMA_INT_TXC1, DMA);
        DMA_EnableChannel(DMA_CH1, DISABLE);
	}
}

 /**
  * @brief	DMA�����жϴ���
  */
 void DMA_Channel2_IRQHandler(void)
 {
	 if(DMA_GetIntStatus(DMA_INT_TXC2, DMA))
	 {
		 uart_dmarx_done_isr(DEV_UART1);
//		 DMA_ClearFlag(DMA_FLAG_TC2, DMA);
		 DMA_ClrIntPendingBit(DMA_INT_TXC2, DMA);
	 }
	 if(DMA_GetIntStatus(DMA_INT_HTX2, DMA))
	 {
		 uart_dmarx_half_done_isr(DEV_UART1);
//		 DMA_ClearFlag(DMA_FLAG_HT2, DMA);
		 DMA_ClrIntPendingBit(DMA_INT_HTX2, DMA);
	 }
 }

 /**
 * @brief	����4�жϴ���
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
 * @brief	DMA�����жϴ���
 */
 void DMA_Channel7_IRQHandler(void)
{
	if(DMA_GetIntStatus(DMA_INT_TXC7, DMA))
	{
		uart_dmatx_done_isr(DEV_UART4);
		DMA_ClearFlag(DMA_FLAG_TC7, DMA);
        DMA_EnableChannel(DMA_CH7, DISABLE);
	}
}

/**
 * @brief	DMA�����жϴ���
 */
 void DMA_Channel8_IRQHandler(void)
 {
	 if(DMA_GetIntStatus(DMA_INT_TXC8, DMA))
	 {
		 uart_dmarx_done_isr(DEV_UART4);
		 DMA_ClearFlag(DMA_FLAG_TC8, DMA);
	 }
	 if(DMA_GetIntStatus(DMA_INT_HTX8, DMA))
	 {
		 uart_dmarx_half_done_isr(DEV_UART4);
		 DMA_ClearFlag(DMA_FLAG_HT8, DMA);
	 }
 }

static void uart_device_register(void)
{
	uart_device_init(DEV_UART1);
	uart_device_init(DEV_UART4);
}

debug_initcall(uart_device_register);

