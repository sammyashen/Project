#include "include.h"

static char s_wifissid[32],s_wifipassword[64];
static EventGroupHandle_t s_wifi_event_group;

/* 将wifi信息存储进nvs，当收到蓝牙wifi配网信息时调用，不能频繁调用 */
static esp_err_t SetWIFIParaToNVS(const char *ssid, const char *password)
{
	esp_err_t err;
	nvs_handle_t wifipara_handle;

	err = nvs_open("wifipara", NVS_READWRITE, &wifipara_handle);
	if(err != ESP_OK)
	{
		ESP_LOGI(TAG, "can't open nvs handle");
		goto exit;
	}
	err = nvs_set_str(wifipara_handle, "wifissid", ssid);
	if(err != ESP_OK)
	{
		ESP_LOGI(TAG, "can't set ssid");
		goto exit;
	}
	err = nvs_set_str(wifipara_handle, "wifipassword", password);
	if(err != ESP_OK)
	{
		ESP_LOGI(TAG, "can't set password");
		goto exit;
	}
	err = nvs_commit(wifipara_handle);
	if(err != ESP_OK)
	{
		ESP_LOGI(TAG, "can't commit nvs_handle");
		goto exit;
	}
	nvs_close(wifipara_handle);

	return ESP_OK;

exit:
	return ESP_FAIL;
}

/* get wifi connect info from nvs */
static esp_err_t GetWIFIParaFromNVS(void)
{
	esp_err_t err;
	nvs_handle_t wifipara_handle;
	size_t len;

	//open handle
	err = nvs_open("wifipara", NVS_READWRITE, &wifipara_handle);
	if(err != ESP_OK)
	{
		ESP_LOGI(TAG, "can't open nvs_handle");
		goto exit;
	}
	//get ssid & password
	len = sizeof(s_wifissid);
	err = nvs_get_str(wifipara_handle, "wifissid", s_wifissid, &len);
	if(err != ESP_OK)
	{
		ESP_LOGI(TAG, "can't get ssid");
		goto exit;
	}
	len = sizeof(s_wifipassword);
	err = nvs_get_str(wifipara_handle, "wifipassword", s_wifipassword, &len);
	if(err != ESP_OK)
	{
		ESP_LOGI(TAG, "can't get password");
		goto exit;
	}
	//commit handle
	err = nvs_commit(wifipara_handle);
	if(err != ESP_OK)
	{
		ESP_LOGI(TAG, "can't commit nvs_handle");
		goto exit;
	}
	//close handle
	nvs_close(wifipara_handle);
	
	return ESP_OK;

exit:
	return ESP_FAIL;
}

/* WIFI状态事件处理回调函数 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	static unsigned int s_retry_num = 0;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) 
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } 
        else 
        {
            esp_wifi_disconnect();
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGI(TAG,"connect to the AP fail");
        }
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
    	ESP_LOGI(TAG,"connect to the AP successed");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/* WFIFI初始化函数 */
void wifi_init_sta(void)//阻塞函数
{
	esp_err_t err;

	s_wifi_event_group = xEventGroupCreate();

	ESP_ERROR_CHECK(esp_netif_init());
	
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	/* 将事件处理函数注册到默认轮询中 */
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
//            .ssid = s_wifissid,
//            .password = s_wifipassword,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
	     	.threshold.authmode = WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };

    err = GetWIFIParaFromNVS();
    if(err != ESP_OK)
    {
    	sprintf(s_wifissid, DEFAULT_WIFI_SSID);
    	sprintf(s_wifipassword, DEFAULT_WIFI_PASSWORD);

    	ESP_LOGI(TAG, "get wifi para from nvs failed!");
    }
    else
    {
    	ESP_LOGI(TAG, "get wifi para from nvs successed!");
    }
    ESP_LOGI(TAG, "ssid:%s,password:%s", s_wifissid, s_wifipassword);
	memcpy(wifi_config.sta.ssid, s_wifissid, sizeof(wifi_config.sta.ssid));
	memcpy(wifi_config.sta.password, s_wifipassword, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );
    ESP_LOGI(TAG, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 s_wifissid, s_wifipassword);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 s_wifissid, s_wifipassword);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group); 
}


