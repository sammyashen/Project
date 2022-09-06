#include "dev_led.h"
#include "topic.h"
#include "include.h"

static void aw21024_readbyte(uint8_t *_pReadBuf, uint8_t _ucDevAddr, uint8_t _ucAddress)
{
	sf_i2c_led_start();
	sf_i2c_led_sendbyte(_ucDevAddr | 0x00);
	if(sf_i2c_led_waitack() != 0)		goto cmd_fail;	
	sf_i2c_led_sendbyte(_ucAddress);
	if(sf_i2c_led_waitack() != 0)		goto cmd_fail;
	sf_i2c_led_start();
	sf_i2c_led_sendbyte(_ucDevAddr | 0x01);
	if(sf_i2c_led_waitack() != 0)		goto cmd_fail;
	*_pReadBuf = sf_i2c_led_readbyte();
	sf_i2c_led_nack();
	sf_i2c_led_stop();

cmd_fail: 
	sf_i2c_led_stop();
}

static void aw21024_writebyte(uint8_t _ucWriteByte, uint8_t _ucDevAddr, uint8_t _ucAddress)
{
	sf_i2c_led_start();
	sf_i2c_led_sendbyte(_ucDevAddr | 0x00);
	if(sf_i2c_led_waitack() != 0)		goto cmd_fail;	
	sf_i2c_led_sendbyte(_ucAddress);
	if(sf_i2c_led_waitack() != 0)		goto cmd_fail;	
	sf_i2c_led_sendbyte(_ucWriteByte);
	sf_i2c_led_nack();
	sf_i2c_led_stop();

cmd_fail: 
	sf_i2c_led_stop();
}

static void led_init(void)
{
	sf_i2c_led_init();
	if(sf_i2c_led_check_device(AW21024_ADDR) == 0x00)
	{
		aw21024_writebyte(0x00, AW21024_ADDR, ADDR_RESET);
		aw21024_writebyte(0x81, AW21024_ADDR, ADDR_GCR);	
		aw21024_writebyte(0x00, AW21024_ADDR, ADDR_PATCFG);
		aw21024_writebyte(0x00, AW21024_ADDR, ADDR_GCFG0);
		aw21024_writebyte(0x00, AW21024_ADDR, ADDR_GCR2);
		aw21024_writebyte(0xFF, AW21024_ADDR, ADDR_GCCR);
		aw21024_writebyte(0x25, AW21024_ADDR, ADDR_WBR);
		aw21024_writebyte(0x25, AW21024_ADDR, ADDR_WBG);
		aw21024_writebyte(0x20, AW21024_ADDR, ADDR_WBB);	
		aw21024_writebyte(0xFF, AW21024_ADDR, ADDR_GCOLR);
		aw21024_writebyte(0xFF, AW21024_ADDR, ADDR_GCOLG);
		aw21024_writebyte(0xFF, AW21024_ADDR, ADDR_GCOLB);

		for(uint8_t i = 0;i < 24;i++)
		{
			aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_BR + i));
			aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
			aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_COL + i));
		}
		rt_kprintf("led configure successed.\r\n");
	}
	else
	{
		sf_i2c_led_stop();
		rt_kprintf("led configure failed.\r\n");
	}
}

const uint8_t gc_ucSoftLEDIndex[6] = {5, 6, 8, 10, 12, 14};/* display "1" */
const uint8_t gc_ucMedLEDIndex[14] = {0, 1, 2, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 15};/* display "2" */
const uint8_t gc_ucHardLEDIndex[14] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 14, 15};/* display "3" */
//const uint8_t gc_ucUserLEDIndex[7] = {1, 2, 4, 7, 8, 10, 11};/* display "4" */
const uint8_t gc_ucUserLEDIndex[12] = {0, 2, 3, 4, 5, 6, 7, 9, 12, 13, 14, 15};/* display "0" */

