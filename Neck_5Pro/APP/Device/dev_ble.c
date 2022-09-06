#include "dev_ble.h"
#include "drv_gpio.h"
#include "dev_uart.h"
#include "include.h"
#include "topic.h"
#include <easyflash.h>


#define BLE_NAME	"Breo_Neck 5 Pro_BLE"

uint8_t esp_handshake_dat[1] = {0x01};
char esp_set_ble_name_dat[20];
uint8_t esp_turn_on_ble[1] = {0x03};
uint8_t MCUID[12];
uint8_t handshake_fb_norecipe[4] = {0x01, 0x01, 0x14, 0x00};
uint8_t handshake_fb_recipe[4] = {0x01, 0x01, 0x14, 0x01};


neck_5pro_ble_t neck_5pro_ble_struct;
uint8_t ble_mac[6] = {0};

rt_timer_t ble_heart_sftimer = RT_NULL;

static bool app_data_process(uint8_t *raw_frame, uint8_t frame_size)
{
	rt_uint32_t time;
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();
	eMASS_MODE mass_mode_tmp;

	if(frame_size == 0)		return false;

	if(unpack_from_app(raw_frame, frame_size, &neck_5pro_ble_struct.breo_app_protocol_struct) == err_ok)
	{
		switch(neck_5pro_ble_struct.neck_5pro_esp_optseq.esp_opt_sta)
		{
			case ESP_BLE_DATA_PROCESS:
				switch(neck_5pro_ble_struct.breo_app_protocol_struct.cmd)
				{
					case 0x01:
						LOG_I("get ble cmd:0x01");
						if(neck_5pro_dev_struct->flash_para.is_save_recipe == 0x01)
							group_pack_to_app(0x01, handshake_fb_recipe, sizeof(handshake_fb_recipe), false);
						else
							group_pack_to_app(0x01, handshake_fb_norecipe, sizeof(handshake_fb_norecipe), false);
					break;
				
					case 0xAB:
						LOG_I("get ble cmd:0xAB");
						if(group_pack_to_app(0xAB, MCUID, sizeof(MCUID), false))		
						{
							if(ble_heart_sftimer != RT_NULL)		
							{
								time = 1000;//1s
								rt_timer_control(ble_heart_sftimer, RT_TIMER_CTRL_SET_TIME, &time);
								rt_timer_start(ble_heart_sftimer);
							}
						}
					break;

					case 0x20:
						//解析数据
						LOG_I("get ble cmd:0x20");
						if(neck_5pro_ble_struct.breo_app_protocol_struct.dat[1] != 0x00)
						{
							mass_mode_tmp = (eMASS_MODE)neck_5pro_ble_struct.breo_app_protocol_struct.dat[1];
							switch(mass_mode_tmp)
							{
								case SOFT_MODE:
									neck_5pro_dev_struct->ble_pack.mass_strength = SOFT_STRENGTH;
									neck_5pro_dev_struct->ble_pack.mass_dir = MASS_FORWARD;
									neck_5pro_dev_struct->ble_pack.heat_level = WARM_HEAT;
									if(neck_5pro_dev_struct->ble_pack.is_manual == SET)
										neck_5pro_dev_struct->ble_pack.mass_place = PLACE_0;
									else
										neck_5pro_dev_struct->ble_pack.mass_place = PLACE_7;
	
									//按摩时间
									if((((neck_5pro_ble_struct.breo_app_protocol_struct.dat[7]<<8)|neck_5pro_ble_struct.breo_app_protocol_struct.dat[8])%60 == 0) &&
									(((neck_5pro_ble_struct.breo_app_protocol_struct.dat[7]<<8)|neck_5pro_ble_struct.breo_app_protocol_struct.dat[8]) <= 600) &&
									(((neck_5pro_ble_struct.breo_app_protocol_struct.dat[7]<<8)|neck_5pro_ble_struct.breo_app_protocol_struct.dat[8]) > 60)
									)
									{
										neck_5pro_dev_struct->ble_pack.mass_time = (neck_5pro_ble_struct.breo_app_protocol_struct.dat[7]<<8)|neck_5pro_ble_struct.breo_app_protocol_struct.dat[8];
										neck_5pro_dev_struct->dev_global.mass_cnt = 0;
										neck_5pro_dev_struct->ble_pack.remain_time = neck_5pro_dev_struct->ble_pack.mass_time;
									}
								break;

								case MID_MODE:
								case HARD_MODE:
									neck_5pro_dev_struct->ble_pack.mass_strength = SOFT_STRENGTH;
									neck_5pro_dev_struct->ble_pack.mass_dir = MASS_FORWARD;
									neck_5pro_dev_struct->ble_pack.heat_level = WARM_HEAT;
									if(neck_5pro_dev_struct->ble_pack.is_manual == SET)
										neck_5pro_dev_struct->ble_pack.mass_place = PLACE_0;
									else
										neck_5pro_dev_struct->ble_pack.mass_place = PLACE_5;
										
									//按摩时间
									if((((neck_5pro_ble_struct.breo_app_protocol_struct.dat[7]<<8)|neck_5pro_ble_struct.breo_app_protocol_struct.dat[8])%60 == 0) &&
									(((neck_5pro_ble_struct.breo_app_protocol_struct.dat[7]<<8)|neck_5pro_ble_struct.breo_app_protocol_struct.dat[8]) <= 600) &&
									(((neck_5pro_ble_struct.breo_app_protocol_struct.dat[7]<<8)|neck_5pro_ble_struct.breo_app_protocol_struct.dat[8]) > 60)
									)
									{
										neck_5pro_dev_struct->ble_pack.mass_time = (neck_5pro_ble_struct.breo_app_protocol_struct.dat[7]<<8)|neck_5pro_ble_struct.breo_app_protocol_struct.dat[8];
										neck_5pro_dev_struct->dev_global.mass_cnt = 0;
										neck_5pro_dev_struct->ble_pack.remain_time = neck_5pro_dev_struct->ble_pack.mass_time;
									}
								break;

								case USER_MODE:
									//按摩力度
									if(neck_5pro_ble_struct.breo_app_protocol_struct.dat[2] != 0xFF)
										neck_5pro_dev_struct->ble_pack.mass_strength = (eMASS_STRENGTH)neck_5pro_ble_struct.breo_app_protocol_struct.dat[2];
									else
									{
										//仅切换模式时，特殊处理
										if(neck_5pro_dev_struct->ble_pack.mass_mode != mass_mode_tmp)
											neck_5pro_dev_struct->ble_pack.mass_strength = neck_5pro_dev_struct->flash_para.strength_recipe;
									}
									neck_5pro_dev_struct->flash_para.strength_recipe = neck_5pro_dev_struct->ble_pack.mass_strength;
									//热敷等级
									if(neck_5pro_ble_struct.breo_app_protocol_struct.dat[3] != 0xFF)
									{
										if(neck_5pro_ble_struct.breo_app_protocol_struct.dat[3] == 0x00)
											neck_5pro_dev_struct->ble_pack.heat_level = NONE_HEAT;
										else if(neck_5pro_ble_struct.breo_app_protocol_struct.dat[3] == 0x2B)
											neck_5pro_dev_struct->ble_pack.heat_level = HOT_HEAT;
										else
											neck_5pro_dev_struct->ble_pack.heat_level = WARM_HEAT;
									}
									else
									{
										//仅切换模式时，特殊处理
										if(neck_5pro_dev_struct->ble_pack.mass_mode != mass_mode_tmp)
											neck_5pro_dev_struct->ble_pack.heat_level = neck_5pro_dev_struct->flash_para.heat_recipe;
									}
									neck_5pro_dev_struct->flash_para.heat_recipe = neck_5pro_dev_struct->ble_pack.heat_level;
									//按摩方向
									if(neck_5pro_ble_struct.breo_app_protocol_struct.dat[5] != 0xFF)
									{
										if(neck_5pro_ble_struct.breo_app_protocol_struct.dat[5] == 0x01)
											neck_5pro_dev_struct->ble_pack.mass_dir = MASS_FORWARD;
										else if(neck_5pro_ble_struct.breo_app_protocol_struct.dat[5] == 0x02)
											neck_5pro_dev_struct->ble_pack.mass_dir = MASS_REVERSE;
									}
									else
									{
										//仅切换模式时，特殊处理
										if(neck_5pro_dev_struct->ble_pack.mass_mode != mass_mode_tmp)
											neck_5pro_dev_struct->ble_pack.mass_dir = neck_5pro_dev_struct->flash_para.dir_recipe;
									}
									neck_5pro_dev_struct->flash_para.dir_recipe = neck_5pro_dev_struct->ble_pack.mass_dir;
									//是否定点
									if(neck_5pro_ble_struct.breo_app_protocol_struct.dat[4] != 0xFF)
									{
										if(neck_5pro_dev_struct->ble_pack.is_manual == RESET)
										{
											if(neck_5pro_ble_struct.breo_app_protocol_struct.dat[4] == 0x00)
											{
												neck_5pro_dev_struct->ble_pack.is_manual = SET;
												neck_5pro_dev_struct->flash_para.place_recipe = neck_5pro_dev_struct->ble_pack.mass_place;
												neck_5pro_dev_struct->ble_pack.mass_place = PLACE_0;
												Topic_Pushlish(DEV_MANUAL_SW, neck_5pro_dev_struct);
											}
											else		//APP组包缺陷
											{
												//按摩位置
												if(neck_5pro_ble_struct.breo_app_protocol_struct.dat[6] != 0xFF)
												{
													uint8_t X = 0;
													eMASS_PLACE place_tmp = PLACE_0;
													X = neck_5pro_ble_struct.breo_app_protocol_struct.dat[6];
													X = (X&~0x05)|((X&0x01)<<2)|((X&0x04)>>2);
													place_tmp = (eMASS_PLACE)X;
													if(neck_5pro_dev_struct->ble_pack.mass_place != place_tmp)//只有位置处方更改时重置状态机
													{
														neck_5pro_dev_struct->ble_pack.mass_place = (eMASS_PLACE)X;
														neck_5pro_dev_struct->flash_para.place_recipe = neck_5pro_dev_struct->ble_pack.mass_place;
														Topic_Pushlish(DEV_MANUAL_SW, neck_5pro_dev_struct);
													}
													else
													{
														neck_5pro_dev_struct->ble_pack.mass_place = (eMASS_PLACE)X;
														neck_5pro_dev_struct->flash_para.place_recipe = neck_5pro_dev_struct->ble_pack.mass_place;	
													}
												}
											}
										}
										else
										{
											if(neck_5pro_ble_struct.breo_app_protocol_struct.dat[4] == 0x01)
											{
												neck_5pro_dev_struct->ble_pack.is_manual = RESET;
												neck_5pro_dev_struct->ble_pack.mass_place = neck_5pro_dev_struct->flash_para.place_recipe;
												Topic_Pushlish(DEV_MANUAL_SW, neck_5pro_dev_struct);
											}
											else		//APP组包缺陷
											{
												//按摩位置
												neck_5pro_dev_struct->ble_pack.mass_place = PLACE_0;
												if(neck_5pro_dev_struct->ble_pack.mass_mode != mass_mode_tmp)
												{
													neck_5pro_dev_struct->ble_pack.is_manual = RESET;
													neck_5pro_dev_struct->flash_para.place_recipe = neck_5pro_dev_struct->ble_pack.mass_place;
												}
											}
										}
									}	
									else	
									{
										//定点关闭时切换模式，特殊处理
										if(neck_5pro_dev_struct->ble_pack.is_manual == RESET)
											neck_5pro_dev_struct->ble_pack.mass_place = neck_5pro_dev_struct->flash_para.place_recipe;
										else	//定点打开时切换模式，特殊处理
										{
											if(neck_5pro_dev_struct->ble_pack.mass_mode != mass_mode_tmp)
											{
												neck_5pro_dev_struct->ble_pack.is_manual = RESET;
												neck_5pro_dev_struct->ble_pack.mass_place = neck_5pro_dev_struct->flash_para.place_recipe;
											}
										}
									}
									//按摩时间
									if((((neck_5pro_ble_struct.breo_app_protocol_struct.dat[7]<<8)|neck_5pro_ble_struct.breo_app_protocol_struct.dat[8])%60 == 0) &&
									(((neck_5pro_ble_struct.breo_app_protocol_struct.dat[7]<<8)|neck_5pro_ble_struct.breo_app_protocol_struct.dat[8]) <= 600) && 
									(((neck_5pro_ble_struct.breo_app_protocol_struct.dat[7]<<8)|neck_5pro_ble_struct.breo_app_protocol_struct.dat[8]) > 60)
									)
									{
										neck_5pro_dev_struct->ble_pack.mass_time = (neck_5pro_ble_struct.breo_app_protocol_struct.dat[7]<<8)|neck_5pro_ble_struct.breo_app_protocol_struct.dat[8];
										neck_5pro_dev_struct->dev_global.mass_cnt = 0;
										neck_5pro_dev_struct->ble_pack.remain_time = neck_5pro_dev_struct->ble_pack.mass_time;
									}
								break;
							}
							//发布消息，重置心跳包定时器
							if(neck_5pro_dev_struct->ble_pack.mass_mode != mass_mode_tmp)
							{
								neck_5pro_dev_struct->ble_pack.mass_mode = mass_mode_tmp;
								if(neck_5pro_dev_struct->ble_pack.mass_mode == USER_MODE)
								{
									//存储用户处方
									neck_5pro_dev_struct->flash_para.is_save_recipe = 0x01;
									if(ef_set_env_blob("user_mode_recipe", &neck_5pro_dev_struct->flash_para, sizeof(flash_para_t)) == EF_NO_ERR)
										LOG_I("save recipe successed.");
								}
								else if(neck_5pro_dev_struct->ble_pack.mass_mode == SOFT_MODE)
								{
									neck_5pro_dev_struct->ble_pack.is_manual = RESET;
									neck_5pro_dev_struct->ble_pack.mass_place = PLACE_7;
								}
								else
								{
									neck_5pro_dev_struct->ble_pack.is_manual = RESET;
									neck_5pro_dev_struct->ble_pack.mass_place = PLACE_5;
								}
								Topic_Pushlish(DEV_MANUAL_SW, neck_5pro_dev_struct);
								Topic_Pushlish(DEV_MODE_CHANGE, neck_5pro_dev_struct);
							}
							else
							{
								Topic_Pushlish(BUZZER_TONE, neck_5pro_dev_struct);
							}
							Topic_Pushlish(DEV_HEAT_SW, neck_5pro_dev_struct);
							Topic_Pushlish(UPDATE_HEART_PACK, neck_5pro_dev_struct);
						}
					break;

					default:
						LOG_W("error ble cmd:%d", neck_5pro_ble_struct.breo_app_protocol_struct.cmd);
					break;
				}
			break;
		}
	}

	return false;
}

