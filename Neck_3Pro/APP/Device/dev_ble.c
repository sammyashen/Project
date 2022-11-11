#include "dev_ble.h"

static task_t ble_task;
static task_t ble_fsm_task;

//BLE状态机及其状态定义
static struct state_machine ble_fsm;
static struct state ble_idle, ble_pwr_on, ble_init, ble_pwr_off;
static struct state ble_init_idle, ble_init_bleCFG, ble_init_getMAC, ble_init_setADV, ble_init_createSRV;
static struct state ble_init_startSRV, ble_init_setMSG, ble_init_sppCFG, ble_init_startADV, ble_init_waitNotify;
static struct state ble_init_setSPP, ble_init_spp, ble_init_enterCMD, ble_init_stopADV;

static bool ble_evt_compare(void *key, struct event *event)
{
	if(event->type != BLE_EVT)   		return false;
	if(strcmp(key, event->data) == 0)	return true;
	else								return false;
}

static struct state ble_idle = {   
	.transitions = (struct transition[]){
		{BLE_EVT, (void *)"start", ble_evt_compare, NULL, &ble_pwr_on},   
	},
	.transition_nums = 1,
};

static void ble_pwr_on_entery(void *state_data, struct event *event)
{
	BLE_PWR_ENABLE;
}
static void ble_pwr_on_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint8_t size = 0;
	uint8_t buf[256] = {0};

	size = uart_read(DEV_UART4, buf, 256);
	if(size != 0){
		for(uint8_t i=0;i<size;i++){			
			if(buf[i] == 'r' && buf[i+4] == 'y'){			//got "ready"
				tiny_set_event(&ble_fsm_task, EVT_3);
				break;
			}
		}
	}
}
static struct state ble_pwr_on = {
	.transitions = (struct transition[]){
		{BLE_EVT, (void *)"hold", ble_evt_compare, &ble_pwr_on_process, &ble_pwr_on},   
		{BLE_EVT, (void *)"change", ble_evt_compare, NULL, &ble_init},   
		{BLE_EVT, (void *)"stop", ble_evt_compare, NULL, &ble_pwr_off},
	},
	.transition_nums = 3,
	.action_entry = &ble_pwr_on_entery,
};

static struct state ble_init = {
	.state_entry = &ble_init_idle,
	.transitions = (struct transition[]){  
		{BLE_EVT, (void *)"stop", ble_evt_compare, NULL, &ble_pwr_off},
	},
	.transition_nums = 1,
};

static void ble_init_idle_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	tiny_set_event(&ble_fsm_task, EVT_3);
}
static struct state ble_init_idle = {	   
	.state_parent = &ble_init,
	.transitions = (struct transition[]){
		{BLE_EVT, (void *)"hold", ble_evt_compare, &ble_init_idle_process, &ble_init_idle}, 
		{BLE_EVT, (void *)"change", ble_evt_compare, NULL, &ble_init_bleCFG},
	},
	.transition_nums = 2,
}; 

static void ble_init_bleCFG_entery(void *state_data, struct event *event)
{
	uart_write(DEV_UART4, "AT+BLEINIT=2\r\n", 14);	
}
static void ble_init_bleCFG_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint8_t size = 0;
	uint8_t buf[256] = {0};

	size = uart_read(DEV_UART4, buf, 256);
	if(size != 0){
		for(uint8_t i=0;i<size;i++){			
			if(buf[i] == 'O' && buf[i+1] == 'K'){			//got "OK"
				tiny_set_event(&ble_fsm_task, EVT_3);
				break;
			}
		}
	}
}
static struct state ble_init_bleCFG = {	   
	.state_parent = &ble_init,
	.transitions = (struct transition[]){
		{BLE_EVT, (void *)"hold", ble_evt_compare, &ble_init_bleCFG_process, &ble_init_bleCFG}, 
		{BLE_EVT, (void *)"change", ble_evt_compare, NULL, &ble_init_getMAC},
	},
	.transition_nums = 2,
	.action_entry = &ble_init_bleCFG_entery,
}; 

