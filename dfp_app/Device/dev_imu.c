#include "dev_imu.h"
#include "rtthread.h"
#include "tiny_dev.h"

#include "perf_counter.h"
#include "fifo.h"

static imu_pack_data spi2_pack_0_data[150];
static imu_pack_data spi2_pack_1_data[150];
static uint64_t master_tmst = 0;
static uint64_t tmst_anchor = 0;
static uint32_t delta_tmst = 0;
static bool is_got_int = false;
static bool is_positive = true;
static _fifo_t spi2_tx_fifo_0;
static _fifo_t spi2_tx_fifo_1;
static struct rt_timer dma_timer;

volatile static uint16_t spi2_pkt_cnt = 0;
volatile static uint8_t spi2_stat = 0;
volatile static uint8_t spi2_fifo_index = 0;

static uint8_t spi2_rx_dma[2] = {0};
static uint8_t tmst_dma[8] = {0};

static void get_tmst(void)
{
	uint8_t tmst[8] = {0};

	tmst[0] = (uint8_t)(master_tmst>>56);
	tmst[1] = (uint8_t)(master_tmst>>48);
	tmst[2] = (uint8_t)(master_tmst>>40);
	tmst[3] = (uint8_t)(master_tmst>>32);
	tmst[4] = (uint8_t)(master_tmst>>24);
	tmst[5] = (uint8_t)(master_tmst>>16);
	tmst[6] = (uint8_t)(master_tmst>>8);
	tmst[7] = (uint8_t)(master_tmst);

	rt_kprintf("tmst = [0x");
	for(uint8_t i=0;i<8;i++)
	{
		rt_kprintf("%02X", tmst[i]);
	}
	rt_kprintf("]\r\n");
}
MSH_CMD_EXPORT(get_tmst, spi test);

static void wr_fifo(void)
{
	imu_pack_data spi2_pack_data = {0};

	spi2_pack_data.tmst = 0x1122334455667788;

	if(spi2_fifo_index == 0x00){
		spi2_pack_data.acc_x = 0x11AA;
		spi2_pack_data.gyro_x = 0x22AA;
		fifo_write(&spi2_tx_fifo_0, (uint8_t *)&spi2_pack_data.acc_x, sizeof(imu_pack_data));
	}
	else{
		spi2_pack_data.acc_x = 0x11BB;
		spi2_pack_data.gyro_x = 0x22BB;
		fifo_write(&spi2_tx_fifo_1, (uint8_t *)&spi2_pack_data.acc_x, sizeof(imu_pack_data));
	}
}
MSH_CMD_EXPORT(wr_fifo, spi test);

void EXTI15_10_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_LINE11) != RESET)
    {
    	tmst_anchor = (get_system_ticks())>>6;//us
    	is_got_int = true;
        EXTI_ClrITPendBit(EXTI_LINE11);
    }
}

void DMA_Channel7_IRQHandler(void)
{
	rt_interrupt_enter();
	if(DMA_GetIntStatus(DMA_INT_TXC7, DMA)){
		switch(spi2_stat)
		{
			case 0x00:
				rt_timer_start(&dma_timer);
				if((spi2_rx_dma[0] == 0x5B && spi2_rx_dma[1] == 0xFF) || 
						(spi2_rx_dma[1] == 0x5B)){			//获取时间戳
					spi2_stat = 0x01;
					spi2_dma_rx_config(tmst_dma, 8);
				}else if((spi2_rx_dma[0] == 0x5C && spi2_rx_dma[1] == 0xFF) || 
						(spi2_rx_dma[1] == 0x5C)){			//发送fifo数据
					if(spi2_fifo_index == 0x00)
						spi2_pkt_cnt = fifo_get_occupy_size(&spi2_tx_fifo_0)/sizeof(imu_pack_data);
					else
						spi2_pkt_cnt = fifo_get_occupy_size(&spi2_tx_fifo_1)/sizeof(imu_pack_data);
					spi2_stat = 0x02;
					spi2_dma_tx_config((uint8_t *)&spi2_pkt_cnt, 2);
				}
			break;

			case 0x01:
				spi2_stat = 0x00;
				spi2_dma_rx_config(spi2_rx_dma, 2);
				if(spi2_rx_dma[0] == 0x5B && spi2_rx_dma[1] == 0xFF){
					master_tmst = ((uint64_t)tmst_dma[0]<<56)|((uint64_t)tmst_dma[1]<<48)|((uint64_t)tmst_dma[2]<<40)|
									((uint64_t)tmst_dma[3]<<32)|((uint64_t)tmst_dma[4]<<24)|((uint64_t)tmst_dma[5]<<16)|
									  ((uint64_t)tmst_dma[6]<<8)|((uint64_t)tmst_dma[7]);
				}else{
					tmst_dma[0] = spi2_rx_dma[0];
					master_tmst = ((uint64_t)tmst_dma[1]<<56)|((uint64_t)tmst_dma[2]<<48)|((uint64_t)tmst_dma[3]<<40)|
									((uint64_t)tmst_dma[4]<<32)|((uint64_t)tmst_dma[5]<<24)|((uint64_t)tmst_dma[6]<<16)|
									  ((uint64_t)tmst_dma[7]<<8)|((uint64_t)tmst_dma[0]);
				}
				//计算系统时间差
				if(!is_got_int)		break;
				is_got_int = false;
				if(master_tmst > tmst_anchor){
					is_positive = true;
					delta_tmst = master_tmst - tmst_anchor;
				}else{
					is_positive = false;
					delta_tmst = tmst_anchor - master_tmst;
				}
				rt_timer_stop(&dma_timer);
			break;
		}
		DMA->INTCLR = DMA_INT_TXC7;
	}
	rt_interrupt_leave();
}

