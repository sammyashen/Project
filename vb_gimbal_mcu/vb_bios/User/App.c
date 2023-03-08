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
/*****************************************************************************************/

static task_t bios_fsm_task;
static struct state_machine bios_fsm;
static struct state bios_idle, bios_rd_update_flag, bios_check_crc, bios_copy_code, bios_jump, bios_error;

static bool bios_evt_compare(void *key, struct event *event)
{
	if(event->type != BIOS_EVT)   		return false;
	if(strcmp(key, event->data) == 0)	return true;
	else								return false;
}

static struct state bios_idle = {   
	.transitions = (struct transition[]){
		{BIOS_EVT, (void *)"start", bios_evt_compare, NULL, &bios_rd_update_flag},   
	},
	.transition_nums = 1,
};

static void bios_rd_update_flag_entry(void *state_data, struct event *event)
{
	uint8_t env_update_flag_e = 0;
	
	flash_read(ENV_UPDATE_FLAG_ADDR, (uint8_t *)&env_update_flag_e, 1);
	if(env_update_flag_e == APP_UPDATE || env_update_flag_e == BOOT_UPDATE){	
		tiny_set_event(&bios_fsm_task, EVT_2);
	}else{
		tiny_set_event(&bios_fsm_task, EVT_3);
	}
}
static struct state bios_rd_update_flag = {
	.transitions = (struct transition[]){  
		{BIOS_EVT, (void *)"change", bios_evt_compare, NULL, &bios_check_crc},
		{BIOS_EVT, (void *)"jump", bios_evt_compare, NULL, &bios_jump},
	},
	.transition_nums = 2,
	.action_entry = &bios_rd_update_flag_entry,
};

