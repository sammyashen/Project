#ifndef _INCLUDE_H
#define _INCLUDE_H

//esp head files
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_sleep.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "esp_http_client.h"
#include "esp_err.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_timer.h"


#include "lwip/err.h"
#include "lwip/sys.h"
#include "driver/gpio.h"
#include "driver/rmt.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "driver/uart.h"
#include "cJSON.h"


//RTOS head files
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

//Drives head files
#include "led_strip.h"
#include "soc/rtc.h"

//C/C++ Lib head files
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>


//User head files
#include "nvs_flash.h"
#include "nvs.h"
#include "user_http_client.h"
#include "ads1299.h"
#include "filter.h"


//Moduel head files
#include "mod_net.h"
#include "mod_ble.h"


extern const char *TAG;



#endif