static bool esp_data_process(uint8_t *raw_frame, uint8_t frame_size)
{
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();

	if(frame_size == 0)		return false;

	if(unpack_from_esp(raw_frame, frame_size, &neck_5pro_ble_struct.breo_esp_protocol_struct) == esp_err_ok)
	{
		switch(neck_5pro_ble_struct.neck_5pro_esp_optseq.esp_opt_sta)
		{
			case ESP_HANDSHAKE:
				if(neck_5pro_ble_struct.breo_esp_protocol_struct.cmd == 0x2003)	//收到握手指令
				{
					return group_pack_to_esp(0x07, 0x01, 0xC0, 0x2003, esp_handshake_dat, sizeof(esp_handshake_dat));
				}
			break;

			case ESP_SETTINGS:
				switch(neck_5pro_ble_struct.neck_5pro_esp_optseq.esp_settings_sta)
				{
					case ESP_SET_BLE_NAME:
						if(neck_5pro_ble_struct.breo_esp_protocol_struct.cmd == 0x0006 &&		//成功执行设置蓝牙名称
							neck_5pro_ble_struct.breo_esp_protocol_struct.dat_len == 0x01 &&
							neck_5pro_ble_struct.breo_esp_protocol_struct.dat[0] == 0x01
							)	
						{
							return true;
						}
						else
						{
							LOG_E("ESP32 set ble name failed.");
						}
					break;

					case ESP_GET_BLE_MAC:
						if(neck_5pro_ble_struct.breo_esp_protocol_struct.cmd == 0x1004 &&		//成功执行获取蓝牙MAC
							neck_5pro_ble_struct.breo_esp_protocol_struct.dat_len == 0x07 &&
							neck_5pro_ble_struct.breo_esp_protocol_struct.dat[0] == 0x01
							)	
						{
							rt_memcpy(ble_mac, &neck_5pro_ble_struct.breo_esp_protocol_struct.dat[1], 6);
							return true;
						}
						else
						{
							LOG_E("ESP32 get ble mac failed.");
						}
					break;

					case ESP_SET_ADV:
						if(neck_5pro_ble_struct.breo_esp_protocol_struct.cmd == 0x0005 &&		//成功执行设置蓝牙广播
							neck_5pro_ble_struct.breo_esp_protocol_struct.dat_len == 0x01 &&
							neck_5pro_ble_struct.breo_esp_protocol_struct.dat[0] == 0x01
							)	
						{
							return true;
						}
						else
						{
							LOG_E("ESP32 set ble adv failed.");
						}
					break;
				}
			break;

			case ESP_BLE_TURN_ON:
				if(neck_5pro_ble_struct.breo_esp_protocol_struct.cmd == 0x0003 &&		//成功执行启动蓝牙
					neck_5pro_ble_struct.breo_esp_protocol_struct.dat_len == 0x01 &&
					neck_5pro_ble_struct.breo_esp_protocol_struct.dat[0] == 0x01
					)	
				{
					return true;
				}
				else
				{
					LOG_E("ESP32 turn on ble failed.");
				}
			break;

			case ESP_BLE_DATA_PROCESS:
				if(neck_5pro_ble_struct.breo_esp_protocol_struct.cmd == 0x2001)		//蓝牙链接状态通知
				{
					if((neck_5pro_ble_struct.breo_esp_protocol_struct.dat[0] & 0x04) == 0x00)	//BLE链接断开
						LOG_I("BLE disconnect.");
					else 
					{
						neck_5pro_dev_struct->ble_con_pre_sta = neck_5pro_dev_struct->dev_status;
						neck_5pro_dev_struct->dev_status = DEV_BLE_CONNECTED;
						Topic_Pushlish(DEV_STA_UPDATE, neck_5pro_dev_struct);
						LOG_I("BLE connected.");
					}
					return true;
				}
				else if(neck_5pro_ble_struct.breo_esp_protocol_struct.cmd == 0x2003)	//收到握手指令
				{
					return group_pack_to_esp(0x07, 0x01, 0xC0, 0x2003, esp_handshake_dat, sizeof(esp_handshake_dat));
				}
			break;

			default:
			break;
		}
	}

	return false;
}

