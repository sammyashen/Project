#include "include.h"

#define LOG_TAG				"vb_gimbal"
#define LOG_LVL				LOG_LVL_DBG
#define ULOG_OUTPUT_LVL		LOG_LVL_DBG
#include <rtdbg.h>

/****************************** user need to modify **************************************/
#define ENV_UPDATE_FLAG_ADDR			(0x7E000)
#define ENV_FW_VERSION_ADDR				(0x7E004)

#define APP_FW_DOWNLOAD_START_ADDR		(0x46000)
#define APP_FW_DOWNLOAD_SIZE			(0x32000)
#define APP_FW_BASE_INFO_OFFSET			(0x00)
#define APP_FW_SIZE_OFFSET				(0x04)
#define APP_FW_CRC_OFFSET				(0x08)
#define APP_FW_CODE_OFFSET				(0x10)

#define BOOT_FW_DOWNLOAD_START_ADDR		(0x3E000)
#define BOOT_FW_DOWNLOAD_SIZE			(0x8000)
#define BOOT_FW_BASE_INFO_OFFSET		(0x00)
#define BOOT_FW_SIZE_OFFSET				(0x04)
#define BOOT_FW_CRC_OFFSET				(0x08)
#define BOOT_FW_CODE_OFFSET				(0x10)

#define APP_FW_START_ADDR				(0xC000)
#define APP_FW_SIZE						(0x32000)

#define BOOT_FW_START_ADDR				(0x4000)
#define BOOT_FW_SIZE					(0x8000)

#define APP_UPDATE		(0xAA)
#define BOOT_UPDATE		(0xAB)
#define UPDATE_ERR		(0xCC)
#define UPDATE_OK		(0xFF)

#define FW_VERSION		(0x64)
/*****************************************************************************************/
const uint8_t fw_version_init = FW_VERSION;

static void jump2app(uint32_t app_addr)
{
	uint32_t i=0;
	void (*SysMemBootJump)(void);        
	__IO uint32_t BootAddr = app_addr;  

	__disable_irq();

	/* 关闭滴答定时器，复位到默认值 */
	SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

	/* 清楚本项目中所有注册的中断回调 */
	enIrqResign(Int000_IRQn);
	enIrqResign(Int001_IRQn);
	enIrqResign(Int002_IRQn);
	enIrqResign(Int003_IRQn);

	/* 关闭所有中断，清除所有中断挂起标志 */
	for (i = 0; i < 8; i++)
	{
		NVIC->ICER[i] = 0xFFFFFFFF;
		NVIC->ICPR[i] = 0xFFFFFFFF;
	}	

	__enable_irq();

	/* 跳转到系统BootLoader，首地址是MSP，地址+4是复位中断服务程序地址 */
	SysMemBootJump = (void (*)(void)) (*((uint32_t *) (BootAddr + 4)));
	/* 设置主堆栈指针 */
	__set_MSP(*(uint32_t *)BootAddr);
	/* 在RTOS工程，这条语句很重要，设置为特权级模式，使用MSP指针 */
	__set_CONTROL(0);

	SysMemBootJump(); 

	/* 跳转成功的话，不会执行到这里 */
	while (1)
	{
		//user code
	}
}

void protocol_notify(uint8_t cmd, uint8_t frame, uint8_t dat_len, uint8_t *data)
{
	uint8_t buf[256] = {0};
	uint16_t crc16 = 0;

	buf[0] = 0xA5;
	buf[1] = cmd;
	buf[2] = frame;
	buf[3] = dat_len;
	memcpy(&buf[4], data, dat_len);
	crc16_calculate(buf, (dat_len + 4), (~cmd), &crc16);
	buf[4+dat_len] = (uint8_t)(crc16 >> 8);
	buf[5+dat_len] = (uint8_t)crc16;

	uart_write(DEV_UART1, buf, (dat_len + 6));
}

