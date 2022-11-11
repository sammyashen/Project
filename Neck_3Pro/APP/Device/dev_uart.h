#ifndef _DEV_UART_H_
#define _DEV_UART_H_

#include "n32l40x.h"
#include <stdint.h>
#include <stdbool.h>
#include "fifo.h"
#include "init.h"
#include "tiny_os.h"
#include "drv_uart.h"

/* 串口缓存大小 */
#define UART1_TX_BUF_SIZE           (512)	
#define UART1_RX_BUF_SIZE           (512)	
#define	UART1_DMA_RX_BUF_SIZE		(256)	
#define	UART1_DMA_TX_BUF_SIZE		(256)

#define UART4_TX_BUF_SIZE           (512)	
#define UART4_RX_BUF_SIZE           (512)	
#define	UART4_DMA_RX_BUF_SIZE		(256)	
#define	UART4_DMA_TX_BUF_SIZE		(256)


/* 串口设备数据结构 */
typedef struct
{
	uint8_t status;				/* 发送状态 */
	_fifo_t tx_fifo;			/* 发送fifo */
	_fifo_t rx_fifo;			/* 接收fifo */
	uint8_t *dmarx_buf;			/* dma接收缓存 */
	uint16_t dmarx_buf_size;	/* dma接收缓存大小*/
	uint8_t *dmatx_buf;			/* dma发送缓存 */
	uint16_t dmatx_buf_size;	/* dma发送缓存大小 */
	uint16_t last_dmarx_size;	/* dma上一次接收数据大小 */
}uart_device_t;

#define DEV_UART1	0
#define DEV_UART4	1

uint16_t uart_write(uint8_t uart_id, const uint8_t *buf, uint16_t size);
uint16_t uart_read(uint8_t uart_id, uint8_t *buf, uint16_t size);
void uart_poll_dma_tx(uint8_t uart_id);



#endif



