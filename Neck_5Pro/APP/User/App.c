#include "n32g45x.h"
#include "rtthread.h"
#include <fal.h>
#include <easyflash.h>
#include "include.h"
#include "topic.h"
#include "init.h"

#include "drv_key.h"
#include "drv_gpio.h"
#include "dev_uart.h"
#include "dev_adc.h"
#include "dev_mp.h"
#include "dev_buzzer.h"
#include "dev_led.h"
#include "dev_heat.h"
#include "dev_motor.h"
#include "dev_ble.h"

static neck_5pro_t neck_5pro_dev = {
	.dev_status = NONE_STA,
	.ble_con_pre_sta = NONE_STA,

	.ble_pack.batt_val = 5,
	.ble_pack.mass_mode = SOFT_MODE,
	.ble_pack.mass_strength = SOFT_STRENGTH,
	.ble_pack.mass_dir = MASS_FORWARD,
	.ble_pack.heat_level = WARM_HEAT,
	.ble_pack.curr_temp = 40,
	.ble_pack.mass_time = 600,
	.ble_pack.remain_time = 0,
	.ble_pack.mass_place = PLACE_7,
	.ble_pack.curr_place = MOVING,
	.ble_pack.is_manual = RESET,
	
	.dev_global.is_dev_on = RESET,
	.dev_global.is_dev_work = RESET,
	.dev_global.is_aging_done = RESET,
	.dev_global.mass_cnt = 0,
	.dev_global.tra_dir = TRA_FORWARD,
	.dev_global.tra_speed = MID_SPEED,
	.dev_global.manual_tra_start = RESET,

	.low_pwr_flag = 0,
};

void get_dev_struct(void)
{
	LOG_I("neck_5pro_dev.dev_status:%d", neck_5pro_dev.dev_status);
	LOG_I("neck_5pro_dev.pre_sta:%d", neck_5pro_dev.ble_con_pre_sta);
	LOG_I(" ");
	LOG_I("neck_5pro_dev.batt_val:%d", neck_5pro_dev.ble_pack.batt_val);
	LOG_I("neck_5pro_dev.mass_strength:%d", neck_5pro_dev.ble_pack.mass_strength);
	LOG_I("neck_5pro_dev.mass_dir:%d", neck_5pro_dev.ble_pack.mass_dir);
	LOG_I("neck_5pro_dev.heat_level:%d", neck_5pro_dev.ble_pack.heat_level);
	LOG_I("neck_5pro_dev.mass_time:%d", neck_5pro_dev.ble_pack.mass_time);
	LOG_I("neck_5pro_dev.mass_remain:%d", neck_5pro_dev.ble_pack.remain_time);
	LOG_I("neck_5pro_dev.mass_place:%d", neck_5pro_dev.ble_pack.mass_place);
	LOG_I("neck_5pro_dev.curr_place:%d", neck_5pro_dev.ble_pack.curr_place);
	LOG_I("neck_5pro_dev.is_manual:%d", neck_5pro_dev.ble_pack.is_manual);
	LOG_I(" ");
	LOG_I("neck_5pro_dev.dev_global.is_dev_on:%d", neck_5pro_dev.dev_global.is_dev_on);
	LOG_I("neck_5pro_dev.dev_global.is_dev_work:%d", neck_5pro_dev.dev_global.is_dev_work);
	LOG_I("neck_5pro_dev.dev_global.tra_dir:%d", neck_5pro_dev.dev_global.tra_dir);
	LOG_I("neck_5pro_dev.dev_global.tra_speed:%d", neck_5pro_dev.dev_global.tra_speed);
	LOG_I(" ");
	LOG_I("neck_5pro_dev.low_pwr_flag:%d", neck_5pro_dev.low_pwr_flag);
	LOG_I(" ");
	LOG_I("neck_5pro_dev.flash_para.is_save_recipe:%d", neck_5pro_dev.flash_para.is_save_recipe);
	LOG_I("neck_5pro_dev.flash_para.strength_recipe:%d", neck_5pro_dev.flash_para.strength_recipe);
	LOG_I("neck_5pro_dev.flash_para.dir_recipe:%d", neck_5pro_dev.flash_para.dir_recipe);
	LOG_I("neck_5pro_dev.flash_para.place_recipe:%d", neck_5pro_dev.flash_para.place_recipe);
}
MSH_CMD_EXPORT(get_dev_struct, get dev struct);