static uint8_t BLEMAC[6] = {0};
static uint8_t BLEMAC_L3str[6] = {0};
static void str2hex(uint8_t *hex, uint8_t *str, uint8_t len)
{
	uint8_t h1 = 0,h2 = 0;
	
	for(uint8_t i=0;i<len;i++){
		if(str[2*i] >= '0' && str[2*i] <= '9'){
			h1 = str[2*i] - '0' + 0x00;
		}
		else if(str[2*i] >= 'A' && str[2*i] <= 'F'){
			h1 = str[2*i] - 'A' + 0x0A;
		}
		else if(str[2*i] >= 'a' && str[2*i] <= 'f'){
			h1 = str[2*i] - 'a' + 0x0A;
		}
		else{
			h1 = 0xFF;
		}

		if(str[2*i+1] >= '0' && str[2*i+1] <= '9'){
			h2 = str[2*i+1] - '0' + 0x00;
		}
		else if(str[2*i+1] >= 'A' && str[2*i+1] <= 'F'){
			h2 = str[2*i+1] - 'A' + 0x0A;
		}
		else if(str[2*i+1] >= 'a' && str[2*i+1] <= 'f'){
			h2 = str[2*i+1] - 'a' + 0x0A;
		}
		else{
			h2 = 0xFF;
		}

		if(h1 != 0xFF && h2 != 0xFF){
			hex[i] = (h1<<4) + h2;
		}
	}
}
static void ble_init_getMAC_entery(void *state_data, struct event *event)
{
	uart_write(DEV_UART4, "AT+BLEADDR?\r\n", 13);
}
static void ble_init_getMAC_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint8_t size = 0;
	uint8_t buf[256] = {0};
	uint8_t mac_str[6*2] = {0};
	uint8_t j = 0, k = 0;

	size = uart_read(DEV_UART4, buf, 256);
	if(size != 0){
		for(uint8_t i=0;i<size;i++){			
			if(buf[i] == ':' && buf[i+1] == '"'){	//got +BLEADDR:"a0:76:4e:6c:c9:ee"
				j = i+2;
				while(buf[j] != '"'){
					if(buf[j] != ':'){
						mac_str[k++] = buf[j];
					}
					j++;
				}
				strncpy((char *)BLEMAC_L3str, (char *)&mac_str[3*2], 6);
				str2hex(BLEMAC, mac_str, 6);
				tiny_set_event(&ble_fsm_task, EVT_3);
				break;
			}
		}
	}
}
static struct state ble_init_getMAC = {	   
	.state_parent = &ble_init,
	.transitions = (struct transition[]){
		{BLE_EVT, (void *)"hold", ble_evt_compare, &ble_init_getMAC_process, &ble_init_getMAC}, 
		{BLE_EVT, (void *)"change", ble_evt_compare, NULL, &ble_init_setADV},
	},
	.transition_nums = 2,
	.action_entry = &ble_init_getMAC_entery,
}; 

static void ble_init_setADV_entery(void *state_data, struct event *event)
{
	uint8_t ble_name[10] = "iNeck 3Pro";
	uint8_t ble_adv[80] = {0};

	sprintf((char *)ble_adv, "AT+BLEADVDATAEX=\"%s_%s\",\"00FF\",\"112233445566\",1\r\n", ble_name, BLEMAC_L3str);
	uart_write(DEV_UART4, ble_adv, strlen((char *)ble_adv));
}
static void ble_init_setADV_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint8_t size = 0;
	uint8_t buf[256] = {0};

	size = uart_read(DEV_UART4, buf, 256);
	if(size != 0){
		for(uint8_t i=0;i<size;i++){			
			if(buf[i] == 'O' && buf[i+1] == 'K'){			//got "OK"
				tiny_set_event(&ble_fsm_task, EVT_3);
				break;
			}
		}
	}
}
static struct state ble_init_setADV = {	   
	.state_parent = &ble_init,
	.transitions = (struct transition[]){
		{BLE_EVT, (void *)"hold", ble_evt_compare, &ble_init_setADV_process, &ble_init_setADV}, 
		{BLE_EVT, (void *)"change", ble_evt_compare, NULL, &ble_init_createSRV},
	},
	.transition_nums = 2,
	.action_entry = &ble_init_setADV_entery,
}; 

