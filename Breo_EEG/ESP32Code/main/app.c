/* 
*	Breo_EEG Project
*/
#include "include.h"

const char *TAG = "BREO_EEG";

void app_main(void)
{
/****************************Set Deep_Sleep*****************************************/
    switch(esp_sleep_get_wakeup_cause())
    {
    	case ESP_SLEEP_WAKEUP_TIMER: 
    	{
            ESP_LOGI(TAG, "Wake up from timer.");
            break;
        }

        default:
            ESP_LOGI(TAG, "Not wake up from timer.");
    }

    ESP_LOGI(TAG, "Enabling timer wakeup, %ds\n", CONFIG_WAKE_UP_TIME_SEC);
    esp_sleep_enable_timer_wakeup(CONFIG_WAKE_UP_TIME_SEC * 1000000);
/***********************************************************************************/    
    ESP_LOGI(TAG, "start EEG project!");

	//initialize NVS
	esp_err_t ret = nvs_flash_init();
	if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	//init spi
	ADS1299_Init();
	
	//init wifi
	wifi_init_sta();
	//init ble
  	ble_init();
  	//http get date test
	xTaskCreate(&http_client_task, "http_test_task", (4*1024), NULL, 7, NULL);

/************************处理完数据后进入深度睡眠*******************************************/
//	ESP_LOGI(TAG, "Entering deep sleep\n");
//	esp_wifi_stop();
//	esp_bluedroid_disable();
//	esp_bt_controller_disable();
//	esp_deep_sleep_start();
/*******************************************************************************************/

  	return;
}