static void bios_check_crc_entry(void *state_data, struct event *event)
{
	uint8_t env_update_flag_e = 0;
	uint32_t env_fw_crc = 0;
	uint32_t env_fw_size = 0;
	uint32_t check_crc32 = 0;
	uint8_t *ptr = NULL;
	uint8_t env_sector_bk[4*10] = {0xFF};

	flash_read(ENV_UPDATE_FLAG_ADDR, (uint8_t *)&env_update_flag_e, 1);
	if(env_update_flag_e == APP_UPDATE){	
		uint8_t tmp[4] = {0};
		flash_read((APP_FW_DOWNLOAD_START_ADDR+APP_FW_CRC_OFFSET), tmp, 4);
		env_fw_crc = (uint32_t)((tmp[0]<<24)|(tmp[1]<<16)|(tmp[2]<<8)|tmp[3]);	//小端存储，重新组数据
		flash_read((APP_FW_DOWNLOAD_START_ADDR+APP_FW_SIZE_OFFSET), tmp, 4);
		env_fw_size = (uint32_t)((tmp[0]<<24)|(tmp[1]<<16)|(tmp[2]<<8)|tmp[3]);
		//限制size取值
		if(env_fw_size > APP_FW_SIZE)	env_fw_size = APP_FW_SIZE - 4;
		ptr = (uint8_t *)(APP_FW_DOWNLOAD_START_ADDR+APP_FW_CODE_OFFSET);
		crc32(ptr, env_fw_size, &check_crc32);
	}else{
		uint8_t tmp[4] = {0};
		flash_read((BOOT_FW_DOWNLOAD_START_ADDR+BOOT_FW_CRC_OFFSET), tmp, 4);
		env_fw_crc = (uint32_t)((tmp[0]<<24)|(tmp[1]<<16)|(tmp[2]<<8)|tmp[3]);
		flash_read((BOOT_FW_DOWNLOAD_START_ADDR+BOOT_FW_SIZE_OFFSET), tmp, 4);
		env_fw_size = (uint32_t)((tmp[0]<<24)|(tmp[1]<<16)|(tmp[2]<<8)|tmp[3]);
		//限制size取值
		if(env_fw_size > BOOT_FW_SIZE)	env_fw_size = BOOT_FW_SIZE - 4;
		ptr = (uint8_t *)(BOOT_FW_DOWNLOAD_START_ADDR+BOOT_FW_CODE_OFFSET);
		crc32(ptr, env_fw_size, &check_crc32);
	}

	if(env_fw_crc == check_crc32){
		tiny_set_event(&bios_fsm_task, EVT_2);
	}else{
		//先备份env扇区
		flash_read(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
		//修改对应变量
		env_sector_bk[0] = UPDATE_ERR;
		//擦除env扇区
		flash_erase(ENV_UPDATE_FLAG_ADDR, sizeof(env_sector_bk));
		//回写flash
		flash_write(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
		tiny_set_event(&bios_fsm_task, EVT_3);
	}
}
static struct state bios_check_crc = {	   
	.transitions = (struct transition[]){
		{BIOS_EVT, (void *)"change", bios_evt_compare, NULL, &bios_copy_code},
		{BIOS_EVT, (void *)"jump", bios_evt_compare, NULL, &bios_jump},
	},
	.transition_nums = 2,
	.action_entry = &bios_check_crc_entry,
};

static void bios_copy_code_entry(void *state_data, struct event *event)
{
	uint8_t env_update_flag_e = 0;
	uint32_t env_fw_size = 0;
	uint8_t *ptr = NULL;
	uint8_t env_sector_bk[4*10] = {0xFF};

	flash_read(ENV_UPDATE_FLAG_ADDR, (uint8_t *)&env_update_flag_e, 1);
	if(env_update_flag_e == APP_UPDATE){	
		flash_erase(APP_FW_START_ADDR, APP_FW_SIZE);
		uint8_t tmp[4] = {0};
		flash_read((APP_FW_DOWNLOAD_START_ADDR+APP_FW_SIZE_OFFSET), tmp, 4);
		env_fw_size = (uint32_t)((tmp[0]<<24)|(tmp[1]<<16)|(tmp[2]<<8)|tmp[3]);
		//限制size取值
		if(env_fw_size > APP_FW_SIZE)	env_fw_size = APP_FW_SIZE - 4;
		flash_write(APP_FW_START_ADDR, (uint8_t *)(APP_FW_DOWNLOAD_START_ADDR+APP_FW_CODE_OFFSET), env_fw_size);
		ptr = (uint8_t *)(APP_FW_DOWNLOAD_START_ADDR+APP_FW_BASE_INFO_OFFSET+1);
	}else if(env_update_flag_e == BOOT_UPDATE){	
		flash_erase(BOOT_FW_START_ADDR, BOOT_FW_SIZE);
		uint8_t tmp[4] = {0};
		flash_read((BOOT_FW_DOWNLOAD_START_ADDR+BOOT_FW_SIZE_OFFSET), tmp, 4);
		env_fw_size = (uint32_t)((tmp[0]<<24)|(tmp[1]<<16)|(tmp[2]<<8)|tmp[3]);
		//限制size取值
		if(env_fw_size > BOOT_FW_SIZE)	env_fw_size = BOOT_FW_SIZE - 4;
		flash_write(BOOT_FW_START_ADDR, (uint8_t *)(BOOT_FW_DOWNLOAD_START_ADDR+BOOT_FW_CODE_OFFSET), env_fw_size);
		ptr = (uint8_t *)(BOOT_FW_DOWNLOAD_START_ADDR+BOOT_FW_BASE_INFO_OFFSET+1);
	}
	env_update_flag_e = UPDATE_OK;
	flash_read(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
	env_sector_bk[0] = env_update_flag_e;
	memcpy(&env_sector_bk[4], ptr, 4);
	flash_erase(ENV_UPDATE_FLAG_ADDR, sizeof(env_sector_bk));
	flash_write(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
	tiny_set_event(&bios_fsm_task, EVT_2);
}
static struct state bios_copy_code = {	   
	.transitions = (struct transition[]){
		{BIOS_EVT, (void *)"change", bios_evt_compare, NULL, &bios_jump},
	},
	.transition_nums = 1,
	.action_entry = &bios_copy_code_entry,
};

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

    /* 清除本项目中所有注册的中断回调 */

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

static void bios_jump_entry(void *state_data, struct event *event)
{
	jump2app(APP_FW_START_ADDR);
}
static struct state bios_jump = {	   
	.action_entry = &bios_jump_entry,
};

/*
 *EVT_0：send "start"
 *EVT_1: send "stop"
 *EVT_2: send "change"
 *EVT_3: send "jump"
 */
static void bios_fsm_task_cb(void *para, uint32_t evt)
{
	//ble fsm
	if(evt & EVT_0){
		tiny_clr_event(&bios_fsm_task, EVT_0);
		statem_handle_event(&bios_fsm, &(struct event){BIOS_EVT, (void *)"start"});
	}
	if(evt & EVT_1){
		tiny_clr_event(&bios_fsm_task, EVT_1);
		statem_handle_event(&bios_fsm, &(struct event){BIOS_EVT, (void *)"stop"});
	}
	if(evt & EVT_2){
		tiny_clr_event(&bios_fsm_task, EVT_2);
		statem_handle_event(&bios_fsm, &(struct event){BIOS_EVT, (void *)"change"});
	}
	if(evt & EVT_3){
		tiny_clr_event(&bios_fsm_task, EVT_3);
		statem_handle_event(&bios_fsm, &(struct event){BIOS_EVT, (void *)"jump"});
	}

	//喂狗
	SWDT_RefreshCounter();
}

void app_init(void)
{
    tiny_timer_create(&bios_fsm_task, bios_fsm_task_cb, NULL);
    tiny_timer_start(&bios_fsm_task, TIMER_FOREVER, 10);

	statem_init(&bios_fsm, &bios_idle, &bios_error);
	//直接启动bios
	tiny_set_event(&bios_fsm_task, EVT_0);
}
app_initcall(app_init);

int32_t main(void)
{
	__disable_irq();
	do_init_call();
	__enable_irq();
	while(1)
	{
		tiny_task_loop();
	}
}