int app_init(void)
{
	static uint8_t env_sector_bk[4*10] = {0xFF};
	flash_read(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
	if(env_sector_bk[4] == 0xFF){
		env_sector_bk[4] = fw_version_init;
		flash_erase(ENV_UPDATE_FLAG_ADDR, sizeof(env_sector_bk));
		flash_write(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
	}
	
	return 0;
}
INIT_APP_EXPORT(app_init);

static uint8_t jump_delay_cnt = 0;
static void task(void)
{
	uint8_t buf[64] = {0};
	uint16_t size = 0;
	static uint8_t env_sector_bk[8*1024] = {0xFF};
	
	while(1){
		size = uart_read(DEV_UART1, buf, sizeof(buf));
		if(size != 0){
			if((buf[0] == 0xA5) && (buf[1] == 0xF0) && (buf[2] == 0x5A)){
				jump_delay_cnt = 51;
				protocol_notify(0xF0, 0x80, 0x01, &(uint8_t){0x00});

				flash_read(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
				env_sector_bk[0] = APP_UPDATE;
				flash_erase(ENV_UPDATE_FLAG_ADDR, sizeof(env_sector_bk));
				flash_write(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
			}else if((buf[0] == 0xA5) && (buf[1] == 0xF0) && (buf[2] == 0x5B)){
				jump_delay_cnt = 51;
				protocol_notify(0xF0, 0x80, 0x01, &(uint8_t){0x00});

				flash_read(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
				env_sector_bk[0] = BOOT_UPDATE;
				flash_erase(ENV_UPDATE_FLAG_ADDR, sizeof(env_sector_bk));
				flash_write(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
			}else{
				rt_kprintf("test..test..\r\n");
			}
		}
	}
}

static void ota_test(void)
{
	task();
}
MSH_CMD_EXPORT(ota_test, ota test func);

#include <stdlib.h>
static void print_para(int argc, char **argv)
{
	uint8_t _int8_num = 0;
	float32_t _float32_num = 0;
	
	rt_kprintf("para num = %d\r\n", argc);
	_int8_num = atoi(argv[1]);
	rt_kprintf("int para num = %d\r\n", _int8_num);
	_float32_num = atof(argv[2]);
	rt_kprintf("float para num = %0.2f\r\n", _float32_num);
}
MSH_CMD_EXPORT(print_para, print test func);

static void log_test(void)
{
	LOG_D("log debug");
	LOG_I("log info");
	LOG_W("log warn");
	LOG_E("log error");
	LOG_I("float data = %0.04f", 3.126);
//	rt_uint8_t hex_buf[20] = "hello world.";
	rt_uint8_t *ptr = NULL;
	ptr = (rt_uint8_t *)APP_FW_START_ADDR;
	LOG_HEX("[code hex]", 8, ptr, 100);
//	LOG_HEX("[test_hex]", 8, hex_buf, rt_strlen((char *)hex_buf));
}
MSH_CMD_EXPORT(log_test, log test func);
																				
static void TESTthread_entry(void *para)
{
	while(1)
	{
		_code_perf_start(0)
		{
			rt_thread_mdelay(231);
		}
		_code_perf_end(0);

		srand((rt_uint32_t)get_system_ticks());
		rt_uint32_t n = (rand() & 0x7FFFF);
		_code_perf_start(1)
		{
			for(rt_uint32_t i = 0;i < n;i++);
		}
		_code_perf_end(1);
	}
}

int32_t main(void)
{
	static rt_uint16_t kkk = 0;
	
	static uint8_t env_sector_bk[4*10] = {0xFF};
	flash_read(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
	rt_kprintf("App version:0x%02X,%0.4f\r\n", env_sector_bk[4], 1.2345);

	rt_thread_t rt_TESTthread_id = RT_NULL;
	
	rt_TESTthread_id = rt_thread_create("test",
								TESTthread_entry,
								RT_NULL,
								1024,		//stack size
								16,			//prioity
								10);		//timeslice
	if(rt_TESTthread_id != RT_NULL)
		rt_thread_startup(rt_TESTthread_id);
	while(1)
	{
		if(jump_delay_cnt > 0){
			jump_delay_cnt--;
			if(jump_delay_cnt == 1){
				jump2app(BOOT_FW_START_ADDR);
			}
		}

		_code_perf_start(2)
		{
			for(kkk = 0;kkk < 60000;kkk++);
		}
		_code_perf_end(2);

		//喂狗，初始化已在bios中完成，app只需喂狗，无需重新配置SWDT
		SWDT_RefreshCounter();
		rt_thread_mdelay(10);
	}
}

