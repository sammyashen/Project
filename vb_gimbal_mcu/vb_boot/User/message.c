#include "message.h"
#include "include.h"

static void timer02B_irq_cb(void)
{
	uint16_t size = 0;
	frame_single_t _frame_single_data;
	static uint8_t buf[264] = {0};
	static uint8_t frame_pro_status = GET_START_BIT;
	static uint16_t dat_len = 0;
	static uint16_t dat_len_cnt = 0;
	static uint8_t crc_cnt = 0;
	static __IO uint16_t time_out_cnt = 0;

	if(frame_pro_status != GET_START_BIT){		//3S超时
		time_out_cnt++;
		if(time_out_cnt >= 30000){
			time_out_cnt = 0;
			MEM_ZERO_STRUCT(buf);
			frame_pro_status = GET_START_BIT;
		}
	}else{
		time_out_cnt = 0;
	}

	switch(frame_pro_status)
	{
		case GET_START_BIT:		
			size = uart_read(DEV_UART1, &buf[0], 1);
			if(size != 0){
				if(buf[0] == 0xA5){
					frame_pro_status = GET_CMD_BIT;
				}
			}
		break;

		case GET_CMD_BIT:		
			size = uart_read(DEV_UART1, &buf[1], 1);
			if(size != 0){
				frame_pro_status = GET_FRAME_BIT;
			}
		break;

		case GET_FRAME_BIT:
			size = uart_read(DEV_UART1, &buf[2], 1);
			if(size != 0){
				frame_pro_status = GET_LENGTH_BIT;
			}
		break;

		case GET_LENGTH_BIT:
			size = uart_read(DEV_UART1, &buf[3], 1);
			if(size != 0){
				if(buf[3] < 0xFE){
					dat_len = buf[3];
					dat_len_cnt = 0;
					frame_pro_status = GET_DATA;
				}else if(buf[3] == 0xFE){
					dat_len = 256;
					dat_len_cnt = 0;
					frame_pro_status = GET_DATA;
				}else{
					MEM_ZERO_STRUCT(buf);
					frame_pro_status = GET_START_BIT;
				}
			}
		break;

		case GET_DATA:
			size = uart_read(DEV_UART1, &buf[4+dat_len_cnt], (dat_len-dat_len_cnt));
			if(size != 0){
				dat_len_cnt += size;
				if(dat_len_cnt >= dat_len){
					crc_cnt = 0;
					frame_pro_status = GET_CRC_BIT;
				}
			}
		break;

		case GET_CRC_BIT:
			size = uart_read(DEV_UART1, &buf[4+dat_len+crc_cnt], (2-crc_cnt));
			if(size != 0){
				crc_cnt += size;
				if(crc_cnt >= 2){
					frame_pro_status = GET_FRAME_FINISH;
				}
			}
		break;

		case GET_FRAME_FINISH:
			//单帧入队
			memcpy(&_frame_single_data.start, &buf[0], 4);
			memcpy(&_frame_single_data.data[0], &buf[4], buf[3]);
			_frame_single_data.crc = (uint16_t)(buf[4+dat_len]<<8)|buf[5+dat_len];
			fifo_write(&frame_recv_struct.rx_fifo, (uint8_t *)&_frame_single_data, sizeof(frame_single_t));
			
			MEM_ZERO_STRUCT(buf);
			frame_pro_status = GET_START_BIT;
		break;

		default:
		break;
	}
}

static void timer02B_init(void)
{
    stc_tim0_base_init_t stcTimerCfg;
    stc_irq_regi_conf_t stcIrqRegiConf;
    stc_clk_freq_t stcClk_t;

    MEM_ZERO_STRUCT(stcTimerCfg);
    MEM_ZERO_STRUCT(stcIrqRegiConf);

	/* 获取pclk1时钟频率 */
	CLK_GetClockFreq(&stcClk_t);
	/* 使能内部低速时钟LRC(32.768K) */
	CLK_LrcCmd(Enable);
    /* 开启定时器0时钟 */
	PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM02, Enable);
	/*config register for channel B */
    stcTimerCfg.Tim0_CounterMode = Tim0_Sync;
    stcTimerCfg.Tim0_SyncClockSource = Tim0_Pclk1;
    stcTimerCfg.Tim0_ClockDivision = Tim0_ClkDiv8;
    stcTimerCfg.Tim0_CmpValue = (uint16_t)(stcClk_t.pclk1Freq/8/10000 - 1);	//0.1ms
    TIMER0_BaseInit(M4_TMR02, Tim0_ChannelB, &stcTimerCfg);
    /* Enable channel B interrupt */
    TIMER0_IntCmd(M4_TMR02, Tim0_ChannelB, Enable);
    /* Register TMR_INI_GCMB Int to Vect.No.025 */
    stcIrqRegiConf.enIRQn = Int025_IRQn;
    /* Select I2C Error or Event interrupt function */
    stcIrqRegiConf.enIntSrc = INT_TMR02_GCMB;
    /* Callback function */
    stcIrqRegiConf.pfnCallback = &timer02B_irq_cb;
    /* Registration IRQ */
    enIrqRegistration(&stcIrqRegiConf);
    /* Clear Pending */
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    /* Set priority */
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_15);
    /* Enable NVIC */
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
    /*start timer0*/
    TIMER0_Cmd(M4_TMR02, Tim0_ChannelB, Enable);
}

static void msg_device_register(void)
{
	timer02B_init();
	fifo_register(&frame_recv_struct.rx_fifo, (uint8_t *)&frame_recv_struct.frame_single_buf[0], 
					sizeof(frame_recv_struct.frame_single_buf), NULL, NULL);
}
device_initcall(msg_device_register);


