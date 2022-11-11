#include "dev_led.h"

static ble_led_t g_tBLE_LED;
#define BLE_LED_ENABLE()	BT_LED_ON
#define BLE_LED_DISABLE()	BT_LED_OFF

static void ble_led_start(uint16_t _usOnTime, uint16_t _usOffTime, uint16_t _usCycle)
{
	if (_usOnTime == 0)
	{
		return;
	}

	g_tBLE_LED.usOnTime = _usOnTime;
	g_tBLE_LED.usOffTime = _usOffTime;
	g_tBLE_LED.usCycle = _usCycle;
	g_tBLE_LED.usCount = 0;
	g_tBLE_LED.usCycleCount = 0;
	g_tBLE_LED.ucState = 0;
	g_tBLE_LED.ucEnalbe = 1;	

	BLE_LED_ENABLE();	
}

static void ble_led_process(void)
{
	if ((g_tBLE_LED.ucEnalbe == 0) || (g_tBLE_LED.usOffTime == 0))
	{
		return;
	}

	if (g_tBLE_LED.ucState == 0)
	{
		if (g_tBLE_LED.usOffTime > 0)	
		{
			if (++g_tBLE_LED.usCount >= g_tBLE_LED.usOnTime)
			{
				BLE_LED_DISABLE();		
				g_tBLE_LED.usCount = 0;
				g_tBLE_LED.ucState = 1;
			}
		}
		else
		{
			;	
		}
	}
	else if (g_tBLE_LED.ucState == 1)
	{
		if (++g_tBLE_LED.usCount >= g_tBLE_LED.usOffTime)
		{
			if (g_tBLE_LED.usCycle > 0)
			{
				if (++g_tBLE_LED.usCycleCount >= g_tBLE_LED.usCycle)
				{
					g_tBLE_LED.ucEnalbe = 0;
				}

				if (g_tBLE_LED.ucEnalbe == 0)
				{
					g_tBLE_LED.usOffTime = 0;
					return;
				}
			}

			g_tBLE_LED.usCount = 0;
			g_tBLE_LED.ucState = 0;

			BLE_LED_ENABLE();			
		}
	}
}

static task_t led_task;

/*
*EVT_0：打开状态灯
*EVT_1：充电、低压
*EVT_2：关闭状态灯
*EVT_3：充满、老化
*EVT_4：BLE断开连接
*EVT_5：BLE建立连接
*EVT_6：打开BLE灯
*EVT_7: 关闭BLE灯
*/
static void led_task_cb(void *para, uint32_t evt)
{
	static uint8_t cnt = 0;
	static uint8_t ble_cnt = 0;
	static FlagStatus blink_flag = RESET;
	static FlagStatus ble_fast_blink_flag = RESET;
	static FlagStatus ble_on = RESET;

	ble_led_process();

	if(evt & EVT_0){
		tiny_clr_event(&led_task, EVT_0);
		if(RED_LED_DECTECT == SET)		RED_LED_ON;
		if(GREEN_LED_DECTECT == RESET)	GREEN_LED_OFF;
		if(blink_flag == SET)			blink_flag = RESET;
	}

	if(evt & EVT_1){
		tiny_clr_event(&led_task, EVT_1);
		if(blink_flag == RESET)			blink_flag = SET;
	}

	if(evt & EVT_2){
		tiny_clr_event(&led_task, EVT_2);
		if(RED_LED_DECTECT == RESET)	RED_LED_OFF;
		if(GREEN_LED_DECTECT == RESET)	GREEN_LED_OFF;
		if(blink_flag == SET)			blink_flag = RESET;
	}

	if(evt & EVT_3){
		tiny_clr_event(&led_task, EVT_3);
		if(RED_LED_DECTECT == RESET)	RED_LED_OFF;
		if(GREEN_LED_DECTECT == SET)	GREEN_LED_ON;
		if(blink_flag == SET)			blink_flag = RESET;
	}

	if(evt & EVT_4){
		tiny_clr_event(&led_task, EVT_4);
		if(ble_fast_blink_flag == SET)	ble_fast_blink_flag = RESET;
	}

	if(evt & EVT_5){
		tiny_clr_event(&led_task, EVT_5);
		if(ble_fast_blink_flag == RESET)		ble_fast_blink_flag = SET;
	}

	if(evt & EVT_6){
		tiny_clr_event(&led_task, EVT_6);
		if(ble_on == RESET)		ble_on = SET;
	}

	if(evt & EVT_7){
		tiny_clr_event(&led_task, EVT_7);
		if(ble_on == SET)		ble_on = RESET;
	}

	if(blink_flag){
		if(++cnt >= 10){
			cnt = 0;
			RED_LED_REV;
			GREEN_LED_OFF;
		}
	}
	else{
		cnt = 0;
	}

	if(ble_fast_blink_flag){
		if(++ble_cnt >= 20){
			ble_cnt = 0;
			if(ble_on)
				ble_led_start(1, 1, 2);
		}
	}
	else{
		if(++ble_cnt >= 40){
			ble_cnt = 0;
			if(ble_on)
				ble_led_start(1, 1, 1);
		}
	}
}

static void led_process_cb(void *msg)
{
	uint32_t *evt = (uint32_t *)msg;

	tiny_set_event(&led_task, *evt);
}

static void led_device_init(void)
{
	tiny_timer_create(&led_task, led_task_cb, NULL);
    tiny_timer_start(&led_task, TIMER_FOREVER, 100);

    Topic_Subscrib(LED_TOPIC, led_process_cb);
}
app_initcall(led_device_init);