static void BLEthread_entry(void *para)
{
	uint8_t esp_pwr_up_cnt = 0;
	uint8_t esp_resend_cnt = 0;

	GetUID(MCUID);
	while(1)
	{
		uint8_t buff[100];
		uint8_t size = 0;
	
		size = uart_read(DEV_UART4, buff, sizeof(buff));

		switch(neck_5pro_ble_struct.neck_5pro_esp_optseq.esp_opt_sta)
		{
			case ESP_PWR_UP:
				ESP32_PWR_ON;
				if(++esp_pwr_up_cnt >= 100)
				{
					neck_5pro_ble_struct.neck_5pro_esp_action = ESP_ACTION_RECV;
					neck_5pro_ble_struct.neck_5pro_esp_optseq.esp_opt_sta = ESP_HANDSHAKE;
				}
			break;

			case ESP_HANDSHAKE:
				if(neck_5pro_ble_struct.neck_5pro_esp_action == ESP_ACTION_RECV)
				{
					if(esp_data_process(buff, size))
					{
						neck_5pro_ble_struct.neck_5pro_esp_action = ESP_ACTION_SEND;
						neck_5pro_ble_struct.neck_5pro_esp_optseq.esp_opt_sta = ESP_SETTINGS;
					}
				}
			break;

			case ESP_SETTINGS:
				switch(neck_5pro_ble_struct.neck_5pro_esp_optseq.esp_settings_sta)
				{
					case ESP_SET_BLE_NAME:
						if(neck_5pro_ble_struct.neck_5pro_esp_action == ESP_ACTION_SEND)
						{
							esp_set_ble_name_dat[0] = 0x01;	//设置BLE名称
							rt_sprintf(&esp_set_ble_name_dat[1], BLE_NAME);
							group_pack_to_esp(0x12, 0x01, 0xC0, 0x0006, (uint8_t *)esp_set_ble_name_dat, rt_strlen(esp_set_ble_name_dat));
							neck_5pro_ble_struct.neck_5pro_esp_action = ESP_ACTION_RECV;
							esp_resend_cnt = 0;
						}
						else
						{
							if(esp_data_process(buff, size))
							{
								neck_5pro_ble_struct.neck_5pro_esp_action = ESP_ACTION_SEND;
								neck_5pro_ble_struct.neck_5pro_esp_optseq.esp_settings_sta = ESP_GET_BLE_MAC;
							}

							if(++esp_resend_cnt >= 100){		//超时重发
								esp_resend_cnt = 0;
								neck_5pro_ble_struct.neck_5pro_esp_action = ESP_ACTION_SEND;
								neck_5pro_ble_struct.neck_5pro_esp_optseq.esp_settings_sta = ESP_SET_BLE_NAME;
							}
						}
					break;

					case ESP_GET_BLE_MAC:
						if(neck_5pro_ble_struct.neck_5pro_esp_action == ESP_ACTION_SEND)
						{
							group_pack_to_esp(0x04, 0x01, 0xC0, 0x1004, NULL, 0);
							neck_5pro_ble_struct.neck_5pro_esp_action = ESP_ACTION_RECV;
							esp_resend_cnt = 0;
						}
						else
						{
							if(esp_data_process(buff, size))
							{
								neck_5pro_ble_struct.neck_5pro_esp_action = ESP_ACTION_SEND;
								neck_5pro_ble_struct.neck_5pro_esp_optseq.esp_settings_sta = ESP_SET_ADV;
							}

							if(++esp_resend_cnt >= 100){		//超时重发
								esp_resend_cnt = 0;
								neck_5pro_ble_struct.neck_5pro_esp_action = ESP_ACTION_SEND;
								neck_5pro_ble_struct.neck_5pro_esp_optseq.esp_settings_sta = ESP_GET_BLE_MAC;
							}
						}
					break;

					case ESP_SET_ADV:
						if(neck_5pro_ble_struct.neck_5pro_esp_action == ESP_ACTION_SEND)
						{
							uint8_t send_buff[9];
							send_buff[0] = 0x81;
							send_buff[1] = 0x01;
							send_buff[2] = 0x05;
							rt_memcpy(&send_buff[3], ble_mac, 6);
						
							group_pack_to_esp(0x12, 0x01, 0xC0, 0x0005, send_buff, 9);
							neck_5pro_ble_struct.neck_5pro_esp_action = ESP_ACTION_RECV;
							esp_resend_cnt = 0;
						}
						else
						{
							if(esp_data_process(buff, size))
							{
								neck_5pro_ble_struct.neck_5pro_esp_action = ESP_ACTION_SEND;
								neck_5pro_ble_struct.neck_5pro_esp_optseq.esp_opt_sta = ESP_BLE_TURN_ON;
							}

							if(++esp_resend_cnt >= 100){		//超时重发
								esp_resend_cnt = 0;
								neck_5pro_ble_struct.neck_5pro_esp_action = ESP_ACTION_SEND;
								neck_5pro_ble_struct.neck_5pro_esp_optseq.esp_settings_sta = ESP_SET_ADV;
							}
						}
					break;
				}
			break;

			case ESP_BLE_TURN_ON:
				if(neck_5pro_ble_struct.neck_5pro_esp_action == ESP_ACTION_SEND)
				{
					group_pack_to_esp(0x12, 0x01, 0xC0, 0x0003, esp_turn_on_ble, sizeof(esp_turn_on_ble));
					neck_5pro_ble_struct.neck_5pro_esp_action = ESP_ACTION_RECV;
					esp_resend_cnt = 0;
				}
				else
				{
					if(esp_data_process(buff, size))
					{
						neck_5pro_ble_struct.neck_5pro_esp_optseq.esp_opt_sta = ESP_BLE_DATA_PROCESS;
					}

					if(++esp_resend_cnt >= 100){		//超时重发
						esp_resend_cnt = 0;
						neck_5pro_ble_struct.neck_5pro_esp_action = ESP_ACTION_SEND;
						neck_5pro_ble_struct.neck_5pro_esp_optseq.esp_opt_sta = ESP_BLE_TURN_ON;
					}
				}
			break;

			case ESP_BLE_DATA_PROCESS:
				esp_data_process(buff, size);//启动后，仅监听ble链接状态
				app_data_process(buff, size);//启动后，监听app透传数据

//				uart_write(DEV_UART1, buff, size);
			break;
		}

		uart_poll_dma_tx(DEV_UART4);
		rt_thread_mdelay(10);
	}
}

