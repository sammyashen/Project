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

/* 串口缓存大小 */
#define UART1_TX_BUF_SIZE           (2048)	
#define UART1_RX_BUF_SIZE           (2048)	
#define	UART1_DMA_RX_BUF_SIZE		(1024)	//1:4较为合适
#define	UART1_DMA_TX_BUF_SIZE		(1024)	

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

void uart_device_init(uint8_t uart_id);
uint16_t uart_write(uint8_t uart_id, const uint8_t *buf, uint16_t size);
uint16_t uart_read(uint8_t uart_id, uint8_t *buf, uint16_t size);
void uart_poll_dma_tx(uint8_t uart_id);


#ifdef __cplusplus
}
#endif

#endif 