void DMA_Channel8_IRQHandler(void)
{
	rt_interrupt_enter();
	if(DMA_GetIntStatus(DMA_INT_TXC8, DMA)){
		switch(spi2_stat)
		{
			case 0x02:
				if(spi2_pkt_cnt > 0){
					spi2_stat = 0x03;
					if(spi2_fifo_index == 0x00){
						//修正时间戳
						for(uint8_t i=0;i<spi2_pkt_cnt;i++){
							if(is_positive)		spi2_pack_0_data[i].tmst += delta_tmst;
							else				spi2_pack_0_data[i].tmst -= delta_tmst;
						}
						spi2_dma_tx_config((uint8_t *)&spi2_pack_0_data[0].acc_x, 20*spi2_pkt_cnt);
					}else{
						for(uint8_t i=0;i<spi2_pkt_cnt;i++){
							if(is_positive)		spi2_pack_1_data[i].tmst += delta_tmst;
							else				spi2_pack_1_data[i].tmst -= delta_tmst;
						}
						spi2_dma_tx_config((uint8_t *)&spi2_pack_1_data[0].acc_x, 20*spi2_pkt_cnt);
					}
					spi2_fifo_index = (~spi2_fifo_index);//切fifo索引
				}else{
					spi2_stat = 0x00;
					spi2_dma_rx_config(spi2_rx_dma, 2);
					if(spi2_fifo_index == 0x00){
						spi2_tx_fifo_0.pread = spi2_tx_fifo_0.pwrite = spi2_tx_fifo_0.buf;//重置指针
						spi2_tx_fifo_0.occupy_size = 0;
					}else{
						spi2_tx_fifo_1.pread = spi2_tx_fifo_1.pwrite = spi2_tx_fifo_1.buf;
						spi2_tx_fifo_1.occupy_size = 0;
					}
				}
			break;

			case 0x03:
				spi2_stat = 0x00;
				spi2_dma_rx_config(spi2_rx_dma, 2);
				if(spi2_fifo_index == 0x00){
					spi2_tx_fifo_0.pread = spi2_tx_fifo_0.pwrite = spi2_tx_fifo_0.buf;//重置指针
					spi2_tx_fifo_0.occupy_size = 0;
				}else{
					spi2_tx_fifo_1.pread = spi2_tx_fifo_1.pwrite = spi2_tx_fifo_1.buf;
					spi2_tx_fifo_1.occupy_size = 0;
				}
				rt_timer_stop(&dma_timer);
			break;
		}
		DMA->INTCLR = DMA_INT_TXC8;
	}
	rt_interrupt_leave();
}

static void dma_timer_timeout(void *parameter)
{
	DMA_RequestRemap(DMA_REMAP_SPI2_TX, DMA, DMA_CH8, DISABLE);
	DMA_EnableChannel(DMA_CH8, DISABLE);
	DMA_ConfigInt(DMA_CH8, DMA_INT_TXC, DISABLE);
	DMA_ClearFlag(DMA_FLAG_TC8, DMA);
	spi2_dma_rx_config(spi2_rx_dma, sizeof(spi2_rx_dma));
	spi2_stat = 0;
	if(spi2_stat == 0x03)	spi2_fifo_index = (~spi2_fifo_index);
}