static void ble_dev_no_work_cb(void *msg)
{
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();
	
	ESP32_PWR_OFF;
	if(rt_thread_find("ble") != RT_NULL)
		rt_thread_delete(rt_thread_find("ble"));
//	//存储用户处方，app连接过一次设备后才生效
//	if(neck_5pro_dev_struct->flash_para.is_save_recipe == 0x01)
//	{
//		if(ef_set_env_blob("user_mode_recipe", &neck_5pro_dev_struct->flash_para, sizeof(flash_para_t)) == EF_NO_ERR)
//		{
//			LOG_I("save recipe successed.");
//		}
//	}
}

static void ble_dev_work_cb(void *msg)
{
	ble_create_thread();
}

static void ble_update_heart_cb(void *msg)
{
	rt_uint32_t time;
	uint8_t X = 0;
	uint8_t buf[12] = {0};
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();
	
	//模式状态
	if(neck_5pro_dev_struct->dev_global.is_dev_work == SET)		buf[0] = 0x01;
	else														buf[0] = 0x00;
	//按摩模式
	buf[1] = neck_5pro_dev_struct->ble_pack.mass_mode;
	//按摩时间
	buf[2] = (uint8_t)(neck_5pro_dev_struct->ble_pack.remain_time>>8);
	buf[3] = (uint8_t)neck_5pro_dev_struct->ble_pack.remain_time;
	//按压力度
	buf[4] = (uint8_t)neck_5pro_dev_struct->ble_pack.mass_strength;
	//热敷温度
	if(neck_5pro_dev_struct->ble_pack.heat_level == NONE_HEAT)		//关闭热敷
		buf[5] = 0x00;
	else if(neck_5pro_dev_struct->ble_pack.heat_level == WARM_HEAT)
		buf[5] = 0x28;
	else if(neck_5pro_dev_struct->ble_pack.heat_level == HOT_HEAT)
		buf[5] = 0x2B;
	else
		buf[5] = 0x00;
	//实时温度
	buf[6] = neck_5pro_dev_struct->ble_pack.curr_temp;
	//电池电量
	buf[7] = neck_5pro_dev_struct->ble_pack.batt_val;
	//是否定点
	if(neck_5pro_dev_struct->ble_pack.is_manual == SET)
		buf[8] = 0x00;
	else
		buf[8] = 0x01;
	//电机转向
	if(neck_5pro_dev_struct->ble_pack.mass_dir == MASS_FORWARD)
		buf[9] = 0x01;
	else
		buf[9] = 0x02;
	//按摩位置
	X = (uint8_t)neck_5pro_dev_struct->ble_pack.mass_place;
	X = (X&~0x05)|((X&0x01)<<2)|((X&0x04)>>2);
	buf[10] = X;
	//当前位置
	buf[11] = (uint8_t)neck_5pro_dev_struct->ble_pack.curr_place;

	if(ble_heart_sftimer != RT_NULL)		
	{
		time = 5000;//5s
		rt_timer_control(ble_heart_sftimer, RT_TIMER_CTRL_SET_TIME, &time);
		rt_timer_start(ble_heart_sftimer);
	}

	group_pack_to_app(0x69, buf, sizeof(buf), false);
}

static void ble_about_topic_subscrib(void)
{
	Topic_Subscrib(DEV_WORK, ble_dev_work_cb);
	Topic_Subscrib(DEV_NO_WORK, ble_dev_no_work_cb);
	Topic_Subscrib(UPDATE_HEART_PACK, ble_update_heart_cb);
}
user_initcall(ble_about_topic_subscrib);

static void ble_heart_sftimer_cb(void *para)
{
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();
	
	Topic_Pushlish(UPDATE_HEART_PACK, neck_5pro_dev_struct);
}

void ble_sftimer_init(void)
{
	ble_heart_sftimer = rt_timer_create("ble_heart_timer", ble_heart_sftimer_cb, RT_NULL, 5000, RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_ONE_SHOT);
}

void ble_create_thread(void)
{
	rt_thread_t rt_BLEthread_id = RT_NULL;
	
	rt_BLEthread_id = rt_thread_create("ble",
								BLEthread_entry,
								RT_NULL,
								1024,		//stack size
								16,			//prioity
								10);		//timeslice
	if(rt_BLEthread_id != RT_NULL)
		rt_thread_startup(rt_BLEthread_id);
}


