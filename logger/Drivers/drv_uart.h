#ifndef _DRV_UART_H_
#define _DRV_UART_H_

#include "stm32f4xx.h"

void uart1_init(void);
void uart1_dmatx_config(uint8_t *mem_addr, uint32_t mem_size);
void uart1_dmarx_config(uint8_t *mem_addr, uint32_t mem_size);
uint16_t uart1_get_dmarx_buf_remain_size(void);
void uart6_init(void);
void uart6_dmatx_config(uint8_t *mem_addr, uint32_t mem_size);
void uart6_dmarx_config(uint8_t *mem_addr, uint32_t mem_size);
uint16_t uart6_get_dmarx_buf_remain_size(void);


#endif


