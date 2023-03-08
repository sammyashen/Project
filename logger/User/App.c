#include "include.h"

static task_t logger_task;
static uint8_t status = 0;

static void logger_task_cb(void *para, uint32_t evt)
{
	FRESULT result;
	static FATFS fs;
	static FIL file;
	static uint8_t recfilename[100] = "";
	static uint8_t led_status = 0;
	static uint8_t led_cnt = 0;
	uint8_t i = 0;
	uint8_t buff[1024] = {0};
	uint32_t size = 0, bw = 0;

	switch(led_status)
	{
		case 1:		//blue led on
			GPIO_SetBits(GPIOB, GPIO_Pin_4);
			GPIO_ResetBits(GPIOA, GPIO_Pin_15);
		break;

		case 2:		//yellow led on
			GPIO_ResetBits(GPIOB, GPIO_Pin_4);
			GPIO_SetBits(GPIOA, GPIO_Pin_15);
		break;

		case 3:		//blue led blink
			if(++led_cnt >= 50){
				led_cnt = 0;
				GPIOB->ODR ^= GPIO_Pin_4;
				GPIO_ResetBits(GPIOA, GPIO_Pin_15);
			}
		break;

		default:
			GPIO_ResetBits(GPIOB, GPIO_Pin_4);
			GPIO_ResetBits(GPIOA, GPIO_Pin_15);
		break;
	}

	switch(status)
	{
		case 0:		//挂载FS
			result = f_mount(FS_SD, &fs);
			if(result == FR_OK){
				tiny_printf("file system mount successed.\r\n");
				status = 1;
			}else{
				__set_FAULTMASK(1);
				NVIC_SystemReset();
			}
		break;

		case 1:		//创建文件
			led_status = 1;
			for(i = 1;i < 0xFF;++i){
				sprintf((char*)recfilename, "%02d.bin", i);
				result = f_open(&file,(const TCHAR *)recfilename, FA_CREATE_NEW);
				if(result == FR_OK){
					tiny_printf("file open successed.\r\n");
					break;
				}
				else if(result == FR_EXIST){
					tiny_printf("file is exist.\r\n");
				}else{
					tiny_printf("file open failed. ret = [%d]\r\n", result);
					status = 9;
					return;
				}
			}
			//Close and open again. Ensure that the file cache is created successfully
			result = f_close(&file);
			tiny_printf("openfile -> [%s]", recfilename);
			if(result != FR_OK)
			{
				tiny_printf("file close failed. ret = [%d]\r\n", result);
				status = 9;
				return;
			}else{
				status = 2;
			}
		break;

		case 2:		//接收数据写入
			size = uart_read(DEV_UART1, buff, sizeof(buff));
			if(size != 0){
				result = f_open(&file,(const TCHAR *)recfilename, FA_WRITE);
				if(result != FR_OK){
					tiny_printf("data write failed by file open failed.\r\n");
					status = 9;
					return;
				}
				f_lseek(&file, file.fsize);
				result = f_write(&file, buff, size, &bw);
				if(result == FR_OK){
					tiny_printf("data write successed.file_size = [%d]\r\n", file.fsize);
				}else{
					tiny_printf("data write failed. ret = [%d]\r\n", result);
					status = 9;
				}
				result = f_close(&file);
				if(result != FR_OK){
					tiny_printf("data write failed by file close failed.\r\n");
					status = 9;
					return;
				}
			}
			led_status = 3;
		break;

		default:
			led_status = 2;
		break;
	}
}

static void led_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOB, GPIO_Pin_4);
	GPIO_ResetBits(GPIOA, GPIO_Pin_15);
}
device_initcall(led_init);

static void app_init(void)
{
    tiny_timer_create(&logger_task, logger_task_cb, NULL);
    tiny_timer_start(&logger_task, TIMER_FOREVER, 10);
}
app_initcall(app_init);

int main(void)
{
	__disable_irq();
	SysTick_Config(168000000 / 1000u);
	do_init_call();
	__enable_irq();
	while(1)
	{
		uart_poll_dma_tx(DEV_UART1);
		uart_poll_dma_tx(DEV_UART6);
		
		tiny_task_loop();
	}
}