static void set_mode_led(uint8_t mode_num, uint8_t brightness)
{
	uint8_t i;

	rt_enter_critical();
	for(i = 0;i < 16;i++)//全灭
	{
		aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_BR + i));
		aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
		aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_COL + i));
	}

	switch(mode_num)
	{
		case 1:
			for(i = 0;i < sizeof(gc_ucSoftLEDIndex);i++)
			{
				aw21024_writebyte(brightness, AW21024_ADDR, (ADDR_BR + gc_ucSoftLEDIndex[i]));
				aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
				aw21024_writebyte(0xFF, AW21024_ADDR, (ADDR_COL + gc_ucSoftLEDIndex[i]));
			}
		break;

		case 2:
			for(i = 0;i < sizeof(gc_ucMedLEDIndex);i++)
			{
				aw21024_writebyte(brightness, AW21024_ADDR, (ADDR_BR + gc_ucMedLEDIndex[i]));
				aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
				aw21024_writebyte(0xFF, AW21024_ADDR, (ADDR_COL + gc_ucMedLEDIndex[i]));
			}
		break;

		case 3:
			for(i = 0;i < sizeof(gc_ucHardLEDIndex);i++)
			{
				aw21024_writebyte(brightness, AW21024_ADDR, (ADDR_BR + gc_ucHardLEDIndex[i]));
				aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
				aw21024_writebyte(0xFF, AW21024_ADDR, (ADDR_COL + gc_ucHardLEDIndex[i]));
			}
		break;

		case 4:
			for(i = 0;i < sizeof(gc_ucUserLEDIndex);i++)
			{
				aw21024_writebyte(brightness, AW21024_ADDR, (ADDR_BR + gc_ucUserLEDIndex[i]));
				aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
				aw21024_writebyte(0xFF, AW21024_ADDR, (ADDR_COL + gc_ucUserLEDIndex[i]));
			}
		break;

		default:
		break;
	}
	rt_exit_critical();
}

static uint8_t get_mode_led_bri(void)
{
	uint8_t bri = 0;

	aw21024_readbyte(&bri, AW21024_ADDR, ADDR_BR+12);

	return bri;
}

static void set_sta_led(eLEDColor color, uint8_t brightness)
{
	switch(color)
	{
		case RED:
			//R
			aw21024_writebyte(brightness, AW21024_ADDR, (ADDR_BR + 20));
			aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
			aw21024_writebyte(0xFF, AW21024_ADDR, (ADDR_COL + 20));
			//G
			aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_BR + 19));
			aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
			aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_COL + 19));
			//B
			aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_BR + 18));
			aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
			aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_COL + 18));
		break;

		case GREEN:
			//R
			aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_BR + 20));
			aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
			aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_COL + 20));
			//G
			aw21024_writebyte(brightness, AW21024_ADDR, (ADDR_BR + 19));
			aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
			aw21024_writebyte(0xFF, AW21024_ADDR, (ADDR_COL + 19));
			//B
			aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_BR + 18));
			aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
			aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_COL + 18));
		break;

		case BLUE:
			//R
			aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_BR + 20));
			aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
			aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_COL + 20));
			//G
			aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_BR + 19));
			aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
			aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_COL + 19));
			//B
			aw21024_writebyte(brightness, AW21024_ADDR, (ADDR_BR + 18));
			aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
			aw21024_writebyte(0xFF, AW21024_ADDR, (ADDR_COL + 18));
		break;

		case PURPLE:
			//R
			aw21024_writebyte(0xB2, AW21024_ADDR, (ADDR_BR + 20));
			aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
			aw21024_writebyte(0xB4, AW21024_ADDR, (ADDR_COL + 20));
			//G
			aw21024_writebyte(0xB2, AW21024_ADDR, (ADDR_BR + 19));
			aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
			aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_COL + 19));
			//B
			aw21024_writebyte(0x86, AW21024_ADDR, (ADDR_BR + 18));
			aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
			aw21024_writebyte(0x96, AW21024_ADDR, (ADDR_COL + 18));
		break;

		default:
			//R
			aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_BR + 20));
			aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
			aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_COL + 20));
			//G
			aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_BR + 19));
			aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
			aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_COL + 19));
			//B
			aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_BR + 18));
			aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
			aw21024_writebyte(0x00, AW21024_ADDR, (ADDR_COL + 18));
		break;
	}
}

static uint8_t get_sta_led_bri(eLEDColor color)
{
	uint8_t bri = 0;

	if(color == RED)
		aw21024_readbyte(&bri, AW21024_ADDR, ADDR_BR+20);
	else if(color == GREEN)
		aw21024_readbyte(&bri, AW21024_ADDR, ADDR_BR+19);
	else 
		aw21024_readbyte(&bri, AW21024_ADDR, ADDR_BR+18);

	return bri;
}

