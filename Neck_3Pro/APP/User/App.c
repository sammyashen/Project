#include "include.h"

#define SOFTWARE_VERSION		"1.0"

volatile ineck_3pro_t iNeck_3Pro = {
	.dev_status 	= SLEEP,
	.ble_status 	= DISCONNECTED,
	.mass_mode 		= NECK_SOOTH_MODE,
	.mass_time 		= 600,
	.mass_remain 	= 600,
	
	.mass_motor1_speed 		= SLOW_SPEED,
	.mass_motor1_dir 		= MASS_FORWARD,
	.mass_motor2_speed 		= SLOW_SPEED,
	.mass_motor2_dir 		= MASS_FORWARD,
	.mass_motor3_speed		= SLOW_SPEED,
	.mass_motor3_dir 		= MASS_FORWARD,

	.is_travel		= SET,
	
	.heat_level 	= HEAT_NONE,
	.is_aging 		= RESET,

	.bat1_level  	= 5,
	.bat2_level		= 5,

	.block_cnt 		= 0,
	.aging_cnt		= 0,

	.is_manual_tra  = RESET,
};

extern void user_shell_task(void);
static void app_task_cb(void *para, uint32_t evt)
{
	static uint8_t cnt = 0;
	static uint8_t _10sec_cnt = 0;
	static uint8_t _7sec_cnt = 0;
	static uint8_t chardone_cnt = 0;
	static FlagStatus is_charge_en = RESET;
	uint8_t keycode = KEY_NONE;

	//1s 
	if(++cnt >= 100){
		cnt = 0;
		IWDG_ReloadKey();
		if(++_7sec_cnt >= 7)
			_7sec_cnt = 7;

//		tiny_printf("bat1:%d,bat2:%d\r\n", iNeck_3Pro.bat1_level, iNeck_3Pro.bat2_level);
		
		if(iNeck_3Pro.block_cnt != 0){
			if(++_10sec_cnt >= 10){
				_10sec_cnt = 10;
				iNeck_3Pro.block_cnt = 0;
			}
		}else{
			_10sec_cnt = 0;
		}
		if(iNeck_3Pro.block_cnt >= 3 && _10sec_cnt < 10){		//10s内连续堵转处理
			iNeck_3Pro.block_cnt = 0;
			_10sec_cnt = 0;
//			Topic_Pushlish(MASS_TOPIC, &(uint32_t){EVT_1});//关闭按摩
			Topic_Pushlish(HEAT_TOPIC, &(uint32_t){EVT_1});//关闭加热
			Topic_Pushlish(BUZZER_TOPIC, &(uint32_t){EVT_2});//长鸣1声
			Topic_Pushlish(BLE_TOPIC, &(uint32_t){EVT_1});//关闭BLE
			Topic_Pushlish(LED_TOPIC, &(uint32_t){EVT_2|EVT_7});//关闭LED
		}
		
		if(iNeck_3Pro.mass_remain > 0){
			iNeck_3Pro.mass_remain--;
			if(iNeck_3Pro.ble_status == CONNECTED){
				Topic_Pushlish(BLE_TOPIC, &(uint32_t){EVT_2});//上传心跳包
			}
		}
		else{
			if(iNeck_3Pro.is_aging == RESET){					//正常工作下
				if(iNeck_3Pro.dev_status == WORK || iNeck_3Pro.dev_status == LOW_PWR){
//					iNeck_3Pro.dev_status = SLEEP;
//					Topic_Pushlish(MASS_TOPIC, &(uint32_t){EVT_1});//关闭按摩
					Topic_Pushlish(HEAT_TOPIC, &(uint32_t){EVT_1});//关闭加热
					Topic_Pushlish(BUZZER_TOPIC, &(uint32_t){EVT_2});//长鸣1声
					Topic_Pushlish(BLE_TOPIC, &(uint32_t){EVT_1});//关闭BLE
					Topic_Pushlish(LED_TOPIC, &(uint32_t){EVT_2|EVT_7});//关闭LED
				}
			}else{									//老化模式下
				iNeck_3Pro.mass_remain = iNeck_3Pro.mass_time;
				Topic_Pushlish(MASS_TOPIC, &(uint32_t){EVT_2});//按摩间歇
			}
		}
	}

	//充电及电量检测
	if(DC_INPUT_DETECT == SET){
		if(ADC_GetSample(BAT1TEMP_SAMPLE) > 1500 && ADC_GetSample(BAT2TEMP_SAMPLE) > 1500){	//<45℃
			if(is_charge_en == RESET){
				is_charge_en = SET;
				enable_charge();
			}
//			CHARGE_ENABLE;
		}else{
			if(is_charge_en == SET){
				is_charge_en = RESET;
				disable_charge();
			}
//			CHARGE_DISABLE;
		}
	
		if(iNeck_3Pro.is_aging == RESET){
			//接入充电器关机
			if(iNeck_3Pro.dev_status == SLEEP || iNeck_3Pro.dev_status == WORK || iNeck_3Pro.dev_status == LOW_PWR){
				Topic_Pushlish(LED_TOPIC, &(uint32_t){EVT_1|EVT_7});//关闭LED
				Topic_Pushlish(BLE_TOPIC, &(uint32_t){EVT_1});//关闭BLE
//				Topic_Pushlish(MASS_TOPIC, &(uint32_t){EVT_1});//关闭按摩
				Topic_Pushlish(HEAT_TOPIC, &(uint32_t){EVT_1});//关闭加热
				if(iNeck_3Pro.dev_status != SLEEP)		Topic_Pushlish(BUZZER_TOPIC, &(uint32_t){EVT_2});//长鸣1声
				iNeck_3Pro.dev_status = CHARGING;
			}
			//充满判断
			if(CHARGE_VER_DETECT == RESET && is_charge_en == SET){
				if(++chardone_cnt >= 50){
					chardone_cnt = 0;
					if(iNeck_3Pro.dev_status == CHARGING){
						iNeck_3Pro.dev_status = CHARGDONE;
						Topic_Pushlish(LED_TOPIC, &(uint32_t){EVT_3|EVT_7});
					}
				}
			}else{
				chardone_cnt = 0;
			}
		}
	}else{
		if(iNeck_3Pro.dev_status == CHARGING || iNeck_3Pro.dev_status == CHARGDONE){
//			iNeck_3Pro.dev_status = SLEEP;
//			Topic_Pushlish(MASS_TOPIC, &(uint32_t){EVT_1});//关闭按摩
			Topic_Pushlish(HEAT_TOPIC, &(uint32_t){EVT_1});//关闭加热
			Topic_Pushlish(BLE_TOPIC, &(uint32_t){EVT_1});//关闭BLE
			Topic_Pushlish(LED_TOPIC, &(uint32_t){EVT_2|EVT_7});//关闭LED
		}
	
		if(ADC_GetSample(BAT1VAL_SAMPLE) > 2470)		{if(iNeck_3Pro.bat1_level >= 5)		iNeck_3Pro.bat1_level = 5;}	//3.98V
		else if(ADC_GetSample(BAT1VAL_SAMPLE) > 2400)	{if(iNeck_3Pro.bat1_level >= 4)		iNeck_3Pro.bat1_level = 4;}	//3.87V
		else if(ADC_GetSample(BAT1VAL_SAMPLE) > 2350)	{if(iNeck_3Pro.bat1_level >= 3)		iNeck_3Pro.bat1_level = 3;}	//3.79V
		else if(ADC_GetSample(BAT1VAL_SAMPLE) > 2265)	{if(iNeck_3Pro.bat1_level >= 2)		iNeck_3Pro.bat1_level = 2;}	//3.65V
		else if(ADC_GetSample(BAT1VAL_SAMPLE) > 2140)	{if(iNeck_3Pro.bat1_level >= 1)		iNeck_3Pro.bat1_level = 1;}	//3.45V
		else 																				iNeck_3Pro.bat1_level = 0;	//<3.45V

		if(ADC_GetSample(BAT2VAL_SAMPLE) > 2470)		{if(iNeck_3Pro.bat2_level >= 5)		iNeck_3Pro.bat2_level = 5;}
		else if(ADC_GetSample(BAT2VAL_SAMPLE) > 2400)	{if(iNeck_3Pro.bat2_level >= 4)		iNeck_3Pro.bat2_level = 4;}
		else if(ADC_GetSample(BAT2VAL_SAMPLE) > 2350)	{if(iNeck_3Pro.bat2_level >= 3)		iNeck_3Pro.bat2_level = 3;}
		else if(ADC_GetSample(BAT2VAL_SAMPLE) > 2265)	{if(iNeck_3Pro.bat2_level >= 2)		iNeck_3Pro.bat2_level = 2;}
		else if(ADC_GetSample(BAT2VAL_SAMPLE) > 2140)	{if(iNeck_3Pro.bat2_level >= 1)		iNeck_3Pro.bat2_level = 1;}
		else 											iNeck_3Pro.bat2_level = 0;

		if(iNeck_3Pro.is_aging == RESET){
			if(iNeck_3Pro.dev_status == WORK){
				if(iNeck_3Pro.bat1_level < 2 || iNeck_3Pro.bat2_level < 2){
					iNeck_3Pro.dev_status = LOW_PWR;
					Topic_Pushlish(LED_TOPIC, &(uint32_t){EVT_1});
				}
			}
			else if(iNeck_3Pro.dev_status == LOW_PWR){
				if(iNeck_3Pro.bat1_level < 1 || iNeck_3Pro.bat2_level < 1){
//					Topic_Pushlish(MASS_TOPIC, &(uint32_t){EVT_1});//关闭按摩
					Topic_Pushlish(HEAT_TOPIC, &(uint32_t){EVT_1});//关闭加热
					Topic_Pushlish(BUZZER_TOPIC, &(uint32_t){EVT_2});//长鸣1声
					Topic_Pushlish(BLE_TOPIC, &(uint32_t){EVT_1});//关闭BLE
					Topic_Pushlish(LED_TOPIC, &(uint32_t){EVT_2|EVT_7});//关闭LED
				}
			}
		}
	}

	keycode = bsp_GetKey();
	if(keycode != KEY_NONE)
	{
		switch(keycode)
		{
			case KEY_1_UP:			//开机、切转速
				if(iNeck_3Pro.dev_status == CHARGING || iNeck_3Pro.dev_status == CHARGDONE || iNeck_3Pro.is_aging == SET ||
					iNeck_3Pro.dev_status == STOPPING){
					break;
				}
				if(iNeck_3Pro.dev_status == SLEEP && GPIO_ReadInputDataBit(GPIOB, GPIO_PIN_4) == SET){
					iNeck_3Pro.dev_status = WORK;
					Topic_Pushlish(MASS_TOPIC, &(uint32_t){EVT_0});//启动按摩
					Topic_Pushlish(HEAT_TOPIC, &(uint32_t){EVT_0});//开启加热
					Topic_Pushlish(BUZZER_TOPIC, &(uint32_t){EVT_0});//短鸣1声
					Topic_Pushlish(BLE_TOPIC, &(uint32_t){EVT_0});//打开BLE
					Topic_Pushlish(LED_TOPIC, &(uint32_t){EVT_0|EVT_6});//打开LED
				}else if(iNeck_3Pro.dev_status == WORK || iNeck_3Pro.dev_status == LOW_PWR){
					if(iNeck_3Pro.mass_motor1_speed == FAST_SPEED)		iNeck_3Pro.mass_motor1_speed = SLOW_SPEED;
					else if(iNeck_3Pro.mass_motor1_speed == MID_SPEED)	iNeck_3Pro.mass_motor1_speed = FAST_SPEED;
					else												iNeck_3Pro.mass_motor1_speed = MID_SPEED;
					if(iNeck_3Pro.mass_motor2_speed == FAST_SPEED)		iNeck_3Pro.mass_motor2_speed = SLOW_SPEED;
					else if(iNeck_3Pro.mass_motor2_speed == MID_SPEED)	iNeck_3Pro.mass_motor2_speed = FAST_SPEED;
					else												iNeck_3Pro.mass_motor2_speed = MID_SPEED;
					Topic_Pushlish(BUZZER_TOPIC, &(uint32_t){EVT_1});//短鸣N声
				}
			break;

			case KEY_1_LONG:		//关机
				if(iNeck_3Pro.dev_status == CHARGING || iNeck_3Pro.dev_status == CHARGDONE || iNeck_3Pro.dev_status == STOPPING){
					break;
				}
				if(iNeck_3Pro.dev_status == WORK || iNeck_3Pro.dev_status == LOW_PWR){
//					iNeck_3Pro.dev_status = SLEEP;
//					Topic_Pushlish(MASS_TOPIC, &(uint32_t){EVT_1});//关闭按摩
					Topic_Pushlish(HEAT_TOPIC, &(uint32_t){EVT_1});//关闭加热
					Topic_Pushlish(BUZZER_TOPIC, &(uint32_t){EVT_2});//长鸣1声
					Topic_Pushlish(BLE_TOPIC, &(uint32_t){EVT_1});//关闭BLE
					Topic_Pushlish(LED_TOPIC, &(uint32_t){EVT_2|EVT_7});//关闭LED
				}
			break;

			case KEY_1_LONG_UP:		
				if(iNeck_3Pro.dev_status == SLEEP){
					MCU_PWR_DISABLE;
				}
			break;

			case KEY_1_DOWN:		
				if(iNeck_3Pro.dev_status == SLEEP){
					MCU_PWR_ENABLE;
				}
			break;

			case KEY_2_UP:			//切模式
				if(iNeck_3Pro.dev_status == CHARGING || iNeck_3Pro.dev_status == CHARGDONE || iNeck_3Pro.is_aging == SET ||
					iNeck_3Pro.dev_status == STOPPING){
					break;
				}
				if(iNeck_3Pro.mass_mode == NECK_SOOTH_MODE){
					iNeck_3Pro.mass_mode = MASS_MODE;
					iNeck_3Pro.heat_level = HEAT_NONE;
				}else if(iNeck_3Pro.mass_mode == KNEAD_MODE){
					iNeck_3Pro.mass_mode = NECK_SOOTH_MODE;
					iNeck_3Pro.heat_level = HEAT_WARM;
				}else{
					iNeck_3Pro.mass_mode = KNEAD_MODE;
					iNeck_3Pro.heat_level = HEAT_NONE;
				}
				Topic_Pushlish(BUZZER_TOPIC, &(uint32_t){EVT_0});//短鸣1声
			break;

			case KEY_2_LONG:		//手动移位
				if(iNeck_3Pro.dev_status == CHARGING || iNeck_3Pro.dev_status == CHARGDONE || iNeck_3Pro.is_aging == SET ||
					iNeck_3Pro.dev_status == STOPPING){
					break;
				}
				iNeck_3Pro.is_travel = RESET;
				iNeck_3Pro.is_manual_tra = SET;
				Topic_Pushlish(BUZZER_TOPIC, &(uint32_t){EVT_0});//短鸣1声
			break;

			case KEY_2_LONG_UP:		//手动移位停止
				if(iNeck_3Pro.dev_status == CHARGING || iNeck_3Pro.dev_status == CHARGDONE || iNeck_3Pro.is_aging == SET ||
					iNeck_3Pro.dev_status == STOPPING){
					break;
				}
				iNeck_3Pro.is_travel = RESET;
				iNeck_3Pro.is_manual_tra = RESET;
			break;

			case KEY_3_LONG:		//进老化
				if(iNeck_3Pro.dev_status == CHARGING || iNeck_3Pro.dev_status == CHARGDONE || iNeck_3Pro.is_aging == SET || 
					iNeck_3Pro.dev_status == STOPPING){
					break;
				}
				if(_7sec_cnt < 7){
					iNeck_3Pro.dev_status = WORK;
					iNeck_3Pro.is_aging = SET;
					iNeck_3Pro.mass_mode = NECK_SOOTH_MODE;
					iNeck_3Pro.heat_level = HEAT_WARM;
					Topic_Pushlish(MASS_TOPIC, &(uint32_t){EVT_0});//启动按摩
					Topic_Pushlish(HEAT_TOPIC, &(uint32_t){EVT_0});//开启加热
					Topic_Pushlish(BLE_TOPIC, &(uint32_t){EVT_1});//关闭BLE
					Topic_Pushlish(BUZZER_TOPIC, &(uint32_t){EVT_0});//短鸣1声
					Topic_Pushlish(LED_TOPIC, &(uint32_t){EVT_3});//绿灯常亮
				}
			break;
		}
	}

	bsp_KeyScan();
	user_shell_task();
	uart_poll_dma_tx(DEV_UART1);
}