static void ble_init_createSRV_entery(void *state_data, struct event *event)
{
	uart_write(DEV_UART4, "AT+BLEGATTSSRVCRE\r\n", 19);
}
static void ble_init_createSRV_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint8_t size = 0;
	uint8_t buf[256] = {0};

	size = uart_read(DEV_UART4, buf, 256);
	if(size != 0){
		for(uint8_t i=0;i<size;i++){			
			if(buf[i] == 'O' && buf[i+1] == 'K'){			//got "OK"
				tiny_set_event(&ble_fsm_task, EVT_3);
				break;
			}
		}
	}
}
static struct state ble_init_createSRV = {	   
	.state_parent = &ble_init,
	.transitions = (struct transition[]){
		{BLE_EVT, (void *)"hold", ble_evt_compare, &ble_init_createSRV_process, &ble_init_createSRV}, 
		{BLE_EVT, (void *)"change", ble_evt_compare, NULL, &ble_init_startSRV},
	},
	.transition_nums = 2,
	.action_entry = &ble_init_createSRV_entery,
};

static void ble_init_startSRV_entery(void *state_data, struct event *event)
{
	uart_write(DEV_UART4, "AT+BLEGATTSSRVSTART=1\r\n", 23);
}
static void ble_init_startSRV_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint8_t size = 0;
	uint8_t buf[256] = {0};

	size = uart_read(DEV_UART4, buf, 256);
	if(size != 0){
		for(uint8_t i=0;i<size;i++){			
			if(buf[i] == 'O' && buf[i+1] == 'K'){			//got "OK"
				tiny_set_event(&ble_fsm_task, EVT_3);
				break;
			}
		}
	}
}
static struct state ble_init_startSRV = {	   
	.state_parent = &ble_init,
	.transitions = (struct transition[]){
		{BLE_EVT, (void *)"hold", ble_evt_compare, &ble_init_startSRV_process, &ble_init_startSRV}, 
		{BLE_EVT, (void *)"change", ble_evt_compare, NULL, &ble_init_setMSG},
	},
	.transition_nums = 2,
	.action_entry = &ble_init_startSRV_entery,
};

static void ble_init_setMSG_entery(void *state_data, struct event *event)
{
	uart_write(DEV_UART4, "AT+SYSMSG=4\r\n", 13);
}
static void ble_init_setMSG_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint8_t size = 0;
	uint8_t buf[256] = {0};

	size = uart_read(DEV_UART4, buf, 256);
	if(size != 0){
		for(uint8_t i=0;i<size;i++){			
			if(buf[i] == 'O' && buf[i+1] == 'K'){			//got "OK"
				tiny_set_event(&ble_fsm_task, EVT_3);
				break;
			}
		}
	}
}
static struct state ble_init_setMSG = {	   
	.state_parent = &ble_init,
	.transitions = (struct transition[]){
		{BLE_EVT, (void *)"hold", ble_evt_compare, &ble_init_setMSG_process, &ble_init_setMSG}, 
		{BLE_EVT, (void *)"change", ble_evt_compare, NULL, &ble_init_sppCFG},
	},
	.transition_nums = 2,
	.action_entry = &ble_init_setMSG_entery,
};

static void ble_init_sppCFG_entery(void *state_data, struct event *event)
{
	uart_write(DEV_UART4, "AT+BLESPPCFG=1,1,6,1,5\r\n", 24);
}
static void ble_init_sppCFG_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint8_t size = 0;
	uint8_t buf[256] = {0};

	size = uart_read(DEV_UART4, buf, 256);
	if(size != 0){
		for(uint8_t i=0;i<size;i++){			
			if(buf[i] == 'O' && buf[i+1] == 'K'){			//got "OK"
				tiny_set_event(&ble_fsm_task, EVT_3);
				break;
			}
		}
	}
}
static struct state ble_init_sppCFG = {	   
	.state_parent = &ble_init,
	.transitions = (struct transition[]){
		{BLE_EVT, (void *)"hold", ble_evt_compare, &ble_init_sppCFG_process, &ble_init_sppCFG}, 
		{BLE_EVT, (void *)"change", ble_evt_compare, NULL, &ble_init_startADV},
	},
	.transition_nums = 2,
	.action_entry = &ble_init_sppCFG_entery,
};