const uint8_t gc_ucBackgroundIndex[2] = {16, 17};
void set_background_led(uint8_t brightness)
{
	for(uint8_t i = 0;i < sizeof(gc_ucBackgroundIndex);i++)
	{
		aw21024_writebyte(brightness, AW21024_ADDR, (ADDR_BR + gc_ucBackgroundIndex[i]));
		aw21024_writebyte(0x00, AW21024_ADDR, ADDR_UPDATE);
		aw21024_writebyte(0xFF, AW21024_ADDR, (ADDR_COL + gc_ucBackgroundIndex[i]));
	}
}

static led_ctrl_t led_ctrl;
rt_timer_t mode_led_sftimer = RT_NULL, sta_led_sftimer = RT_NULL;
//led设备定时器回调函数
static void mode_led_sftimer_cb(void *para)
{
	static uint8_t disp_num_tmp = 0;
	led_ctrl_t *led_ctrl_struct = &led_ctrl;
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();

	if(led_ctrl_struct->mode_led_ctrl.opt_type == LED_BLINK)
	{
		if(led_ctrl_struct->mode_led_ctrl.is_blink_forever != 0x00)
		{
			if(get_mode_led_bri() != 0x00)
				set_mode_led(led_ctrl_struct->mode_led_ctrl.disp_num, 0x00);
			else
				set_mode_led(led_ctrl_struct->mode_led_ctrl.disp_num, 0xFF);
		}
		else
		{
			if(led_ctrl_struct->mode_led_ctrl.blink_cnt++ >= (led_ctrl_struct->mode_led_ctrl.blink_target<<1))
			{
				led_ctrl_struct->mode_led_ctrl.blink_cnt = (led_ctrl_struct->mode_led_ctrl.blink_target<<1);
				set_mode_led(led_ctrl_struct->mode_led_ctrl.disp_num, 0x00);
			}
			else
			{
				if(get_mode_led_bri() != 0x00)
					set_mode_led(led_ctrl_struct->mode_led_ctrl.disp_num, 0x00);
				else
					set_mode_led(led_ctrl_struct->mode_led_ctrl.disp_num, 0xFF);
			}
		}
	}
	else if(led_ctrl_struct->mode_led_ctrl.opt_type == LED_ON)
	{
		if(disp_num_tmp != led_ctrl_struct->mode_led_ctrl.disp_num)
		{
			disp_num_tmp = led_ctrl_struct->mode_led_ctrl.disp_num;
			set_mode_led(led_ctrl_struct->mode_led_ctrl.disp_num, 0xFF);
		}
	}
	else if(led_ctrl_struct->mode_led_ctrl.opt_type == LED_OFF)
	{
		set_background_led(0x00);
		set_mode_led(led_ctrl_struct->mode_led_ctrl.disp_num, 0x00);
		if(mode_led_sftimer != RT_NULL)		rt_timer_stop(mode_led_sftimer);
	}
}

const uint8_t gc_ucLEDBrBright[250] = {	
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6,	
	6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 
	14, 14, 14, 15, 15, 15, 16, 16, 16, 17, 17, 18, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 
	28, 28, 29, 30, 30, 31, 32, 32, 33, 34, 35, 36, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 55,
	56, 57, 59, 60, 61, 63, 64, 66, 67, 69, 70, 72, 74, 75, 77, 79, 81, 82, 84, 86, 88, 90, 92, 94, 97, 99, 101, 103, 106, 108,
	111, 113, 116, 119, 121, 124, 127, 130, 133, 136, 139, 142, 145, 149, 152, 156, 159, 163, 167, 171, 174, 179, 183, 187, 191,
	196, 200, 205, 209, 214, 219, 224, 229, 235, 240, 245, 251, 252, 253, 254, 255, 255
};

