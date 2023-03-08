#include "rtthread.h"
#include "hc32_ddl.h"
#include "dev_uart.h"

#define LOG_TAG				"high_speed"
#define LOG_LVL				LOG_LVL_DBG
#define ULOG_OUTPUT_LVL		LOG_LVL_DBG
#include <rtdbg.h>

static rt_sem_t hs_sem = RT_NULL;

static void hs_comm_start(void)
{
	rt_err_t ret = RT_EOK;

	hs_sem = rt_sem_create("hs_sem", 1, RT_IPC_FLAG_FIFO);
	ret = rt_sem_trytake(hs_sem);
	if(ret == RT_EOK){
		LOG_I("hs communicate start.");
		TIMERA_Cmd(M4_TMRA1, Enable);
		rt_sem_take(hs_sem, RT_WAITING_FOREVER);
		LOG_I("hs communicate end.");
		rt_sem_delete(hs_sem);
	}else{
		LOG_E("hs_sem create failed.");
	}
}
MSH_CMD_EXPORT(hs_comm_start, high speed communicate test);

static void hs_process(void)
{
	rt_uint16_t size = 0;
	rt_uint8_t buf[16] = {0};
	rt_uint8_t buf_tx[64] = {0};

	size = uart_read(DEV_UART1, &buf[0], sizeof(buf));
	if(size != 0){
		if(!rt_strcmp((char *)buf, "hs_stop\r\n")){
			TIMERA_Cmd(M4_TMRA1, Disable);
			rt_sem_release(hs_sem);
		}
	}

	rt_memset(buf_tx, 0, sizeof(buf_tx));
	rt_sprintf((char *)buf_tx, "[%d] hello.\r\n", rt_tick_get());
	uart_write(DEV_UART1, buf_tx, rt_strlen((char *)buf_tx));
}

static void TimeraUnit1_IrqCallback(void)
{
	rt_interrupt_enter();
    hs_process();
    TIMERA_ClearFlag(M4_TMRA1, TimeraFlagOverflow);
    rt_interrupt_leave();
}

static void timerA1_int_set(rt_uint32_t freq, IRQn_Type irqNbr)
{
	stc_timera_base_init_t stcTimeraInit;
    stc_irq_regi_conf_t stcIrqRegiConf;
    stc_clk_freq_t stcClk_t;
    rt_uint16_t usPeriod;
	en_timera_clk_div_t usPrescaler;

    /* Configuration peripheral clock */
    PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIMA1, Enable);

	CLK_GetClockFreq(&stcClk_t);
    if(freq < 100 && freq > 1){
		usPrescaler = TimeraPclkDiv1024;
		usPeriod =  (stcClk_t.pclk1Freq / 1024) / freq - 1;
    }else if(freq < 4000){
	 	usPrescaler = TimeraPclkDiv128; 				 
	 	usPeriod =  (stcClk_t.pclk1Freq / 128) / freq  - 1;		 
	}else{
		usPrescaler = TimeraPclkDiv1; 				 
	 	usPeriod =  stcClk_t.pclk1Freq / freq  - 1;
	}

    /* Configuration timera unit 1 structure */
    stcTimeraInit.enClkDiv = usPrescaler;
    stcTimeraInit.enCntMode = TimeraCountModeSawtoothWave;
    stcTimeraInit.enCntDir = TimeraCountDirUp;
    stcTimeraInit.enSyncStartupEn = Disable;
    stcTimeraInit.u16PeriodVal = usPeriod;		
    TIMERA_BaseInit(M4_TMRA1, &stcTimeraInit);
    TIMERA_IrqCmd(M4_TMRA1, TimeraIrqOverflow, Enable);

    /* Configure interrupt of timera unit 1 */
    stcIrqRegiConf.enIntSrc = INT_TMRA1_OVF;
    stcIrqRegiConf.enIRQn = irqNbr;
    stcIrqRegiConf.pfnCallback = &TimeraUnit1_IrqCallback;
    enIrqRegistration(&stcIrqRegiConf);
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_12);
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);

    TIMERA_Cmd(M4_TMRA1, Disable);
}

static int timerA1_init(void)
{
    timerA1_int_set(4000, Int006_IRQn);
    return 0;
}
INIT_PREV_EXPORT(timerA1_init);

