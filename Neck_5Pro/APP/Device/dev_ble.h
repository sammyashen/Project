#ifndef _DEV_BLE_H
#define _DEV_BLE_H

#include "n32g45x.h"
#include "rtthread.h"
#include "init.h"
#include "esp_protocol_cfg.h"
#include "app_protocol_cfg.h"

typedef enum{
	BLE_DISCONNECT = 0,
	BLE_CONNECTING,
	BLE_CONNECTED,
}eBLE_STA;

typedef enum{
	ESP_SET_BLE_NAME = 0,
	ESP_GET_BLE_MAC,
	ESP_SET_ADV,
}eESP_SETTINGS;

typedef enum{
	ESP_PWR_UP = 0,
	ESP_HANDSHAKE,
	ESP_SETTINGS,
	ESP_BLE_TURN_ON,
	ESP_BLE_DATA_PROCESS,
	ESP_PWR_DOWN,
}eESP_STA;

typedef struct{
	eESP_SETTINGS esp_settings_sta;
	eESP_STA esp_opt_sta;
}esp_optseq_t;

typedef enum{
	ESP_ACTION_RECV = 0,
	ESP_ACTION_SEND,
}eESP_ACTION;

typedef struct{
	eESP_ACTION neck_5pro_esp_action;
	eBLE_STA neck_5pro_ble_sta;
	esp_optseq_t neck_5pro_esp_optseq;
	breo_esp_protocol_t breo_esp_protocol_struct;
	breo_app_protocol_t breo_app_protocol_struct;
}neck_5pro_ble_t;

void ble_create_thread(void);
void ble_sftimer_init(void);


#endif