static void sta_led_sftimer_cb(void *para)
{
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();
	led_ctrl_t *led_ctrl_struct = &led_ctrl;
	static uint8_t breath_phase = 0;

	if(led_ctrl_struct->sta_led_ctrl.opt_type != LED_BREATH)
		breath_phase = 0;

	if(led_ctrl_struct->sta_led_ctrl.opt_type == LED_BLINK)
	{
		if(led_ctrl_struct->sta_led_ctrl.is_blink_forever != 0x00)
		{
			if(get_sta_led_bri(led_ctrl_struct->sta_led_ctrl.color) != 0x00)
			{
//				set_sta_led(led_ctrl_struct->sta_led_ctrl.color, 0x00);
//				if(neck_5pro_dev_struct->dev_status == DEV_AGING)//老化时，特殊处理
//				{
//					if(led_ctrl_struct->sta_led_ctrl.color == RED)			led_ctrl_struct->sta_led_ctrl.color = GREEN;
//					else if(led_ctrl_struct->sta_led_ctrl.color == GREEN)	led_ctrl_struct->sta_led_ctrl.color = BLUE;
//					else if(led_ctrl_struct->sta_led_ctrl.color == BLUE)	led_ctrl_struct->sta_led_ctrl.color = RED;
//					else													led_ctrl_struct->sta_led_ctrl.color = RED;
//				}
				if(neck_5pro_dev_struct->dev_global.is_aging_done == RESET)//老化时，特殊处理
				{
					set_sta_led(led_ctrl_struct->sta_led_ctrl.color, 0x00);
					if(neck_5pro_dev_struct->dev_status == DEV_AGING)
					{
						if(led_ctrl_struct->sta_led_ctrl.color == RED)			led_ctrl_struct->sta_led_ctrl.color = GREEN;
						else if(led_ctrl_struct->sta_led_ctrl.color == GREEN)	led_ctrl_struct->sta_led_ctrl.color = BLUE;
						else if(led_ctrl_struct->sta_led_ctrl.color == BLUE)	led_ctrl_struct->sta_led_ctrl.color = RED;
						else													led_ctrl_struct->sta_led_ctrl.color = RED;
					}
				}
				else
				{
					led_ctrl_struct->sta_led_ctrl.color = PURPLE;
					set_sta_led(led_ctrl_struct->sta_led_ctrl.color, 0xFF);
				}
			}
			else
				set_sta_led(led_ctrl_struct->sta_led_ctrl.color, 0xFF);
		}
		else
		{
			if(led_ctrl_struct->sta_led_ctrl.blink_cnt++ >= (led_ctrl_struct->sta_led_ctrl.blink_target<<1))
			{
				led_ctrl_struct->sta_led_ctrl.blink_cnt = (led_ctrl_struct->sta_led_ctrl.blink_target<<1);
				set_sta_led(led_ctrl_struct->sta_led_ctrl.color, 0x00);
				if(led_ctrl_struct->sta_led_ctrl.blink_target == 0x03)	//关机
				{
					neck_5pro_dev_struct->dev_global.is_dev_on = RESET;
//					neck_5pro_dev_struct->dev_global.is_dev_work = RESET;
//					Topic_Pushlish(BUZZER_YOWL, neck_5pro_dev_struct);
				}
				else if(led_ctrl_struct->sta_led_ctrl.blink_target == 0x05)	//更新状态
				{
					neck_5pro_dev_struct->dev_status = neck_5pro_dev_struct->ble_con_pre_sta;
					Topic_Pushlish(DEV_STA_UPDATE, neck_5pro_dev_struct);
				}
			}
			else
			{
				if(get_sta_led_bri(led_ctrl_struct->sta_led_ctrl.color) != 0x00)
					set_sta_led(led_ctrl_struct->sta_led_ctrl.color, 0x00);
				else
					set_sta_led(led_ctrl_struct->sta_led_ctrl.color, 0xFF);
			}
		}
	}
	else if(led_ctrl_struct->sta_led_ctrl.opt_type == LED_BREATH)
	{
		switch(breath_phase)
		{
			case 0:
				if(get_sta_led_bri(led_ctrl_struct->sta_led_ctrl.color) != 0x00)
				{
					led_ctrl_struct->sta_led_ctrl.breath_dir = 0x01;
					led_ctrl_struct->sta_led_ctrl.breath_index = 249;
				}
				else
				{
					led_ctrl_struct->sta_led_ctrl.breath_dir = 0;
					led_ctrl_struct->sta_led_ctrl.breath_index = 0;
				}
				breath_phase = 0x01;
			break;

			case 1:
				if(led_ctrl_struct->sta_led_ctrl.breath_dir != 0x00)//递减
				{
					if(led_ctrl_struct->sta_led_ctrl.breath_index > 0)	led_ctrl_struct->sta_led_ctrl.breath_index--;
					else												led_ctrl_struct->sta_led_ctrl.breath_dir = 0x00;
				}
				else												//递增
				{
					if(led_ctrl_struct->sta_led_ctrl.breath_index >= 249)	led_ctrl_struct->sta_led_ctrl.breath_dir = 0x01;
					else													led_ctrl_struct->sta_led_ctrl.breath_index++;
				}
				set_sta_led(led_ctrl_struct->sta_led_ctrl.color, gc_ucLEDBrBright[led_ctrl_struct->sta_led_ctrl.breath_index]);
			break;
		}
	}
	else if(led_ctrl_struct->sta_led_ctrl.opt_type == LED_ON)
	{
		set_sta_led(led_ctrl_struct->sta_led_ctrl.color, 0xFF);
	}
	else if(led_ctrl_struct->sta_led_ctrl.opt_type == LED_OFF)
	{
		set_sta_led(NONE, 0x00);
		if(sta_led_sftimer != RT_NULL)		rt_timer_stop(sta_led_sftimer);
	}
}