static void IWDG_Configure(void)
{
	IWDG_WriteConfig(IWDG_WRITE_ENABLE);      			/* д��0x5555,�����������Ĵ���д�빦�� */
    IWDG_SetPrescalerDiv(IWDG_PRESCALER_DIV256);      	/* ����ʱ�ӷ�Ƶ,40K/256=156HZ(6.4ms)*/
    IWDG_CntReload(156);        						/* ι��ʱ�� 1s/6.4MS=313 .ע�ⲻ�ܴ���0xfff*/
    IWDG_ReloadKey();
    IWDG_Enable();
}

neck_5pro_t *get_neck_5pro_struct(void)
{
	neck_5pro_t *p_neck_5pro_struct = &neck_5pro_dev;

	return p_neck_5pro_struct;
}

static void topic_init(void)
{
	Topic_Init(BUZZER_YOWL, NULL, NULL);
	Topic_Init(BUZZER_TONE, NULL, NULL);
	Topic_Init(DEV_MODE_CHANGE, NULL, NULL);
	Topic_Init(DEV_WORK, NULL, NULL);
	Topic_Init(DEV_NO_WORK, NULL, NULL);
	Topic_Init(DEV_STA_UPDATE, NULL, NULL);
	Topic_Init(DEV_HEAT_SW, NULL, NULL);
	Topic_Init(DEV_MANUAL_SW, NULL, NULL);
	Topic_Init(UPDATE_HEART_PACK, NULL, NULL);
}
user_initcall(topic_init);

static void dev_sftimer_init(void)
{
	adc_filter_sftimer_init();
	buzzer_sftimer_init();
	led_sftimer_init();
	heat_sftimer_init();
	motor_sftimer_init();
	ble_sftimer_init();
}

