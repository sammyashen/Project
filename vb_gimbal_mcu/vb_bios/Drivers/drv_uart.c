#include "drv_uart.h"

static void tim01_init(void)
{
	stc_clk_freq_t stcClkTmp;
    stc_tim0_base_init_t stcTimerCfg;
    stc_tim0_trigger_init_t StcTimer0TrigInit;
	stc_clk_freq_t stcClk_t;

	MEM_ZERO_STRUCT(stcClkTmp);
    MEM_ZERO_STRUCT(stcTimerCfg);
    MEM_ZERO_STRUCT(StcTimer0TrigInit);
	/* 获取pclk1时钟频率 */
	CLK_GetClockFreq(&stcClk_t);
	/* 使能内部低速时钟LRC(32.768K) */
	CLK_LrcCmd(Enable);
	/* 开启定时器0时钟 */
	PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM01, Enable);
	/* Clear CNTAR register for channel A */
    TIMER0_WriteCntReg(M4_TMR01, Tim0_ChannelA, 0u);
#if 1
	/* 配置同步计数模式 */
	stcTimerCfg.Tim0_CounterMode = Tim0_Sync;
	stcTimerCfg.Tim0_SyncClockSource = Tim0_Pclk1;	//100MHz
    stcTimerCfg.Tim0_ClockDivision = Tim0_ClkDiv4;
    stcTimerCfg.Tim0_CmpValue = (uint16_t)((stcClk_t.pclk1Freq/4/1000)*5);	//约5ms
#else
	/* 配置异步计数模式 */
	stcTimerCfg.Tim0_CounterMode = Tim0_Async;
    stcTimerCfg.Tim0_AsyncClockSource = Tim0_LRC;	//32.768KHz
    stcTimerCfg.Tim0_ClockDivision = Tim0_ClkDiv0;
    stcTimerCfg.Tim0_CmpValue = (uint16_t)((32768/1000)*1);//约1ms
#endif
	TIMER0_BaseInit(M4_TMR01, Tim0_ChannelA, &stcTimerCfg);
	/* 清除标志位 */
	TIMER0_ClearFlag(M4_TMR01, Tim0_ChannelA);
	/* Config timer0 hardware trigger */
    StcTimer0TrigInit.Tim0_InTrigEnable = false;
    StcTimer0TrigInit.Tim0_InTrigClear = true;
    StcTimer0TrigInit.Tim0_InTrigStart = true;
    StcTimer0TrigInit.Tim0_InTrigStop = false;
    TIMER0_HardTriggerInit(M4_TMR01, Tim0_ChannelA, &StcTimer0TrigInit);
}

static void usart1_err_irq_cb(void)
{
	if(Set == USART_GetStatus(M4_USART1, UsartFrameErr)){
        USART_ClearStatus(M4_USART1, UsartFrameErr);
    }

    if(Set == USART_GetStatus(M4_USART1, UsartParityErr)){
        USART_ClearStatus(M4_USART1, UsartParityErr);
    }

    if(Set == USART_GetStatus(M4_USART1, UsartOverrunErr)){
        USART_ClearStatus(M4_USART1, UsartOverrunErr);
    }
}

extern void uart1_idle_irq_handler(void);
static void usart1_rto_irq_cb(void)
{
	TIMER0_Cmd(M4_TMR01, Tim0_ChannelA, Disable);
    USART_ClearStatus(M4_USART1, UsartRxTimeOut);
	uart1_idle_irq_handler();
}

void uart1_init(void)
{
	stc_irq_regi_conf_t stcIrqRegiCfg;
    const stc_usart_uart_init_t stcInitCfg = {
        UsartIntClkCkOutput,		//使用内部时钟源，输出通道的时钟信号
        UsartClkDiv_1,
        UsartDataBits8,
        UsartDataLsbFirst,
        UsartOneStopBit,
        UsartParityNone,
        UsartSampleBit8,
        UsartStartBitFallEdge,
        UsartRtsEnable,
    };

	tim01_init();
	
	/* 开启USART1时钟 */
	PWC_Fcg1PeriphClockCmd(PWC_FCG1_PERIPH_USART1, Enable);
	/* 映射GPIO功能 */
//	PORT_SetFunc(PortA, Pin10, Func_Usart1_Rx, Disable);
//    PORT_SetFunc(PortA, Pin09, Func_Usart1_Tx, Disable);
	PORT_DebugPortSetting(TDI, Disable);	//disable JTDI
	PORT_SetFunc(PortA, Pin06, Func_Usart1_Rx, Disable);
    PORT_SetFunc(PortA, Pin15, Func_Usart1_Tx, Disable);
	/* 初始化USART1 */
	USART_UART_Init(M4_USART1, &stcInitCfg);
	/* 设置波特率 */
	USART_SetBaudrate(M4_USART1, 921600);
	/* 设置接收错误中断 */
	stcIrqRegiCfg.enIRQn = Int000_IRQn;
    stcIrqRegiCfg.pfnCallback = &usart1_err_irq_cb;
    stcIrqRegiCfg.enIntSrc = INT_USART1_EI;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
	/* 设置超时中断中断 */
    stcIrqRegiCfg.enIRQn = Int003_IRQn;
    stcIrqRegiCfg.pfnCallback = &usart1_rto_irq_cb;
    stcIrqRegiCfg.enIntSrc = INT_USART1_RTO;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
	/* 开启超时中断 */
	USART_FuncCmd(M4_USART1, UsartTimeOut, Enable);
	USART_FuncCmd(M4_USART1, UsartTimeOutInt, Enable);
}

