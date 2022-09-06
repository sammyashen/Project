#ifndef __DRI_UART_H
#define __DRI_UART_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "n32g45x.h"

extern void bsp_uart1_init(void);
extern void bsp_uart1_dmatx_config(uint8_t *mem_addr, uint32_t mem_size);
extern void bsp_uart1_dmarx_config(uint8_t *mem_addr, uint32_t mem_size);
extern uint16_t bsp_uart1_get_dmarx_buf_remain_size(void);

#endif