static void ble_init_startADV_entery(void *state_data, struct event *event)
{
	uart_write(DEV_UART4, "AT+BLEADVSTART\r\n", 16);
}
static void ble_init_startADV_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint8_t size = 0;
	uint8_t buf[256] = {0};

	size = uart_read(DEV_UART4, buf, 256);
	if(size != 0){
		for(uint8_t i=0;i<size;i++){			
			if(buf[i] == 'O' && buf[i+1] == 'K'){			//got "OK"
				tiny_set_event(&ble_fsm_task, EVT_3);
				break;
			}
		}
	}
}
static struct state ble_init_startADV = {	   
	.state_parent = &ble_init,
	.transitions = (struct transition[]){
		{BLE_EVT, (void *)"hold", ble_evt_compare, &ble_init_startADV_process, &ble_init_startADV}, 
		{BLE_EVT, (void *)"change", ble_evt_compare, NULL, &ble_init_waitNotify},
	},
	.transition_nums = 2,
	.action_entry = &ble_init_startADV_entery,
};

static void ble_init_waitNotify_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint8_t size = 0;
	uint8_t buf[256] = {0};

	size = uart_read(DEV_UART4, buf, 256);
	if(size != 0){
		for(uint8_t i=0;i<size;i++){			
			if(buf[i] == 'W' && buf[i+4] == 'E'){			//got "+WRITE:0,1,6,1,2"
				tiny_set_event(&ble_fsm_task, EVT_3);
				break;
			}
		}
	}
}
static struct state ble_init_waitNotify = {	   
	.state_parent = &ble_init,
	.transitions = (struct transition[]){
		{BLE_EVT, (void *)"hold", ble_evt_compare, &ble_init_waitNotify_process, &ble_init_waitNotify}, 
		{BLE_EVT, (void *)"change", ble_evt_compare, NULL, &ble_init_setSPP},
	},
	.transition_nums = 2,
};

static void ble_init_setSPP_entery(void *state_data, struct event *event)
{
	iNeck_3Pro.ble_status = CONNECTED;
	Topic_Pushlish(LED_TOPIC, &(uint32_t){EVT_5});
	uart_write(DEV_UART4, "AT+BLESPP\r\n", 11);
}
static void ble_init_setSPP_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint8_t size = 0;
	uint8_t buf[256] = {0};

	size = uart_read(DEV_UART4, buf, 256);
	if(size != 0){
		for(uint8_t i=0;i<size;i++){			
			if(buf[i] == '>'){			//got ">"
				tiny_set_event(&ble_fsm_task, EVT_3);
				break;
			}
		}
	}
}
static struct state ble_init_setSPP = {	   
	.state_parent = &ble_init,
	.transitions = (struct transition[]){
		{BLE_EVT, (void *)"hold", ble_evt_compare, &ble_init_setSPP_process, &ble_init_setSPP}, 
		{BLE_EVT, (void *)"change", ble_evt_compare, NULL, &ble_init_spp},
	},
	.transition_nums = 2,
	.action_entry = &ble_init_setSPP_entery,
};
/***********************************************蓝牙数据处理*******************************************************************/
static uint8_t checksum(uint8_t *buf, uint8_t pack_len)
{
    uint16_t cnt = 0, chekssum = 0;               

    chekssum = 0x00;

    for(cnt=0;cnt<(pack_len);cnt++)
    {
        chekssum += buf[cnt];
    }
    chekssum=(~chekssum)&0xff;

    return chekssum;
}