//led设备定时器初始化
void led_sftimer_init(void)
{
	mode_led_sftimer = rt_timer_create("mode_led_timer", mode_led_sftimer_cb, RT_NULL, 50, RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_PERIODIC);

	sta_led_sftimer = rt_timer_create("sta_led_timer", sta_led_sftimer_cb, RT_NULL, 50, RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_PERIODIC);
}

static void led_dev_work_cb(void *msg)
{
	rt_uint32_t time;
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();

	led_ctrl.mode_led_ctrl.opt_type = LED_ON;
	led_ctrl.mode_led_ctrl.disp_num = neck_5pro_dev_struct->ble_pack.mass_mode;
	set_background_led(0xFF);
	set_mode_led(led_ctrl.mode_led_ctrl.disp_num, 0xFF);
	
	time = 50;
	rt_timer_control(mode_led_sftimer, RT_TIMER_CTRL_SET_TIME, &time);
	rt_timer_start(mode_led_sftimer);
}

static void led_dev_no_work_cb(void *msg)
{
	led_ctrl.sta_led_ctrl.opt_type = LED_OFF;
	led_ctrl.mode_led_ctrl.opt_type = LED_OFF;
}

static void led_dev_manual_sw_cb(void *msg)
{
	rt_uint32_t time;
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();

	if(neck_5pro_dev_struct->ble_pack.is_manual == SET)
	{
		led_ctrl.mode_led_ctrl.disp_num = neck_5pro_dev_struct->ble_pack.mass_mode;
		led_ctrl.mode_led_ctrl.is_blink_forever = 0x01;
		if(led_ctrl.mode_led_ctrl.opt_type != LED_BLINK)
		{
			led_ctrl.mode_led_ctrl.opt_type = LED_BLINK;
			set_mode_led(led_ctrl.mode_led_ctrl.disp_num, 0x00);
		}

		time = 500;
		rt_timer_control(mode_led_sftimer, RT_TIMER_CTRL_SET_TIME, &time);
		rt_timer_start(mode_led_sftimer);
	}
	else
	{
		led_ctrl.mode_led_ctrl.disp_num = neck_5pro_dev_struct->ble_pack.mass_mode;
		if(led_ctrl.mode_led_ctrl.opt_type != LED_ON)
		{
			led_ctrl.mode_led_ctrl.opt_type = LED_ON;
			set_mode_led(led_ctrl.mode_led_ctrl.disp_num, 0xFF);
		}

		time = 50;
		rt_timer_control(mode_led_sftimer, RT_TIMER_CTRL_SET_TIME, &time);
		rt_timer_start(mode_led_sftimer);
	}
}

static void led_mode_change_cb(void *msg)
{
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();

	led_ctrl.mode_led_ctrl.disp_num = neck_5pro_dev_struct->ble_pack.mass_mode;
}