void tim6_init(void)
{
	TIM_TimeBaseInitType TIM_TimeBaseStructure;
	NVIC_InitType NVIC_InitStructure;
	RCC_ClocksType 			RCC_ClockFreq;

	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM6, ENABLE);

	RCC_GetClocksFreqValue(&RCC_ClockFreq);
	TIM_TimeBaseStructure.Period    = RCC_ClockFreq.SysclkFreq/2/4000 - 1;
    TIM_TimeBaseStructure.Prescaler = 0;
    TIM_TimeBaseStructure.ClkDiv    = 0;
    TIM_TimeBaseStructure.CntMode   = TIM_CNT_MODE_UP;
    TIM_TimeBaseStructure.RepetCnt = 0;
    TIM_InitTimeBase(TIM6, &TIM_TimeBaseStructure);
	TIM_ConfigArPreload(TIM6, ENABLE);
    TIM_ConfigInt(TIM6, TIM_INT_UPDATE, ENABLE);
    TIM_Enable(TIM6, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel                   = TIM6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 4;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

//4KHz
void TIM6_IRQHandler(void)
{
    if (TIM_GetIntStatus(TIM6, TIM_INT_UPDATE) != RESET)
    {
        TIM_ClrIntPendingBit(TIM6, TIM_INT_UPDATE);
		wr_fifo();
    }
}

static int dma_timer_init(void)
{
	tim6_init();
	rt_timer_init(&dma_timer, "dma_timer", dma_timer_timeout, RT_NULL, 
					20, RT_TIMER_CTRL_SET_ONESHOT|RT_TIMER_FLAG_SOFT_TIMER);

	return 0;
}
INIT_APP_EXPORT(dma_timer_init);

static int icm42631_ap_read(void *buf, int size)
{
	icm42631_ops_data_t data = (icm42631_ops_data_t)buf;
	
	i2c_write_bytes(IMU_DEV_ADDR, REG_BANK_SEL, 1, &data->bank, 6000ul);
	i2c_read_bytes(IMU_DEV_ADDR, data->reg, size, &data->buf[0], 6000ul);
	return size;
}

static int icm42631_ap_write(const void *buf, int size)
{
	icm42631_ops_data_t data = (icm42631_ops_data_t)buf;

	i2c_write_bytes(IMU_DEV_ADDR, REG_BANK_SEL, 1, &data->bank, 6000ul);
	i2c_write_bytes(IMU_DEV_ADDR, data->reg, size, &data->buf[0], 6000ul);
	return size;
}

uint8_t imu_spi_3wire_trans_byte(uint8_t reg_addr, uint8_t *byte)
{
	uint8_t data = 0;
	GPIO_InitType GPIO_InitStructure;

	GPIO_InitStructure.Pin = GPIO_PIN_5;
	GPIO_InitStructure.GPIO_Alternate = GPIO_AF0_SPI1;
	SPI1_CS_ENABLE;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
	spi1_4wire_trans_byte(reg_addr);
    if((reg_addr | REG_RD) == REG_RD){
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Input;
		GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
    }
    data = spi1_4wire_trans_byte(0x00);
    SPI1_CS_DISABLE;

    return data;
}

static int icm42631_aux1_read(void *buf, int size)
{
	icm42631_ops_data_t data = (icm42631_ops_data_t)buf;
	
	imu_spi_3wire_trans_byte(REG_BANK_SEL | REG_WR, &data->bank);
	data->buf[0] = imu_spi_3wire_trans_byte(data->reg | REG_RD, NULL);

	if(size != 0x01)	return 0;
	else				return size;
}

static int icm42631_aux1_write(const void *buf, int size)
{
	icm42631_ops_data_t data = (icm42631_ops_data_t)buf;

	if(size != 0x01)	return 0;
	else{
		imu_spi_3wire_trans_byte(REG_BANK_SEL | REG_WR, &data->bank);
		imu_spi_3wire_trans_byte(data->reg | REG_WR, &data->buf[0]);
		return size;
	}
}

static tiny_device_ops icm42631_ops[2] = {
	{
		.read = icm42631_ap_read,
		.write = icm42631_ap_write,
	},
	{
		.read = icm42631_aux1_read,
		.write = icm42631_aux1_write,
	},
};

static tiny_device imu_dev = {
	.name = "icm42631",
	.dops = &icm42631_ops[0],
};

static int imu_tiny_dev_register(void)
{
	i2c_hw_init();
	spi_hw_init();
	spi2_dma_rx_config(spi2_rx_dma, sizeof(spi2_rx_dma));
	fifo_register(&spi2_tx_fifo_0, (uint8_t *)&spi2_pack_0_data[0].acc_x, sizeof(spi2_pack_0_data), NULL, NULL);
	fifo_register(&spi2_tx_fifo_1, (uint8_t *)&spi2_pack_1_data[0].acc_x, sizeof(spi2_pack_1_data), NULL, NULL);
	tiny_device_register(&imu_dev);
	return 0;
}
INIT_DEVICE_EXPORT(imu_tiny_dev_register);