void topic(int argc, char **argv)
{
	uint32_t evt = atoi(argv[2]);
	Topic_Pushlish(atoi(argv[1]), &evt);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
topic, topic, topic publish test);

void get_psw(int argc, char **argv)
{
	if(atoi(argv[1]) == 0){
		if(PSW1_DETECT == SET)	tiny_printf("psw1 is high.\r\n");
		else					tiny_printf("psw1 is low.\r\n");
	}else if(atoi(argv[1]) == 1){
		if(PSW2_DETECT == SET)	tiny_printf("psw2 is high.\r\n");
		else					tiny_printf("psw2 is low.\r\n");
	}
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
get_psw, get_psw, get psw status);

void mass_dir(int argc, char **argv)
{
	uint32_t dir = atoi(argv[1]);
	if(dir == 0){
		iNeck_3Pro.mass_motor1_dir = MASS_FORWARD;
		iNeck_3Pro.mass_motor2_dir = MASS_FORWARD;
	}else{
		iNeck_3Pro.mass_motor1_dir = MASS_REVERSE;
		iNeck_3Pro.mass_motor2_dir = MASS_REVERSE;
	}
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
mass_dir, mass_dir, change mass dir);

void mass_speed(int argc, char **argv)
{
	uint32_t speed = atoi(argv[1]);
	if(speed == 0){
		iNeck_3Pro.mass_motor1_speed = NONE_SPEED;
		iNeck_3Pro.mass_motor2_speed = NONE_SPEED;
	}else if(speed == 1){
		iNeck_3Pro.mass_motor1_speed = SLOW_SPEED;
		iNeck_3Pro.mass_motor2_speed = SLOW_SPEED;
	}else if(speed == 2){
		iNeck_3Pro.mass_motor1_speed = MID_SPEED;
		iNeck_3Pro.mass_motor2_speed = MID_SPEED;
	}else{
		iNeck_3Pro.mass_motor1_speed = FAST_SPEED;
		iNeck_3Pro.mass_motor2_speed = FAST_SPEED;
	}
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
mass_speed, mass_speed, change mass speed);