static void ble_frame_pack(uint8_t *frame, uint8_t cmd, uint8_t flag, uint8_t *data, uint8_t data_len)
{
	frame[0] = FRAME_HEAD;
	frame[1] = VERSION;
	frame[2] = DEVICE;
	frame[3] = cmd;
	frame[4] = 0x00;
	frame[5] = flag;
	strncpy((char *)&frame[6], (char *)data, data_len);
	frame[6+data_len] = checksum(&frame[1], (5+data_len));
	frame[6+data_len+1] = FRAME_TAIL;
}

static void ble_notify_handshake(void)
{
	uint8_t frame[30] = {0};

	frame[0] = FRAME_HEAD;
	frame[1] = VERSION;
	frame[2] = DEVICE;
	frame[3] = 0x01;
	frame[4] = 0x00;
	frame[5] = 0x00;
	frame[6] = 0x01;
    frame[7] = 0x08;
    frame[8] = DEVICE;
    frame[9] = 0x04;
    frame[10] = 0x00;
    frame[11] = 0x00;
    frame[12] = DEVICE;
    frame[13] = 0x04;
    frame[14] = 0x03;
    frame[15] = 0x01;
  	frame[16] = checksum(&frame[1], 15);
	frame[17] = FRAME_TAIL;

	uart_write(DEV_UART4, frame, 18);
}

static void ble_heart_pack_notify(void)
{
	uint8_t frame[30] = {0};

	frame[0] = FRAME_HEAD;
	frame[1] = VERSION;
	frame[2] = DEVICE;
	frame[3] = 0x03;
	frame[4] = 0x00;
	frame[5] = 0xFF;
	frame[6] = 0x69;
	frame[7] = 0x0C;
	frame[8] = iNeck_3Pro.mass_mode;
	frame[9] = (iNeck_3Pro.mass_remain/60);
	frame[10] = iNeck_3Pro.heat_level;
	frame[11] = iNeck_3Pro.bat1_level;
	frame[12] = iNeck_3Pro.bat2_level;
	if(iNeck_3Pro.is_travel)	frame[13] = 0x01;
	else						frame[13] = 0x00;
	frame[14] = iNeck_3Pro.mass_motor1_speed;
	frame[15] = iNeck_3Pro.mass_motor2_speed;
	frame[16] = iNeck_3Pro.mass_motor1_dir;
	frame[17] = iNeck_3Pro.mass_motor2_dir;
	frame[18] = 0x00;
	frame[19] = 0x00;
	frame[20] = checksum(&frame[1], 19);
	frame[21] = FRAME_TAIL;

	uart_write(DEV_UART4, frame, 22);
}

static void ble_set_frame_process(uint8_t *frame)
{
	uint8_t i = 0;

	switch(frame[i])
	{
		case 0x09:
		break;

		case 0x40:	//时间设置
			if(frame[2] != 0x00){
				iNeck_3Pro.mass_time = frame[3]*60;
				iNeck_3Pro.mass_remain = iNeck_3Pro.mass_time;
			}
		break;

		case 0x56:	//模式设置
			if(frame[2] == 0x00){
				iNeck_3Pro.mass_mode = MASS_MODE;
				iNeck_3Pro.heat_level = HEAT_NONE;
			}else if(frame[2] == 0x01){
				iNeck_3Pro.mass_mode = KNEAD_MODE;
				iNeck_3Pro.heat_level = HEAT_NONE;
			}else if(frame[2] == 0x02){
				iNeck_3Pro.mass_mode = NECK_SOOTH_MODE;
				iNeck_3Pro.heat_level = HEAT_WARM;
			}
		break;

		case 0x59:	//电机设置
			if(frame[3] == 0x10){
				if(frame[4] == 0x01)	iNeck_3Pro.mass_motor1_dir = MASS_FORWARD;
				else					iNeck_3Pro.mass_motor1_dir = MASS_REVERSE;
				iNeck_3Pro.mass_motor1_speed = frame[2];
			}else if(frame[3] == 0x20){
				if(frame[4] == 0x01)	iNeck_3Pro.mass_motor2_dir = MASS_FORWARD;
				else					iNeck_3Pro.mass_motor2_dir = MASS_REVERSE;
				iNeck_3Pro.mass_motor2_speed = frame[2];
			}else if(frame[3] == 0x30){
				if(frame[4] == 0x01)	iNeck_3Pro.mass_motor3_dir = MASS_FORWARD;
				else					iNeck_3Pro.mass_motor3_dir = MASS_REVERSE;
				iNeck_3Pro.mass_motor3_speed = frame[2];
			}
		break;

		case 0x51:	//加热设置
			if(frame[2] != 0x00){
				iNeck_3Pro.heat_level = HEAT_WARM;
			}else{
				iNeck_3Pro.heat_level = HEAT_NONE;
			}
		break;

		default:
		break;
	}
}

