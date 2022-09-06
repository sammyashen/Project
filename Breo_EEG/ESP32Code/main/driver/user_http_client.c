#include "include.h"

//extern const char howsmyssl_com_root_cert_pem_start[] asm("_binary_howsmyssl_com_root_cert_pem_start");
//extern const char howsmyssl_com_root_cert_pem_end[]   asm("_binary_howsmyssl_com_root_cert_pem_end");

static void http_client_test(void)//阻塞函数
{
	esp_err_t err = ESP_OK;
	char local_response_buffer[512] = {0};
	int content_length = 0;
	char strbuf[30] = {0};

	esp_http_client_config_t config = {
	    .url = "https://api.seniverse.com/v3/weather/now.json?key=Ssx8WamRg9pmJs3zC&location=shenzhen&language=zh-Hans&unit=c",
	};
	esp_http_client_handle_t client = esp_http_client_init(&config);

//get temperature
	esp_http_client_set_method(client, HTTP_METHOD_GET);
	esp_http_client_set_header(client, "content-type", "multipart/form-data");
	esp_http_client_set_timeout_ms(client, 1000);
    err = esp_http_client_open(client, 0);
    if(err == ESP_OK)
    {
    	content_length = esp_http_client_fetch_headers(client);
    	if(content_length >= 0)
    	{
    		int data_read = esp_http_client_read_response(client, local_response_buffer, sizeof(local_response_buffer));
    		if(data_read >= 0 && data_read < sizeof(local_response_buffer))	
    		{
    			printf("get temperature successed, status code:%d, content length:%d\r\n",
    					esp_http_client_get_status_code(client),
    					esp_http_client_get_content_length(client));

    			cJSON *temp_root = cJSON_Parse(local_response_buffer);
				if(temp_root != NULL)
				{
					cJSON *results = cJSON_GetObjectItem(temp_root, "results");
					cJSON *arrayzero = cJSON_GetArrayItem(results, 0);
					cJSON *now = cJSON_GetObjectItem(arrayzero, "now");
					cJSON *temp = cJSON_GetObjectItem(now, "temperature");
					strcpy(strbuf, temp->valuestring);
					cJSON_Delete(temp_root);
					printf("temperature:%s\r\n", strbuf);
				}
    		}
    		else
    		{
    			ESP_LOGE(TAG, "get temperature failed!");
    		}
    	}
    	else	ESP_LOGE(TAG, "get temperature, content-length is low than zero!");
    }
    else	ESP_LOGE(TAG, "get temperature, can't open client!");
	esp_http_client_close(client);
	memset(strbuf, 0, sizeof(strbuf));
	memset(local_response_buffer, 0, sizeof(local_response_buffer));

//get time
	esp_http_client_set_url(client, "http://quan.suning.com/getSysTime.do");
	err = esp_http_client_open(client, 0);
	if(err == ESP_OK)
	{
		content_length = esp_http_client_fetch_headers(client);
		if(content_length >= 0)
    	{
    		int data_read = esp_http_client_read_response(client, local_response_buffer, sizeof(local_response_buffer));
    		if(data_read >= 0 && data_read < sizeof(local_response_buffer))	
    		{
    			printf("get time successed, status code:%d, content length:%d\r\n",
    					esp_http_client_get_status_code(client),
    					esp_http_client_get_content_length(client));

    			cJSON *gettime_root = cJSON_Parse(local_response_buffer);
				if(gettime_root != NULL)
				{
					cJSON *time = cJSON_GetObjectItem(gettime_root, "sysTime2");
					strcpy(strbuf, time->valuestring);
					cJSON_Delete(gettime_root);
					printf("systime:%s\r\n", strbuf);
				}
    		}
    		else
    		{
    			ESP_LOGE(TAG, "get time failed!");
    		}
    	}
    	else	ESP_LOGE(TAG, "get time, content-length is low than zero!");
	}
	else	ESP_LOGE(TAG, "get time, can't open client!");
	esp_http_client_close(client);

//post:参考esp_http_client_example.c/http_native_request 
	
    esp_http_client_cleanup(client);

/************************处理完数据后进入深度睡眠*******************************************/
//	ESP_LOGI(TAG, "Entering deep sleep\n");
//	esp_wifi_stop();
//	esp_bluedroid_disable();
//	esp_bt_controller_disable();
//	esp_deep_sleep_start();
/*******************************************************************************************/    
}

void http_client_task(void *pvParameters)
{
	http_client_test();
	vTaskDelete(NULL);
}