static uint32_t rx_dma_size = 0;
static uint8_t *rx_dma_addr = NULL;
extern void uart1_rx_done_irq_handler(void);
static void dma2_ch0_tc_irq_cb(void)
{
    DMA_ClearIrqFlag(M4_DMA2, DmaCh0, TrnCpltIrq);
	DMA_ChannelCmd(M4_DMA2, DmaCh0, Disable);
	DMA_SetTransferCnt(M4_DMA2, DmaCh0, (uint16_t)rx_dma_size);
	DMA_SetDesAddress(M4_DMA2, DmaCh0, (uint32_t)rx_dma_addr);
	DMA_ChannelCmd(M4_DMA2, DmaCh0, Enable);
	uart1_rx_done_irq_handler();
}

void uart1_rx_dma_config(uint8_t *mem_addr, uint32_t mem_size)
{
	stc_dma_config_t stcDmaInit;
    stc_irq_regi_conf_t stcIrqRegiCfg;

	rx_dma_addr = mem_addr;
	rx_dma_size = mem_size;
    /* 开启DMA2时钟 */
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_DMA2,Enable);
    /* 使能DMA2. */
    DMA_Cmd(M4_DMA2, Enable);
    /* 初始化DMA2结构体. */
    MEM_ZERO_STRUCT(stcDmaInit);
    stcDmaInit.u16BlockSize = 1u; 								/* 1 block */
	stcDmaInit.u16TransferCnt = (uint16_t)mem_size;				/* Transfer count */
    stcDmaInit.u32SrcAddr = ((uint32_t)(&M4_USART1->DR)+2ul); 	/* Set source address. */
    stcDmaInit.u32DesAddr = (uint32_t)mem_addr;     			/* Set destination address. */
    stcDmaInit.stcDmaChCfg.enSrcInc = AddressFix;  				/* Set source address mode. */
    stcDmaInit.stcDmaChCfg.enDesInc = AddressIncrease;  		/* Set destination address mode. */
    stcDmaInit.stcDmaChCfg.enIntEn = Enable;       				/* Enable interrupt. */
    stcDmaInit.stcDmaChCfg.enTrnWidth = Dma8Bit;   				/* Set data width 8bit. */
    DMA_InitChannel(M4_DMA2, DmaCh0, &stcDmaInit);
    /* Enable the specified DMA channel. */
    DMA_ChannelCmd(M4_DMA2, DmaCh0, Enable);
    /* Clear DMA flag. */
    DMA_ClearIrqFlag(M4_DMA2, DmaCh0, TrnCpltIrq);
    /* Enable peripheral circuit trigger function. */
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_AOS,Enable);
    /* Set DMA trigger source. */
    DMA_SetTriggerSrc(M4_DMA2, DmaCh0, EVT_USART1_RI);

    /* 设置中断 */
    stcIrqRegiCfg.enIRQn = Int002_IRQn;
    stcIrqRegiCfg.pfnCallback = &dma2_ch0_tc_irq_cb;
    stcIrqRegiCfg.enIntSrc = INT_DMA2_TC0;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
	/* 开启dma传输 */
	USART_FuncCmd(M4_USART1, UsartRx, Enable);
}

extern void uart1_tx_done_irq_handler(void);
static void dma1_ch0_tc_irq_cb(void)
{
	DMA_ClearIrqFlag(M4_DMA1, DmaCh0, TrnCpltIrq);
	while(!USART_GetStatus(M4_USART1, UsartTxComplete));
	uart1_tx_done_irq_handler();
	USART_FuncCmd(M4_USART1, UsartTxAndTxEmptyInt, Disable);
}

void uart1_tx_dma_config(uint8_t *mem_addr, uint32_t mem_size)
{
	stc_dma_config_t stcDmaInit;
    stc_irq_regi_conf_t stcIrqRegiCfg;

	/* 开启DMA1时钟 */
	PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_DMA1, Enable);
	/* 使能DMA1 */
	DMA_Cmd(M4_DMA1, Enable);
	/* 配置DMA1结构体 */
	MEM_ZERO_STRUCT(stcDmaInit);
    stcDmaInit.u16BlockSize = 1u; 									/* 1 block */
    stcDmaInit.u16TransferCnt = (uint16_t)mem_size;					/* Transfer count */
    stcDmaInit.u32SrcAddr = (uint32_t)mem_addr;  					/* Set source address. */
    stcDmaInit.u32DesAddr = (uint32_t)(&M4_USART1->DR);   			/* Set destination address. */
    stcDmaInit.stcDmaChCfg.enSrcInc = AddressIncrease;   			/* Set source address mode. */
    stcDmaInit.stcDmaChCfg.enDesInc = AddressFix;        			/* Set destination address mode. */
    stcDmaInit.stcDmaChCfg.enIntEn = Enable;             			/* Enable interrupt. */
    stcDmaInit.stcDmaChCfg.enTrnWidth = Dma8Bit;         			/* Set data width 8bit. */
    DMA_InitChannel(M4_DMA1, DmaCh0, &stcDmaInit);
	/* Enable the specified DMA channel. */
    DMA_ChannelCmd(M4_DMA1, DmaCh0, Enable);
    /* Clear DMA flag. */
    DMA_ClearIrqFlag(M4_DMA1, DmaCh0, TrnCpltIrq);
    /* Enable peripheral circuit trigger function. */
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_AOS,Enable);
    /* Set DMA trigger source. */
    DMA_SetTriggerSrc(M4_DMA1, DmaCh0, EVT_USART1_TI);
	/* 设置传输完成中断 */
	stcIrqRegiCfg.enIRQn = Int001_IRQn;
    stcIrqRegiCfg.pfnCallback = &dma1_ch0_tc_irq_cb;
    stcIrqRegiCfg.enIntSrc = INT_DMA1_TC0;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
	/* 开启dma传输 */
	USART_FuncCmd(M4_USART1, UsartTxAndTxEmptyInt, Enable);
}

