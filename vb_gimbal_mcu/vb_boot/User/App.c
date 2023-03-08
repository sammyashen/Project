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
static task_t boot_task;
static task_t boot_fsm_task;
static struct state_machine boot_fsm;
static struct state boot_idle, boot_fw_info, boot_recv_code; 
static struct state boot_recv_finish, boot_wait_finish_ack, boot_jump, boot_error;
static struct state boot_before_jump;

static bool boot_evt_compare(void *key, struct event *event)
{
	if(event->type != BOOT_EVT)   		return false;
	if(strcmp(key, event->data) == 0)	return true;
	else								return false;
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

static uint8_t env_update_flag_e = 0;
static __IO uint32_t recv_code_offset = 0;
frame_recv_t frame_recv_struct;

static void boot_idle_exit(void *state_data, struct event *event)
{
	recv_code_offset = 0;
	flash_read(ENV_UPDATE_FLAG_ADDR, (uint8_t *)&env_update_flag_e, 1);
	if(env_update_flag_e == APP_UPDATE){			//擦除APP下载区代码
		flash_erase(APP_FW_DOWNLOAD_START_ADDR, APP_FW_DOWNLOAD_SIZE);
	}else if(env_update_flag_e == BOOT_UPDATE){		//擦除BOOT下载区代码
		flash_erase(BOOT_FW_DOWNLOAD_START_ADDR, BOOT_FW_DOWNLOAD_SIZE);
	}
}
static struct state boot_idle = {   
	.transitions = (struct transition[]){
		{BOOT_EVT, (void *)"start", boot_evt_compare, NULL, &boot_fw_info},   
	},
	.transition_nums = 1,
	.action_exit = &boot_idle_exit,
};

static void boot_fw_info_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint8_t buf[264] = {0};
	uint16_t size = 0;
	uint16_t crc16 = 0;
	uint8_t env_fw_version = 0, env_fw_version_bk = 0;
	uint32_t fw_size = 0;
	frame_single_t _frame_single_data;
	uint8_t env_sector_bk[4*10] = {0xFF};

	size = fifo_read(&frame_recv_struct.rx_fifo, (uint8_t *)&_frame_single_data, sizeof(frame_single_t));
	if(size == sizeof(frame_single_t)){
		buf[0] = _frame_single_data.start;
		buf[1] = _frame_single_data.cmd;
		buf[2] = _frame_single_data.frame;
		buf[3] = _frame_single_data.length;
		memcpy(&buf[4], &_frame_single_data.data[0], _frame_single_data.length);
		buf[4+_frame_single_data.length] = (uint8_t)(_frame_single_data.crc>>8);
		buf[5+_frame_single_data.length] = (uint8_t)_frame_single_data.crc;
		crc16_calculate(buf, (4+_frame_single_data.length), (~_frame_single_data.cmd), &crc16);
		if(crc16 == _frame_single_data.crc){
			if((_frame_single_data.start == 0xA5) && (_frame_single_data.cmd == CMD_UPDATE_INFO) && (_frame_single_data.frame == 0x5A)){
				flash_read(ENV_FW_VERSION_ADDR, (uint8_t *)&env_fw_version_bk, 1);
				env_fw_version = _frame_single_data.data[1];
				fw_size = (uint32_t)((_frame_single_data.data[4]<<24)|(_frame_single_data.data[5]<<16)|(_frame_single_data.data[6]<<8)|_frame_single_data.data[7]);
				if(env_fw_version != env_fw_version_bk){
					//存储fw_info进flash
					if(env_update_flag_e == APP_UPDATE){
						if(fw_size < (8*1024) || fw_size > APP_FW_SIZE){		//固件大小不对
							protocol_notify(CMD_UPDATE_INFO, ACK_ERR_INVALID_APP, 0x01, &(uint8_t){0x00});
							
							flash_read(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
							env_sector_bk[0] = UPDATE_ERR;
							flash_erase(ENV_UPDATE_FLAG_ADDR, sizeof(env_sector_bk));
							flash_write(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
							
							tiny_set_event(&boot_fsm_task, EVT_5);
						}else{
							protocol_notify(CMD_UPDATE_INFO, ACK_NEED_UPDATE, 0x01, &(uint8_t){0x00});
						
							flash_write((APP_FW_DOWNLOAD_START_ADDR+APP_FW_BASE_INFO_OFFSET), 
										_frame_single_data.data, _frame_single_data.length);
							tiny_set_event(&boot_fsm_task, EVT_2);
						}
					}else if(env_update_flag_e == BOOT_UPDATE){
						if(fw_size < (8*1024) || fw_size > BOOT_FW_SIZE){
							protocol_notify(CMD_UPDATE_INFO, ACK_ERR_INVALID_APP, 0x01, &(uint8_t){0x00});
							
							flash_read(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
							env_sector_bk[0] = UPDATE_ERR;
							flash_erase(ENV_UPDATE_FLAG_ADDR, sizeof(env_sector_bk));
							flash_write(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
							
							tiny_set_event(&boot_fsm_task, EVT_5);
						}else{
							protocol_notify(CMD_UPDATE_INFO, ACK_NEED_UPDATE, 0x01, &(uint8_t){0x00});
						
							flash_write((BOOT_FW_DOWNLOAD_START_ADDR+BOOT_FW_BASE_INFO_OFFSET), 
											_frame_single_data.data, _frame_single_data.length);
							tiny_set_event(&boot_fsm_task, EVT_2);
						}
					}else{
						protocol_notify(CMD_UPDATE_INFO, ACK_ERR_UPDATE_STATUS, 0x01, &(uint8_t){0x00});

						flash_read(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
						env_sector_bk[0] = UPDATE_ERR;
						flash_erase(ENV_UPDATE_FLAG_ADDR, sizeof(env_sector_bk));
						flash_write(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
						
						tiny_set_event(&boot_fsm_task, EVT_5);
					}
				}else{
					protocol_notify(CMD_UPDATE_INFO, ACK_NO_NEED_UPDATE, 0x01, &(uint8_t){0x00});
					
					flash_read(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
					env_sector_bk[0] = UPDATE_ERR;
					flash_erase(ENV_UPDATE_FLAG_ADDR, sizeof(env_sector_bk));
					flash_write(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
					
					tiny_set_event(&boot_fsm_task, EVT_5);
				}
			}else{
				protocol_notify(CMD_UPDATE_INFO, ACK_ERR_FRAME_FLAG, 0x01, &(uint8_t){0x00});

				flash_read(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
				env_sector_bk[0] = UPDATE_ERR;
				flash_erase(ENV_UPDATE_FLAG_ADDR, sizeof(env_sector_bk));
				flash_write(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
				
				tiny_set_event(&boot_fsm_task, EVT_5);
			}
		}else{
			protocol_notify(CMD_UPDATE_INFO, ACK_ERR_CRC16, 0x01, &(uint8_t){0x00});

			flash_read(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
			env_sector_bk[0] = UPDATE_ERR;
			flash_erase(ENV_UPDATE_FLAG_ADDR, sizeof(env_sector_bk));
			flash_write(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
			
			tiny_set_event(&boot_fsm_task, EVT_5);
		}
	}
}
static struct state boot_fw_info = {
	.transitions = (struct transition[]){  
		{BOOT_EVT, (void *)"change", boot_evt_compare, NULL, &boot_recv_code},
		{BOOT_EVT, (void *)"hold", boot_evt_compare, &boot_fw_info_process, &boot_fw_info},
		{BOOT_EVT, (void *)"delay", boot_evt_compare, NULL, &boot_before_jump},
	},
	.transition_nums = 3,
};

static void boot_recv_code_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint8_t buf[264] = {0};
	uint16_t size = 0;
	uint16_t crc16 = 0;
	uint32_t image_size = 0;
	frame_single_t _frame_single_data;
	uint8_t env_sector_bk[4*10] = {0xFF};

	size = fifo_read(&frame_recv_struct.rx_fifo, (uint8_t *)&_frame_single_data, sizeof(frame_single_t));
	if(size == sizeof(frame_single_t)){
		buf[0] = _frame_single_data.start;
		buf[1] = _frame_single_data.cmd;
		buf[2] = _frame_single_data.frame;
		buf[3] = _frame_single_data.length;
		memcpy(&buf[4], &_frame_single_data.data[0], _frame_single_data.length);
		buf[4+_frame_single_data.length] = (uint8_t)(_frame_single_data.crc>>8);
		buf[5+_frame_single_data.length] = (uint8_t)_frame_single_data.crc;
		crc16_calculate(buf, (4+_frame_single_data.length), (~_frame_single_data.cmd), &crc16);
		if(crc16 == _frame_single_data.crc){
			if(_frame_single_data.frame != 0x00 && recv_code_offset < 10){
				protocol_notify(CMD_UPDATING, ACK_ERR_INVALID_FRAME, 0x01, &(uint8_t){0x00});

				flash_read(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
				env_sector_bk[0] = UPDATE_ERR;
				flash_erase(ENV_UPDATE_FLAG_ADDR, sizeof(env_sector_bk));
				flash_write(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
				
				tiny_set_event(&boot_fsm_task, EVT_5);
				return;
			}
		
			if(env_update_flag_e == APP_UPDATE){	
				flash_write((APP_FW_DOWNLOAD_START_ADDR+APP_FW_CODE_OFFSET+recv_code_offset), 
								_frame_single_data.data, _frame_single_data.length);
				
				if((_frame_single_data.length%4) != 0){
					recv_code_offset += ((_frame_single_data.length/4 + 1)*4) ;
				}else{
					recv_code_offset += _frame_single_data.length;
				}

				protocol_notify(CMD_UPDATING, ACK_OK, 0x01, &(uint8_t){0x00});

				uint8_t tmp[4] = {0};
				flash_read((APP_FW_DOWNLOAD_START_ADDR+APP_FW_SIZE_OFFSET), tmp, 4);
				image_size = (uint32_t)((tmp[0]<<24)|(tmp[1]<<16)|(tmp[2]<<8)|tmp[3]);
				if(recv_code_offset >= image_size){
					tiny_set_event(&boot_fsm_task, EVT_2);
				}
			}else if(env_update_flag_e == BOOT_UPDATE){
				flash_write((BOOT_FW_DOWNLOAD_START_ADDR+BOOT_FW_CODE_OFFSET+recv_code_offset), 
								_frame_single_data.data, _frame_single_data.length);
								
				if((_frame_single_data.length%4) != 0){
					recv_code_offset += ((_frame_single_data.length/4 + 1)*4) ;
				}else{
					recv_code_offset += _frame_single_data.length;
				}

				protocol_notify(CMD_UPDATING, ACK_OK, 0x01, &(uint8_t){0x00});

				uint8_t tmp[4] = {0};
				flash_read((BOOT_FW_DOWNLOAD_START_ADDR+BOOT_FW_SIZE_OFFSET), tmp, 4);
				image_size = (uint32_t)((tmp[0]<<24)|(tmp[1]<<16)|(tmp[2]<<8)|tmp[3]);
				if(recv_code_offset >= image_size){
					tiny_set_event(&boot_fsm_task, EVT_2);
				}
			}else{
				protocol_notify(CMD_UPDATING, ACK_ERR_UPDATE_STATUS, 0x01, &(uint8_t){0x00});

				flash_read(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
				env_sector_bk[0] = UPDATE_ERR;
				flash_erase(ENV_UPDATE_FLAG_ADDR, sizeof(env_sector_bk));
				flash_write(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
				
				tiny_set_event(&boot_fsm_task, EVT_5);
			}
		}else{
			protocol_notify(CMD_UPDATING, ACK_ERR_CRC16, 0x01, &(uint8_t){0x00});
		}
	}
}
static struct state boot_recv_code = {
	.transitions = (struct transition[]){  
		{BOOT_EVT, (void *)"change", boot_evt_compare, NULL, &boot_recv_finish},
		{BOOT_EVT, (void *)"hold", boot_evt_compare, &boot_recv_code_process, &boot_recv_code},
		{BOOT_EVT, (void *)"delay", boot_evt_compare, NULL, &boot_before_jump},
	},
	.transition_nums = 3,
};

static uint8_t finish_delay_cnt = 0;
static void boot_recv_finish_entry(void *state_data, struct event *event)
{
	finish_delay_cnt = 0;
}
static void boot_recv_finish_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint32_t image_crc32 = 0;
	uint32_t check_image_crc32 = 0;
	uint32_t image_size = 0;
	uint8_t *ptr = NULL;
	uint8_t env_sector_bk[4*10] = {0xFF};

	if(++finish_delay_cnt >= 5){
		finish_delay_cnt = 0;
		if(env_update_flag_e == APP_UPDATE){
			uint8_t tmp[4] = {0};
			flash_read((APP_FW_DOWNLOAD_START_ADDR+APP_FW_CRC_OFFSET), tmp, 4);
			image_crc32 = (uint32_t)((tmp[0]<<24)|(tmp[1]<<16)|(tmp[2]<<8)|tmp[3]);	//小端存储，重新组数据
			flash_read((APP_FW_DOWNLOAD_START_ADDR+APP_FW_SIZE_OFFSET), tmp, 4);
			image_size = (uint32_t)((tmp[0]<<24)|(tmp[1]<<16)|(tmp[2]<<8)|tmp[3]);
			ptr = (uint8_t *)(APP_FW_DOWNLOAD_START_ADDR+APP_FW_CODE_OFFSET);
			crc32(ptr, image_size, &check_image_crc32);
		}else if(env_update_flag_e == BOOT_UPDATE){
			uint8_t tmp[4] = {0};
			flash_read((BOOT_FW_DOWNLOAD_START_ADDR+BOOT_FW_CRC_OFFSET), tmp, 4);
			image_crc32 = (uint32_t)((tmp[0]<<24)|(tmp[1]<<16)|(tmp[2]<<8)|tmp[3]);
			flash_read((BOOT_FW_DOWNLOAD_START_ADDR+BOOT_FW_SIZE_OFFSET), tmp, 4);
			image_size = (uint32_t)((tmp[0]<<24)|(tmp[1]<<16)|(tmp[2]<<8)|tmp[3]);
			ptr = (uint8_t *)(BOOT_FW_DOWNLOAD_START_ADDR+BOOT_FW_CODE_OFFSET);
			crc32(ptr, image_size, &check_image_crc32);
		}

		if(image_crc32 != check_image_crc32){
			protocol_notify(CMD_UPDATED, ACK_ERR_CRC32, 0x01, &(uint8_t){0x00});

			flash_read(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
			env_sector_bk[0] = UPDATE_ERR;
			flash_erase(ENV_UPDATE_FLAG_ADDR, sizeof(env_sector_bk));
			flash_write(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
			
			tiny_set_event(&boot_fsm_task, EVT_5);
		}else{
			protocol_notify(CMD_UPDATED, ACK_OK, 0x01, &(uint8_t){0x00});
			tiny_set_event(&boot_fsm_task, EVT_2);
		}
	}
}
static struct state boot_recv_finish = {	 
	.transitions = (struct transition[]){
		{BOOT_EVT, (void *)"change", boot_evt_compare, NULL, &boot_wait_finish_ack},  
		{BOOT_EVT, (void *)"hold", boot_evt_compare, &boot_recv_finish_process, &boot_recv_finish},
		{BOOT_EVT, (void *)"delay", boot_evt_compare, NULL, &boot_before_jump},
	},
	.transition_nums = 3,
	.action_entry = &boot_recv_finish_entry,
};

static void boot_wait_finish_ack_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint8_t buf[264] = {0};
	uint16_t size = 0;
	uint16_t crc16 = 0;
	frame_single_t _frame_single_data;
	uint8_t env_sector_bk[4*10] = {0xFF};

	size = fifo_read(&frame_recv_struct.rx_fifo, (uint8_t *)&_frame_single_data, sizeof(frame_single_t));
	if(size == sizeof(frame_single_t)){
		buf[0] = _frame_single_data.start;
		buf[1] = _frame_single_data.cmd;
		buf[2] = _frame_single_data.frame;
		buf[3] = _frame_single_data.length;
		memcpy(&buf[4], &_frame_single_data.data[0], _frame_single_data.length);
		buf[4+_frame_single_data.length] = (uint8_t)(_frame_single_data.crc>>8);
		buf[5+_frame_single_data.length] = (uint8_t)_frame_single_data.crc;
		crc16_calculate(buf, (4+_frame_single_data.length), (~_frame_single_data.cmd), &crc16);
		if(crc16 == _frame_single_data.crc){
			if((_frame_single_data.start == 0xA5) && (_frame_single_data.cmd == CMD_UPDATE_REBOOT) && (_frame_single_data.frame == 0x5A)){
				protocol_notify(CMD_UPDATE_REBOOT, ACK_OK, 0x01, &(uint8_t){0x00});
				tiny_set_event(&boot_fsm_task, EVT_2);
			}
		}else{
			protocol_notify(CMD_UPDATE_REBOOT, ACK_ERR_CRC16, 0x01, &(uint8_t){0x00});

			flash_read(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
			env_sector_bk[0] = UPDATE_ERR;
			flash_erase(ENV_UPDATE_FLAG_ADDR, sizeof(env_sector_bk));
			flash_write(ENV_UPDATE_FLAG_ADDR, env_sector_bk, sizeof(env_sector_bk));
			
			tiny_set_event(&boot_fsm_task, EVT_5);
		}
	}
}
static struct state boot_wait_finish_ack = {
	.transitions = (struct transition[]){  
		{BOOT_EVT, (void *)"change", boot_evt_compare, NULL, &boot_before_jump},
		{BOOT_EVT, (void *)"hold", boot_evt_compare, &boot_wait_finish_ack_process, &boot_wait_finish_ack},
		{BOOT_EVT, (void *)"delay", boot_evt_compare, NULL, &boot_before_jump},
	},
	.transition_nums = 3,
};

static void boot_before_jump_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	static uint8_t delay_cnt = 0;

	if(++delay_cnt >= 5){
		delay_cnt = 0;
		tiny_set_event(&boot_fsm_task, EVT_2);
	}
}
static struct state boot_before_jump = {
	.transitions = (struct transition[]){  
		{BOOT_EVT, (void *)"change", boot_evt_compare, NULL, &boot_jump},
		{BOOT_EVT, (void *)"hold", boot_evt_compare, &boot_before_jump_process, &boot_before_jump},
	},
	.transition_nums = 2,
};

static void boot_jump_entry(void *state_data, struct event *event)
{
	//复位系统
	 __set_FAULTMASK(1);     
    NVIC_SystemReset();
}
static struct state boot_jump = {	   
	.action_entry = &boot_jump_entry,
};

/*
 *EVT_0: send "start"
 */
static void boot_task_cb(void *para, uint32_t evt)
{
	if(evt & EVT_0){
		tiny_clr_event(&boot_task, EVT_0);
		tiny_set_event(&boot_fsm_task, EVT_0);
	}
	tiny_set_event(&boot_fsm_task, EVT_4);

	//喂狗，初始化已在bios中完成，boot只需喂狗，无需重新配置SWDT
	SWDT_RefreshCounter();
}

/*
 *EVT_0: send "start"
 *EVT_1: send "stop"
 *EVT_2: send "change"
 *EVT_3: send "jump"
 *EVT_4: send "hold"
 *EVT_5: send "delay"
 */
static void boot_fsm_task_cb(void *para, uint32_t evt)
{
	//boot fsm
	if(evt & EVT_0){
		tiny_clr_event(&boot_fsm_task, EVT_0);
		statem_handle_event(&boot_fsm, &(struct event){BOOT_EVT, (void *)"start"});
	}
	if(evt & EVT_1){
		tiny_clr_event(&boot_fsm_task, EVT_1);
		statem_handle_event(&boot_fsm, &(struct event){BOOT_EVT, (void *)"stop"});
	}
	if(evt & EVT_2){
		tiny_clr_event(&boot_fsm_task, EVT_2);
		statem_handle_event(&boot_fsm, &(struct event){BOOT_EVT, (void *)"change"});
	}
	if(evt & EVT_3){
		tiny_clr_event(&boot_fsm_task, EVT_3);
		statem_handle_event(&boot_fsm, &(struct event){BOOT_EVT, (void *)"jump"});
	}
	if(evt & EVT_4){
		tiny_clr_event(&boot_fsm_task, EVT_4);
		statem_handle_event(&boot_fsm, &(struct event){BOOT_EVT, (void *)"hold"});
	}
	if(evt & EVT_5){
		tiny_clr_event(&boot_fsm_task, EVT_5);
		statem_handle_event(&boot_fsm, &(struct event){BOOT_EVT, (void *)"delay"});
	}
	
	uart_poll_dma_tx(DEV_UART1);
}

void app_init(void)
{
    tiny_timer_create(&boot_task, boot_task_cb, NULL);
    tiny_timer_start(&boot_task, TIMER_FOREVER, 10);
    tiny_task_create(&boot_fsm_task, boot_fsm_task_cb, NULL);

    statem_init(&boot_fsm, &boot_idle, &boot_error);
	//直接启动boot
	tiny_set_event(&boot_task, EVT_0);
}
app_initcall(app_init);

int32_t main(void)
{
	SCB->VTOR = ((uint32_t)BOOT_FW_START_ADDR & SCB_VTOR_TBLOFF_Msk);
	__disable_irq();
	do_init_call();
	__enable_irq();
	while(1)
	{
		tiny_task_loop();
	}
}

