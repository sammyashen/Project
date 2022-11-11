#include "bsp.h"
#include "iap_addr_def.h"
#include "upgrade_format_def.h"
#include "stdio.h"

static void RCC_Configuration(void)
{
    RCC_EnableAPB2PeriphClk(USARTy_GPIO_CLK | RCC_APB2_PERIPH_AFIO, ENABLE);
    USARTy_APBxClkCmd(USARTy_CLK, ENABLE);
}

static void uart1_init (uint32_t BaudRate)
{
    GPIO_InitType GPIO_InitStructure;
    USART_InitType USART_InitStructure;
//    NVIC_InitType NVIC_InitStructure;

	/* Initialize GPIO_InitStructure */
    GPIO_InitStruct(&GPIO_InitStructure);

    /* Configure USARTx Tx as alternate function push-pull */
    GPIO_InitStructure.Pin            = USARTy_TxPin;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Alternate = USARTy_Tx_GPIO_AF;
    GPIO_InitPeripheral(USARTy_GPIO, &GPIO_InitStructure);
    
    /* Configure USARTx Rx as alternate function push-pull and pull-up */
    GPIO_InitStructure.Pin            = USARTy_RxPin;
    GPIO_InitStructure.GPIO_Pull      = GPIO_Pull_Up;
    GPIO_InitStructure.GPIO_Alternate = USARTy_Rx_GPIO_AF;
    GPIO_InitPeripheral(USARTy_GPIO, &GPIO_InitStructure);

	USART_DeInit(USART1);
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.BaudRate            = BaudRate;
    USART_InitStructure.WordLength          = USART_WL_8B;
    USART_InitStructure.StopBits            = USART_STPB_1;
    USART_InitStructure.Parity              = USART_PE_NO;
    USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
    USART_InitStructure.Mode                = USART_MODE_RX | USART_MODE_TX;

    /* Configure USARTy */
    USART_Init(USARTy, &USART_InitStructure);

//    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
//    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);

//    USART_ConfigInt(USARTy, USART_FLAG_RXDNE, ENABLE);
//    USART_ClrFlag(USARTy, USART_FLAG_TXC);
    USART_Enable(USARTy, ENABLE);
}

static void flash_init(void)
{
	if(FLASH_HSICLOCK_DISABLE == FLASH_ClockInit())
    {
        printf("HSI oscillator not yet ready\r\n");
        while(1);
    }
}

void bsp_init ()
{
    RCC_Configuration();
    uart1_init(115200);
	flash_init();		
}


static int is_lr_sent = 0;

int fputc(int ch, FILE* f)
{
    if (ch == '\r')
    {
        is_lr_sent = 1;
    }
    else if (ch == '\n')
    {
        if (!is_lr_sent)
        {
            USART_SendData(USARTy, (uint8_t)'\r');
            /* Loop until the end of transmission */
            while (USART_GetFlagStatus(USARTy, USART_FLAG_TXDE) == RESET)
            {
            }
        }
        is_lr_sent = 0;
    }
    else
    {
        is_lr_sent = 0;
    }
    USART_SendData(USARTy, (uint8_t)ch);
    /* Loop until the end of transmission */
    while (USART_GetFlagStatus(USARTy, USART_FLAG_TXDE) == RESET)
    {
    }
    return ch;
}
