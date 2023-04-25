#ifndef __DRV_UART_H__
#define __DRV_UART_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "n32l40x.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

void uart3_init(void);
void uart3_dmarx_config(uint8_t *mem_addr, uint32_t mem_size);
uint16_t uart3_get_dmarx_buf_remain_size(void);
void uart2_init(void);
void uart2_dmarx_config(uint8_t *mem_addr, uint32_t mem_size);
uint16_t uart2_get_dmarx_buf_remain_size(void);

#ifdef __cplusplus
}
#endif

#endif 


