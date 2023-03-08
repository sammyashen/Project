#include "include.h"

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

static task_t boot_task;

static void jump2app(uint32_t app_addr)
{
	uint32_t i=0;
	void (*SysMemBootJump)(void);        
	__IO uint32_t BootAddr = app_addr;  

	disable_irq();

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

	enable_irq();

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

/*
 *EVT_0: send "start"
 */
static void boot_task_cb(void *para, uint32_t evt)
{
	uint8_t buf[64] = {0};
	uint16_t size = 0;
	static uint8_t jump_delay_cnt = 0;
	static uint8_t env_sector_bk[8*1024] = {0xFF};
	
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
			tiny_printf("test,,test..\r\n");
		}
	}

	if(jump_delay_cnt > 0){
		jump_delay_cnt--;
		if(jump_delay_cnt == 1){
			jump2app(BOOT_FW_START_ADDR);
		}
	}

	//喂狗，初始化已在bios中完成，app只需喂狗，无需重新配置SWDT
	SWDT_RefreshCounter();
	uart_poll_dma_tx(DEV_UART1);
}

void app_init(void)
{
    tiny_timer_create(&boot_task, boot_task_cb, NULL);
    tiny_timer_start(&boot_task, TIMER_FOREVER, 10);

	static uint8_t env_sector_bk[4*10] = {0xFF};
	flash_read(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
	if(env_sector_bk[4] == 0xFF){
		env_sector_bk[4] = fw_version_init;
		flash_erase(ENV_UPDATE_FLAG_ADDR, sizeof(env_sector_bk));
		flash_write(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
	}
}
app_initcall(app_init);

int32_t main(void)
{
	SCB->VTOR = ((uint32_t)APP_FW_START_ADDR & SCB_VTOR_TBLOFF_Msk);
	__disable_irq();
	do_init_call();
	__enable_irq();
	tiny_printf("testSSS123456789ABCDEFGHIJKLMN..\r\n");

	uint8_t env_fw_version_bk = 0;
	flash_read(ENV_FW_VERSION_ADDR, (uint8_t *)&env_fw_version_bk, 1);
	tiny_printf("fw_version_bk = 0x%02X\r\n", env_fw_version_bk);
	uint8_t env_update_flag_e = 0;
	flash_read(ENV_UPDATE_FLAG_ADDR, (uint8_t *)&env_update_flag_e, 1);
	tiny_printf("env_update_flag_e = 0x%02X\r\n", env_update_flag_e);
	while(1)
	{
		tiny_task_loop();
	}
}