/*
*@brief main���������ڴ������߳�
*@note 	���ȼ�ԽС���߼����ȼ�Խ�ߡ�
*/
int main(void)
{
	size_t save_val_len;
	static uint16_t aging_timeout = 0;
	static uint8_t aging_key_cnt = 0;

	if(fal_init() > 0)
		fal_show_part_table();
	if(EF_NO_ERR == easyflash_init())
	{
		ef_print_env();
		ef_read_all_log_from_flash();
		//����flash�ڴ洢������
		ef_get_env_blob("user_mode_recipe", &neck_5pro_dev.flash_para, sizeof(flash_para_t), &save_val_len);
		/* ȡ����ѹ���书�� */
//		ef_get_env_blob("low_pwr_flag", &neck_5pro_dev.low_pwr_flag, sizeof(uint8_t), &save_val_len);
//		if(neck_5pro_dev.low_pwr_flag == 0)//�ϴ������������ػ�
		neck_5pro_dev.ble_pack.batt_val = 5;
//		else if(neck_5pro_dev.low_pwr_flag == 0x02)//�ϴ��ǵ�ѹ�ػ�
//			neck_5pro_dev.ble_pack.batt_val = 2;
//		else if(neck_5pro_dev.low_pwr_flag == 0x01)//�ϴ��ǵ�ѹ�����ػ�
//			neck_5pro_dev.ble_pack.batt_val = 1;
	}

	IWDG_Configure();
	dev_sftimer_init();
	if(USB_CHK_INPUT == RESET)		
	{
		neck_5pro_dev.dev_global.is_dev_on = SET;
		PWR_ON;
	}
	while(1)
	{
		uint8_t keycode = KEY_NONE;
		adc_filter_t *adc_filter_struct = get_adc_filter_struct();
		
		keycode = bsp_GetKey();
		if(keycode != KEY_NONE)
		{
			switch(keycode)
			{
				case KEY_1_LONG:		//���ػ�
					if(USB_CHK_INPUT == RESET)		break;
					if(neck_5pro_dev.dev_global.is_dev_on == RESET)
					{
						neck_5pro_dev.dev_global.is_dev_on = SET;
						neck_5pro_dev.dev_global.is_dev_work = SET;
						PWR_ON;
						Topic_Pushlish(DEV_WORK, &neck_5pro_dev);
					}
					else
					{
						neck_5pro_dev.dev_global.is_dev_on = RESET;
						neck_5pro_dev.dev_global.is_dev_work = RESET;
						Topic_Pushlish(DEV_NO_WORK, &neck_5pro_dev);
					}
					Topic_Pushlish(BUZZER_YOWL, &neck_5pro_dev);
				break;

				case KEY_1_UP:			//ģʽ�л�
					if(USB_CHK_INPUT == RESET)		break;
					if(neck_5pro_dev.dev_status == DEV_AGING)		break;
					neck_5pro_dev.ble_pack.mass_mode++;
					if(neck_5pro_dev.ble_pack.mass_mode > USER_MODE)
					{
						neck_5pro_dev.ble_pack.mass_mode = SOFT_MODE;
						neck_5pro_dev.ble_pack.mass_strength = SOFT_STRENGTH;
						neck_5pro_dev.ble_pack.mass_dir = MASS_FORWARD;
						neck_5pro_dev.ble_pack.heat_level = WARM_HEAT;
//						if(neck_5pro_dev.ble_pack.is_manual == RESET)
//							neck_5pro_dev.ble_pack.mass_place = PLACE_7;
//						else
//							neck_5pro_dev.ble_pack.mass_place = PLACE_0;
						neck_5pro_dev.ble_pack.is_manual = RESET;
						neck_5pro_dev.ble_pack.mass_place = PLACE_7;
					}
					else
					{
						switch(neck_5pro_dev.ble_pack.mass_mode)
						{
							case SOFT_MODE:
								neck_5pro_dev.ble_pack.mass_strength = SOFT_STRENGTH;
								neck_5pro_dev.ble_pack.mass_dir = MASS_FORWARD;
								neck_5pro_dev.ble_pack.heat_level = WARM_HEAT;
//								if(neck_5pro_dev.ble_pack.is_manual == RESET)
//									neck_5pro_dev.ble_pack.mass_place = PLACE_7;
//								else
//									neck_5pro_dev.ble_pack.mass_place = PLACE_0;
								neck_5pro_dev.ble_pack.is_manual = RESET;
								neck_5pro_dev.ble_pack.mass_place = PLACE_7;
							break;

							case MID_MODE:
							case HARD_MODE:
								neck_5pro_dev.ble_pack.mass_strength = SOFT_STRENGTH;
								neck_5pro_dev.ble_pack.mass_dir = MASS_FORWARD;
								neck_5pro_dev.ble_pack.heat_level = WARM_HEAT;
//								if(neck_5pro_dev.ble_pack.is_manual == RESET)
//									neck_5pro_dev.ble_pack.mass_place = PLACE_5;
//								else
//									neck_5pro_dev.ble_pack.mass_place = PLACE_0;
								neck_5pro_dev.ble_pack.is_manual = RESET;
								neck_5pro_dev.ble_pack.mass_place = PLACE_5;
							break;

							case USER_MODE:
								if(neck_5pro_dev.flash_para.is_save_recipe == 0x01)
								{
									neck_5pro_dev.ble_pack.mass_dir = neck_5pro_dev.flash_para.dir_recipe;
									neck_5pro_dev.ble_pack.mass_strength = neck_5pro_dev.flash_para.strength_recipe;
									neck_5pro_dev.ble_pack.heat_level = neck_5pro_dev.flash_para.heat_recipe;
//									if(neck_5pro_dev.ble_pack.is_manual == RESET)
//										neck_5pro_dev.ble_pack.mass_place = neck_5pro_dev.flash_para.place_recipe;
//									else
//										neck_5pro_dev.ble_pack.mass_place = PLACE_0;
									neck_5pro_dev.ble_pack.is_manual = RESET;
									neck_5pro_dev.ble_pack.mass_place = neck_5pro_dev.flash_para.place_recipe;
								}
								else
								{
									neck_5pro_dev.ble_pack.mass_mode = SOFT_MODE;
									neck_5pro_dev.ble_pack.mass_strength = SOFT_STRENGTH;
									neck_5pro_dev.ble_pack.mass_dir = MASS_FORWARD;
									neck_5pro_dev.ble_pack.heat_level = WARM_HEAT;
//									if(neck_5pro_dev.ble_pack.is_manual == RESET)
//										neck_5pro_dev.ble_pack.mass_place = PLACE_7;
//									else
//										neck_5pro_dev.ble_pack.mass_place = PLACE_0;
									neck_5pro_dev.ble_pack.is_manual = RESET;
									neck_5pro_dev.ble_pack.mass_place = PLACE_7;
								}
							break;
						}
					}

					//�ж��Ƿ�����ϻ�ģʽ
					if(aging_timeout < 700)
					{
						if(++aging_key_cnt > 15)
						{
							aging_key_cnt = 15;
							neck_5pro_dev.dev_status = DEV_AGING;
							neck_5pro_dev.ble_pack.mass_mode = HARD_MODE;
							neck_5pro_dev.ble_pack.mass_strength = MID_STRENGTH;
							neck_5pro_dev.ble_pack.heat_level = WARM_HEAT;
							neck_5pro_dev.ble_pack.mass_place = PLACE_5;
							//�ر�����ģ�飬����ble�߳�
							if(rt_thread_find("ble") != RT_NULL)
								rt_thread_delete(rt_thread_find("ble"));
							ESP32_PWR_OFF;
							Topic_Pushlish(DEV_STA_UPDATE, &neck_5pro_dev);
						}
					}
					Topic_Pushlish(DEV_HEAT_SW, &neck_5pro_dev);
					Topic_Pushlish(DEV_MANUAL_SW, &neck_5pro_dev);
					Topic_Pushlish(DEV_MODE_CHANGE, &neck_5pro_dev);
					Topic_Pushlish(UPDATE_HEART_PACK, &neck_5pro_dev);
				break;

				case KEY_2_UP:			//�Ƿ񶨵�
					if(USB_CHK_INPUT == RESET)		break;
					if(neck_5pro_dev.dev_status == DEV_AGING)		break;
					if(neck_5pro_dev.ble_pack.is_manual == SET)
					{
						neck_5pro_dev.ble_pack.is_manual = RESET;
						//�ָ�λ����Ϣ
						switch(neck_5pro_dev.ble_pack.mass_mode)
						{
							case SOFT_MODE:
								neck_5pro_dev.ble_pack.mass_place = PLACE_7;
							break;

							case MID_MODE:
							case HARD_MODE:
								neck_5pro_dev.ble_pack.mass_place = PLACE_5;
							break;

							case USER_MODE:
								if(neck_5pro_dev.flash_para.is_save_recipe == 0x01)
									neck_5pro_dev.ble_pack.mass_place = neck_5pro_dev.flash_para.place_recipe;
							break;
						}
					}
					else
					{
						neck_5pro_dev.ble_pack.is_manual = SET;
						//����λ����Ϣ
						neck_5pro_dev.ble_pack.mass_place = PLACE_0;
					}
					Topic_Pushlish(BUZZER_TONE, &neck_5pro_dev);
					Topic_Pushlish(DEV_MANUAL_SW, &neck_5pro_dev);
					Topic_Pushlish(UPDATE_HEART_PACK, &neck_5pro_dev);
				break;

				case KEY_2_LONG:		//�ֶ��ƶ�
					if(USB_CHK_INPUT == RESET)		break;
					if(neck_5pro_dev.dev_status == DEV_AGING)		break;
					if(neck_5pro_dev.ble_pack.is_manual == RESET)	break;
					neck_5pro_dev.dev_global.manual_tra_start = SET;
					Topic_Pushlish(BUZZER_TONE, &neck_5pro_dev);
				break;

				case KEY_2_LONG_UP:		//�ֶ��ƶ�ֹͣ
					if(USB_CHK_INPUT == RESET)		break;
					if(neck_5pro_dev.dev_status == DEV_AGING)		break;
					if(neck_5pro_dev.ble_pack.is_manual == RESET)	break;
					neck_5pro_dev.dev_global.manual_tra_start = RESET;
					Topic_Pushlish(BUZZER_TONE, &neck_5pro_dev);
				break;

				case KEY_3_UP:			//�ȷ󿪹�
					if(USB_CHK_INPUT == RESET)		break;
					if(neck_5pro_dev.dev_status == DEV_AGING)		break;
					if(neck_5pro_dev.ble_pack.heat_level == NONE_HEAT)
						neck_5pro_dev.ble_pack.heat_level = WARM_HEAT;
					else
						neck_5pro_dev.ble_pack.heat_level = NONE_HEAT;

					if(neck_5pro_dev.ble_pack.mass_mode == USER_MODE)
						neck_5pro_dev.flash_para.heat_recipe = neck_5pro_dev.ble_pack.heat_level;
					
					Topic_Pushlish(DEV_HEAT_SW, &neck_5pro_dev);
					Topic_Pushlish(BUZZER_TONE, &neck_5pro_dev);
					Topic_Pushlish(UPDATE_HEART_PACK, &neck_5pro_dev);
				break;
			}
		}

		//�ϻ�״̬�£�������
		if(neck_5pro_dev.dev_status == DEV_AGING)
			mpdevice_charge_init();

		//���ϻ�״̬�������豸״̬
		if((USB_CHK_INPUT == RESET) && (neck_5pro_dev.dev_status != DEV_AGING))
		{
			neck_5pro_dev.ble_pack.batt_val = 5;
			//����־���flash
			ef_get_env_blob("low_pwr_flag", &neck_5pro_dev.low_pwr_flag, sizeof(uint8_t), &save_val_len);
			if(neck_5pro_dev.low_pwr_flag != 0)
			{
				neck_5pro_dev.low_pwr_flag = 0;
				LOG_W("set low_pwr_flag = 0");
				ef_set_env_blob("low_pwr_flag", &neck_5pro_dev.low_pwr_flag, sizeof(uint8_t));
			}
			if((adc_filter_struct->bat1temp_filter_adc > 2747) || (adc_filter_struct->bat2temp_filter_adc > 2747))
			{
				mpdevice_off_init();
				if(neck_5pro_dev.dev_status != DEV_CHARGE_PORT)
				{
					neck_5pro_dev.dev_status = DEV_CHARGE_PORT;
					Topic_Pushlish(DEV_STA_UPDATE, &neck_5pro_dev);
				}
			}
			else if((adc_filter_struct->bat1temp_filter_adc < 2684) || (adc_filter_struct->bat2temp_filter_adc < 2684))
			{
				mpdevice_charge_init();
				if((get_mp2651_chargesta() == 5) && (get_sgm41513_chargesta() == 3))
				{
					if(neck_5pro_dev.dev_status != DEV_CHARGE_DONE)
					{
						neck_5pro_dev.dev_status = DEV_CHARGE_DONE;
						Topic_Pushlish(DEV_STA_UPDATE, &neck_5pro_dev);
					}
				}
				else
				{
					if(neck_5pro_dev.dev_status != DEV_CHARGING)
					{
						neck_5pro_dev.dev_status = DEV_CHARGING;
						Topic_Pushlish(DEV_STA_UPDATE, &neck_5pro_dev);
					}
				}
			}

			//����״̬�½���USB
			if((neck_5pro_dev.dev_global.is_dev_on == SET) && (neck_5pro_dev.dev_global.is_dev_work == SET))
			{
				neck_5pro_dev.dev_global.is_dev_work = RESET;
				Topic_Pushlish(DEV_NO_WORK, &neck_5pro_dev);
				Topic_Pushlish(BUZZER_YOWL, &neck_5pro_dev);
				Topic_Pushlish(DEV_STA_UPDATE, &neck_5pro_dev);
			}
		}
		else if((USB_CHK_INPUT == SET) && (neck_5pro_dev.dev_status != DEV_AGING))
		{
			//�γ�USB���ػ���������
			if((neck_5pro_dev.dev_global.is_dev_on == SET) && (neck_5pro_dev.dev_global.is_dev_work == RESET))
			{
				neck_5pro_dev.dev_global.is_dev_on = RESET;
				neck_5pro_dev.dev_global.is_dev_work = RESET;
//				Topic_Pushlish(BUZZER_YOWL, &neck_5pro_dev);
				Topic_Pushlish(DEV_NO_WORK, &neck_5pro_dev);
				PWR_OFF;
			}
			else if((neck_5pro_dev.dev_global.is_dev_on == RESET) && (neck_5pro_dev.dev_global.is_dev_work == SET))//�������쳣�ػ�
			{
				neck_5pro_dev.dev_global.is_dev_on = RESET;
				neck_5pro_dev.dev_global.is_dev_work = RESET;
				Topic_Pushlish(BUZZER_YOWL, &neck_5pro_dev);
				Topic_Pushlish(DEV_NO_WORK, &neck_5pro_dev);
			}
		
			mpdevice_off_init();
			if((adc_filter_struct->bat1_filter_adc < 1985) || (adc_filter_struct->bat2_filter_adc < 1287))//3.3��10.5
			{
				if(aging_timeout > 500)
					if(neck_5pro_dev.ble_pack.batt_val > 1)		neck_5pro_dev.ble_pack.batt_val = 1;
			}
			else if((adc_filter_struct->bat1_filter_adc < 2226) || (adc_filter_struct->bat2_filter_adc < 1377))//3.7��11.2
			{
				if(aging_timeout > 500)
					if(neck_5pro_dev.ble_pack.batt_val > 2)		neck_5pro_dev.ble_pack.batt_val = 2;
			}

			if(neck_5pro_dev.ble_pack.batt_val == 1)	//��ѹ����
			{
				if((neck_5pro_dev.dev_status != DEV_LOW_PWR_PORT) && (neck_5pro_dev.dev_status != DEV_BLE_CONNECTED))
				{
					neck_5pro_dev.dev_status = DEV_LOW_PWR_PORT;
					ef_get_env_blob("low_pwr_flag", &neck_5pro_dev.low_pwr_flag, sizeof(uint8_t), &save_val_len);
					if(neck_5pro_dev.low_pwr_flag != 1)
					{
						neck_5pro_dev.low_pwr_flag = 1;
						LOG_W("set low_pwr_flag = 1");
						ef_set_env_blob("low_pwr_flag", &neck_5pro_dev.low_pwr_flag, sizeof(uint8_t));
					}
					Topic_Pushlish(DEV_STA_UPDATE, &neck_5pro_dev);
				}
			}
			else if(neck_5pro_dev.ble_pack.batt_val == 2)//��ѹ
			{
				if((neck_5pro_dev.dev_status != DEV_LOW_PWR) && (neck_5pro_dev.dev_status != DEV_BLE_CONNECTED))
				{
					neck_5pro_dev.dev_status = DEV_LOW_PWR;
					ef_get_env_blob("low_pwr_flag", &neck_5pro_dev.low_pwr_flag, sizeof(uint8_t), &save_val_len);
					if(neck_5pro_dev.low_pwr_flag != 2)
					{
						neck_5pro_dev.low_pwr_flag = 2;
						LOG_W("set low_pwr_flag = 2");
						ef_set_env_blob("low_pwr_flag", &neck_5pro_dev.low_pwr_flag, sizeof(uint8_t));
					}
					Topic_Pushlish(DEV_STA_UPDATE, &neck_5pro_dev);
				}
			}
			else	//��������
			{
				if((neck_5pro_dev.dev_global.is_dev_on == SET) && (neck_5pro_dev.dev_status != DEV_NORMAL) && (neck_5pro_dev.dev_status != DEV_BLE_CONNECTED))
				{
					neck_5pro_dev.dev_status = DEV_NORMAL;
					Topic_Pushlish(DEV_STA_UPDATE, &neck_5pro_dev);
				}
			}
		}

		aging_timeout++;
		if(aging_timeout > 700)	aging_timeout = 700;
		beep_process();
		bsp_KeyScan();
		uart_poll_dma_tx(DEV_UART1);
		IWDG_ReloadKey();
		rt_thread_mdelay(10);
	}
}