static void led_dev_sta_update_cb(void *msg)
{
	rt_uint32_t time;
	neck_5pro_t *neck_5pro_dev_struct = get_neck_5pro_struct();

	switch(neck_5pro_dev_struct->dev_status)
	{
		case DEV_NORMAL:
			led_ctrl.sta_led_ctrl.opt_type = LED_ON;
			led_ctrl.sta_led_ctrl.color = PURPLE;

			time = 50;
			rt_timer_control(sta_led_sftimer, RT_TIMER_CTRL_SET_TIME, &time);
			rt_timer_start(sta_led_sftimer);
		break;

		case DEV_LOW_PWR:
			led_ctrl.sta_led_ctrl.opt_type = LED_BLINK;
			led_ctrl.sta_led_ctrl.is_blink_forever = 0x01;
			led_ctrl.sta_led_ctrl.color = RED;

			time = 500;
			rt_timer_control(sta_led_sftimer, RT_TIMER_CTRL_SET_TIME, &time);
			rt_timer_start(sta_led_sftimer);
		break;

		case DEV_LOW_PWR_PORT:
			led_ctrl.sta_led_ctrl.opt_type = LED_BLINK;
			led_ctrl.sta_led_ctrl.is_blink_forever = 0x00;
			led_ctrl.sta_led_ctrl.color = RED;
			led_ctrl.sta_led_ctrl.blink_cnt = 0;
			led_ctrl.sta_led_ctrl.blink_target = 3;

			time = 500;
			rt_timer_control(sta_led_sftimer, RT_TIMER_CTRL_SET_TIME, &time);
			rt_timer_start(sta_led_sftimer);
		break;

		case DEV_CHARGING:
			led_ctrl.sta_led_ctrl.opt_type = LED_BREATH;
			led_ctrl.sta_led_ctrl.color = RED;

			time = 10;
			rt_timer_control(sta_led_sftimer, RT_TIMER_CTRL_SET_TIME, &time);
			rt_timer_start(sta_led_sftimer);
		break;

		case DEV_CHARGE_DONE:
			led_ctrl.sta_led_ctrl.opt_type = LED_ON;
			led_ctrl.sta_led_ctrl.color = PURPLE;

			time = 50;
			rt_timer_control(sta_led_sftimer, RT_TIMER_CTRL_SET_TIME, &time);
			rt_timer_start(sta_led_sftimer);
		break;

		case DEV_CHARGE_PORT:
			led_ctrl.sta_led_ctrl.opt_type = LED_ON;
			led_ctrl.sta_led_ctrl.color = RED;

			time = 50;
			rt_timer_control(sta_led_sftimer, RT_TIMER_CTRL_SET_TIME, &time);
			rt_timer_start(sta_led_sftimer);
		break;

		case DEV_BLE_CONNECTED:
			led_ctrl.sta_led_ctrl.opt_type = LED_BLINK;
			led_ctrl.sta_led_ctrl.is_blink_forever = 0x00;
			led_ctrl.sta_led_ctrl.color = BLUE;
			led_ctrl.sta_led_ctrl.blink_cnt = 0;
			led_ctrl.sta_led_ctrl.blink_target = 5;

			time = 200;
			rt_timer_control(sta_led_sftimer, RT_TIMER_CTRL_SET_TIME, &time);
			rt_timer_start(sta_led_sftimer);
		break;

		case DEV_AGING:
			led_ctrl.sta_led_ctrl.opt_type = LED_BLINK;
			led_ctrl.sta_led_ctrl.is_blink_forever = 0x01;
			led_ctrl.sta_led_ctrl.color = RED;

			time = 500;
			rt_timer_control(sta_led_sftimer, RT_TIMER_CTRL_SET_TIME, &time);
			rt_timer_start(sta_led_sftimer);
		break;

		default:
			led_ctrl.sta_led_ctrl.opt_type = LED_OFF;
		break;
	}
}

//led设备相关主题订阅
static void led_about_topic_subscrib(void)
{
	Topic_Subscrib(DEV_WORK, led_dev_work_cb);
	Topic_Subscrib(DEV_NO_WORK, led_dev_no_work_cb);
	Topic_Subscrib(DEV_MODE_CHANGE, led_mode_change_cb);
	Topic_Subscrib(DEV_STA_UPDATE, led_dev_sta_update_cb);
	Topic_Subscrib(DEV_MANUAL_SW, led_dev_manual_sw_cb);
}
user_initcall(led_about_topic_subscrib);

device_initcall(led_init);


