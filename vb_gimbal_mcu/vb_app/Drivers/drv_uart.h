#ifndef __DRV_UART_H__
#define __DRV_UART_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ddl.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

void uart1_init(void);
void uart1_tx_dma_config(uint8_t *mem_addr, uint32_t mem_size);
void uart1_rx_dma_config(uint8_t *mem_addr, uint32_t mem_size);

#ifdef __cplusplus
}
#endif

#endif 


