#ifndef _DEV_UART_H_
#define _DEV_UART_H_

#include "n32l40x.h"
#include <stdint.h>
#include <stdbool.h>
#include "fifo.h"
#include "init.h"
#include "tiny_os.h"
#include "drv_uart.h"

/* ���ڻ����С */
#define UART1_TX_BUF_SIZE           (512)	
#define UART1_RX_BUF_SIZE           (512)	
#define	UART1_DMA_RX_BUF_SIZE		(256)	
#define	UART1_DMA_TX_BUF_SIZE		(256)

#define UART4_TX_BUF_SIZE           (512)	
#define UART4_RX_BUF_SIZE           (512)	
#define	UART4_DMA_RX_BUF_SIZE		(256)	
#define	UART4_DMA_TX_BUF_SIZE		(256)


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
#define DEV_UART4	1

uint16_t uart_write(uint8_t uart_id, const uint8_t *buf, uint16_t size);
uint16_t uart_read(uint8_t uart_id, uint8_t *buf, uint16_t size);
void uart_poll_dma_tx(uint8_t uart_id);



#endif



