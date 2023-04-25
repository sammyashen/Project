#ifndef __DRV_HW_SPI_H__
#define __DRV_HW_SPI_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "n32l40x.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

#define SPI1_CS_ENABLE		GPIO_ResetBits(GPIOB, GPIO_PIN_6)
#define SPI1_CS_DISABLE		GPIO_SetBits(GPIOB, GPIO_PIN_6)

void spi_hw_init(void);
uint8_t spi1_4wire_trans_byte(uint8_t byte);
void spi2_dma_tx_config(uint8_t *mem_addr, uint32_t mem_size);
void spi2_dma_rx_config(uint8_t *mem_addr, uint32_t mem_size);

#ifdef __cplusplus
}
#endif

#endif 