static void ble_init_spp_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint8_t size = 0;
	uint8_t buf[256] = {0};
	uint8_t frame[30] = {0};

	size = uart_read(DEV_UART4, buf, 256);
	if(size != 0){			
		if(buf[0] == '+' && buf[1] == 'B'){
			if(buf[4] == 'C'){		//BLE CONN
				iNeck_3Pro.ble_status = CONNECTED;
				Topic_Pushlish(LED_TOPIC, &(uint32_t){EVT_5});
			}else if(buf[4] == 'D'){	//BLE DISCONN
				iNeck_3Pro.ble_status = DISCONNECTED;
				Topic_Pushlish(LED_TOPIC, &(uint32_t){EVT_4});
				tiny_set_event(&ble_fsm_task, EVT_3);
			}
		}
		else if(buf[0] == FRAME_HEAD && buf[size - 1] == FRAME_TAIL){
			//蓝牙数据解包:帧头[0]+版本信息[1]+设备类型[2]+命令字[3]+数据包序列号[4]+应答标志[5]+数据信息[6、7、、、N]+
			//和校验[N+1]+帧尾[N+2]
			if(buf[3] != 0x01 && buf[3] != 0x02){		//命令字错误
				if((buf[0]==0x4E)&&(buf[1]==0xff)&&(buf[2]==0xff)&&(buf[3]==0xff)&&(buf[4]==0xff)&&(buf[5]==0xff)&&(buf[6]==0x01)){
					ble_notify_handshake();
				}
				else{
					ble_frame_pack(frame, buf[3], CMD_ERR, &buf[6], (size - 8));
					uart_write(DEV_UART4, frame, size);
				}
			}
			else if(buf[size - 2] != checksum(&buf[1], (size - 2))){	//校验错误
				ble_frame_pack(frame, buf[3], SUM_ERR, &buf[6], (size - 8));
				uart_write(DEV_UART4, frame, size);
			}
			else{
				Topic_Pushlish(BUZZER_TOPIC, &(uint32_t){EVT_0});
				ble_set_frame_process(&buf[6]);
			}
		}
	}
}
static struct state ble_init_spp = {	   
	.state_parent = &ble_init,
	.transitions = (struct transition[]){
		{BLE_EVT, (void *)"hold", ble_evt_compare, &ble_init_spp_process, &ble_init_spp}, 
		{BLE_EVT, (void *)"change", ble_evt_compare, NULL, &ble_init_enterCMD}, 
	},
	.transition_nums = 2,
};
/***********************************************************************************************************************/
static void ble_init_enterCMD_entery(void *state_data, struct event *event)
{
	uart_write(DEV_UART4, "+++", 3);
}
static void ble_init_enterCMD_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	static uint8_t delay = 0;

	if(++delay >= 5){
		delay = 0;
		tiny_set_event(&ble_fsm_task, EVT_3);
	}
}
static struct state ble_init_enterCMD = {	   
	.state_parent = &ble_init,
	.transitions = (struct transition[]){
		{BLE_EVT, (void *)"hold", ble_evt_compare, &ble_init_enterCMD_process, &ble_init_enterCMD}, 
		{BLE_EVT, (void *)"change", ble_evt_compare, NULL, &ble_init_stopADV}, 
	},
	.transition_nums = 2,
	.action_entry = &ble_init_enterCMD_entery,
};

