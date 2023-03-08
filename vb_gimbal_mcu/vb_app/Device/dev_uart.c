#include "dev_uart.h"
#include "init.h"

/* ���ڻ��� */
static uint8_t s_uart1_tx_buf[UART1_TX_BUF_SIZE];
static uint8_t s_uart1_rx_buf[UART1_RX_BUF_SIZE];
static uint8_t s_uart1_dmarx_buf[UART1_DMA_RX_BUF_SIZE];
static uint8_t s_uart1_dmatx_buf[UART1_DMA_TX_BUF_SIZE];

/* �����豸ʵ�� */
static uart_device_t s_uart_dev[1] = {0};
uint32_t s_UartTxRxCount[2] = {0};

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
void uart_device_init(uint8_t uart_id)
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
		uart1_rx_dma_config(s_uart_dev[uart_id].dmarx_buf, sizeof(s_uart1_dmarx_buf));
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
	  	recv_total_size = s_uart_dev[uart_id].dmarx_buf_size - DMA_GetTransferCnt(M4_DMA2, DmaCh0);
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
		  	uart1_tx_dma_config(s_uart_dev[uart_id].dmatx_buf, size);
		}
	}
}

void uart1_idle_irq_handler(void)
{
	uart_dmarx_idle_isr(DEV_UART1);
}

void uart1_tx_done_irq_handler(void)
{
	uart_dmatx_done_isr(DEV_UART1);
}

void uart1_rx_done_irq_handler(void)
{
	uart_dmarx_done_isr(DEV_UART1);
}

static void uart_device_register(void)
{
	uart_device_init(DEV_UART1);
}

debug_initcall(uart_device_register);