void re_boot(void)
{
	SoftReset();
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
re_boot, re_boot, sys reset);

void iap(void)
{
	//使用例程板级初始化
	__disable_irq();
	bsp_init();
	__enable_irq();
	SerialDownload();
	SoftReset();
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
iap, iap, iap test);

static void iwdg_init(void)
{
	IWDG_WriteConfig(IWDG_WRITE_ENABLE);
	IWDG_SetPrescalerDiv(IWDG_PRESCALER_DIV256);		//40K/256=156Hz(6.4ms)
	IWDG_CntReload(469);								//3s/6.4ms=468.75
	IWDG_ReloadKey();
	IWDG_Enable();
}
device_initcall(iwdg_init);

void app_init(void)
{
	static task_t app_task;
	
    tiny_timer_create(&app_task, app_task_cb, NULL);
    tiny_timer_start(&app_task, TIMER_FOREVER, 10);
}
app_initcall(app_init);

void topic_init(void)
{
	Topic_Init(BUZZER_TOPIC, NULL, NULL);
	Topic_Init(LED_TOPIC, NULL, NULL);
	Topic_Init(HEAT_TOPIC, NULL, NULL);
	Topic_Init(MOTOR_TOPIC, NULL, NULL);
	Topic_Init(BLE_TOPIC, NULL, NULL);
	Topic_Init(MASS_TOPIC, NULL, NULL);
}
app_initcall(topic_init);

/**
 * @brief  	  : main func.
 * @param[1]  : none.
 * @return    : none.
 */
int main(void)
{
//	NVIC_SetVectorTab();
	__disable_irq();
	do_init_call();
	__enable_irq();
	tiny_printf("software version:%s", SOFTWARE_VERSION);
	while(1)
	{
		tiny_task_loop();
	}
}


