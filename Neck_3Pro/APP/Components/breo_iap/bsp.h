#ifndef __BSP_H__
#define __BSP_H__
#include "n32l40x.h"

#define USARTy            USART1
#define USARTy_GPIO       GPIOB
#define USARTy_CLK        RCC_APB2_PERIPH_USART1
#define USARTy_GPIO_CLK   RCC_APB2_PERIPH_GPIOB
#define USARTy_RxPin      GPIO_PIN_7
#define USARTy_TxPin      GPIO_PIN_6
#define USARTy_Rx_GPIO_AF GPIO_AF0_USART1
#define USARTy_Tx_GPIO_AF GPIO_AF0_USART1
#define USARTy_APBxClkCmd RCC_EnableAPB2PeriphClk
#define USARTy_IRQn       USART1_IRQn
#define USARTy_IRQHandler USART1_IRQHandler


void bsp_init (void);
void flash_init(void);

#endif
