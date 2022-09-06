#include "debug.h"
#include "dev_uart.h"
#include "dev_adc.h"
#include "dev_heat.h"
#include "include.h"

static struct rt_semaphore anosem;

//extern __IO uint16_t g_usFG1Freq;
//extern __IO uint16_t g_usFG2Freq;
//extern heat_t ntc1_heat_struct,ntc2_heat_struct;

void ano_debug(void)
{
	uint8_t send_data[20];
	uint8_t send_cnt = 0;
	adc_filter_t *adc_filter_struct = get_adc_filter_struct();
	
	rt_sem_init(&anosem, "anosem", 0, RT_IPC_FLAG_FIFO);

	do
	{
		rt_memset(send_data, 0, 20);
		uint16_t temp = 0;
		send_cnt = 0;
		uint8_t sum = 0;
		send_data[send_cnt++] = 0xAA;
		send_data[send_cnt++] = 0x01;
		send_data[send_cnt++] = 0xAF;
		send_data[send_cnt++] = 0xF1;
		send_data[send_cnt++] = 0x02;
		temp = adc_filter_struct->ntc1_filter_adc;
		send_data[send_cnt++] = BYTE1(temp);
		send_data[send_cnt++] = BYTE0(temp);
		
		for(uint8_t i=0;i<send_cnt;i++)
			sum += send_data[i];
		send_data[send_cnt++] = sum;
		uart_write(DEV_UART1, send_data, send_cnt);
		rt_thread_mdelay(50);

		rt_memset(send_data, 0, 20);
		send_cnt = 0;
		sum = 0;
		send_data[send_cnt++] = 0xAA;
		send_data[send_cnt++] = 0x01;
		send_data[send_cnt++] = 0xAF;
		send_data[send_cnt++] = 0xF2;
		send_data[send_cnt++] = 0x02;
		temp = adc_filter_struct->ntc2_filter_adc;
		send_data[send_cnt++] = BYTE1(temp);
		send_data[send_cnt++] = BYTE0(temp);
		for(uint8_t i=0;i<send_cnt;i++)
			sum += send_data[i];
		send_data[send_cnt++] = sum;
		uart_write(DEV_UART1, send_data, send_cnt);
		rt_thread_mdelay(50);
	}while(rt_sem_take(&anosem, 200) != RT_EOK);
}
MSH_CMD_EXPORT(ano_debug, ano debug);



