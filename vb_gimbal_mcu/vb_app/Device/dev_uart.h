#ifndef __DEV_UART_H__
#define __DEV_UART_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ddl.h"
#include "fifo.h"
#include "drv_uart.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/* ���ڻ����С */
#define UART1_TX_BUF_SIZE           (2048)	
#define UART1_RX_BUF_SIZE           (2048)	
#define	UART1_DMA_RX_BUF_SIZE		(1024)	//1:4��Ϊ����
#define	UART1_DMA_TX_BUF_SIZE		(1024)	

/* �����豸���ݽṹ */
typedef struct
{
	uint8_t status;				/* ����״̬ */
	_fifo_t tx_fifo;			/* ����fifo */
	_fifo_t rx_fifo;			/* ����fifo */
	uint8_t *dmarx_buf;			/* dma���ջ��� */
	uint16_t dmarx_buf_size;	/* dma���ջ����С*/
	uint8_t *dmatx_buf;			/* dma���ͻ��� */
	uint16_t dmatx_buf_size;	/* dma���ͻ����С */
	uint16_t last_dmarx_size;	/* dma��һ�ν������ݴ�С */
}uart_device_t;

#define DEV_UART1	0

void uart_device_init(uint8_t uart_id);
uint16_t uart_write(uint8_t uart_id, const uint8_t *buf, uint16_t size);
uint16_t uart_read(uint8_t uart_id, uint8_t *buf, uint16_t size);
void uart_poll_dma_tx(uint8_t uart_id);


#ifdef __cplusplus
}
#endif

#endif 



