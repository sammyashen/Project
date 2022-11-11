#ifndef _DRV_UART_H_
#define _DRV_UART_H_

#include "n32l40x.h"

void uart1_init(void);
void uart1_dmatx_config(uint8_t *mem_addr, uint32_t mem_size);
void uart1_dmarx_config(uint8_t *mem_addr, uint32_t mem_size);
uint16_t uart1_get_dmarx_buf_remain_size(void);

void uart4_init(void);
void uart4_dmatx_config(uint8_t *mem_addr, uint32_t mem_size);
void uart4_dmarx_config(uint8_t *mem_addr, uint32_t mem_size);
uint16_t uart4_get_dmarx_buf_remain_size(void);


#endif