static void ble_init_stopADV_entery(void *state_data, struct event *event)
{
	uart_write(DEV_UART4, "AT+BLEADVSTOP\r\n", 15);
}
static void ble_init_stopADV_process(void *oldstate_data, struct event *event, void *state_new_data)
{
	uint8_t size = 0;
	uint8_t buf[256] = {0};

	size = uart_read(DEV_UART4, buf, 256);
	if(size != 0){
		for(uint8_t i=0;i<size;i++){			
			if(buf[i] == 'O' && buf[i+1] == 'K'){			//got "OK"
				tiny_set_event(&ble_fsm_task, EVT_3);
				break;
			}
		}
	}
}
static struct state ble_init_stopADV = {	   
	.state_parent = &ble_init,
	.transitions = (struct transition[]){
		{BLE_EVT, (void *)"hold", ble_evt_compare, &ble_init_stopADV_process, &ble_init_stopADV}, 
		{BLE_EVT, (void *)"change", ble_evt_compare, NULL, &ble_init_startADV},
	},
	.transition_nums = 2,
	.action_entry = &ble_init_stopADV_entery,
};

static void ble_pwr_off_entery(void *state_data, struct event *event)
{
	BLE_PWR_DISABLE;
}
static struct state ble_pwr_off = {
	.transitions = (struct transition[]){  
		{BLE_EVT, (void *)"start", ble_evt_compare, NULL, &ble_pwr_on},   
	},
	.transition_nums = 1,
	.action_entry = &ble_pwr_off_entery,
};

/*
*EVT_0：打开BLE
*EVT_1: 关闭BLE
*EVT_2: 上传心跳包
*/
static void ble_task_cb(void *para, uint32_t evt)
{
    if(evt & EVT_0){
		tiny_clr_event(&ble_task, EVT_0);
		tiny_set_event(&ble_fsm_task, EVT_0);
	}
	if(evt & EVT_1){
		tiny_clr_event(&ble_task, EVT_1);
		tiny_set_event(&ble_fsm_task, EVT_1);
	}
	if(evt & EVT_2){
		tiny_clr_event(&ble_task, EVT_2);
		ble_heart_pack_notify();
	}
	tiny_set_event(&ble_fsm_task, EVT_2);
}

/*
 *EVT_0：send "start"
 *EVT_1：send "stop"
 *EVT_2: send "hold"
 *EVT_3: send "change"
 */
 static void ble_fsm_task_cb(void *para, uint32_t evt)
 {
	//ble fsm
	if(evt & EVT_0){
		tiny_clr_event(&ble_fsm_task, EVT_0);
		statem_handle_event(&ble_fsm, &(struct event){BLE_EVT, (void *)"start"});
	}
	if(evt & EVT_1){
		tiny_clr_event(&ble_fsm_task, EVT_1);
		statem_handle_event(&ble_fsm, &(struct event){BLE_EVT, (void *)"stop"});
	}
	if(evt & EVT_2){
		tiny_clr_event(&ble_fsm_task, EVT_2);
		statem_handle_event(&ble_fsm, &(struct event){BLE_EVT, (void *)"hold"});
	}
	if(evt & EVT_3){
		tiny_clr_event(&ble_fsm_task, EVT_3);
		statem_handle_event(&ble_fsm, &(struct event){BLE_EVT, (void *)"change"});
	}

	uart_poll_dma_tx(DEV_UART4);
 }


static void ble_process_cb(void *msg)
{
	uint32_t *evt = (uint32_t *)msg;

	tiny_set_event(&ble_task, *evt);
}

static void ble_device_init(void)
{
	tiny_timer_create(&ble_task, ble_task_cb, NULL);
	tiny_timer_start(&ble_task, TIMER_FOREVER, 100);
	tiny_task_create(&ble_fsm_task, ble_fsm_task_cb, NULL);

	Topic_Subscrib(BLE_TOPIC, ble_process_cb);

	statem_init(&ble_fsm, &ble_idle, &ble_pwr_off);
}
app_initcall(ble_device_init);



