#ifndef _BU_COMMON_H_
#define _BU_COMMON_H_

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "esp_heap_task_info.h"

#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_now.h"
#include "esp_crc.h"
//#include "mqtt_client.h"
#include "driver/gpio.h"


#ifdef __cplusplus
extern "C" {
#endif /**< _cplusplus */


#define BU_WIFI_CHANNEL 1
#define BU_WIFI_IF ESP_IF_WIFI_STA
#define BU_WIFI_MODE WIFI_MODE_STA


#define BU_ERROR_CHECK(cond, err, format, ...) do { \
    if ( !cond ) { \
        if ( *format != '\0' ) { \
            ESP_LOGW(TAG, "<%s> " format, esp_err_to_name(err), ##__VA_ARGS__); \
        } \
        return err; \
    } \
} while(0)


#define BU_PARAM_CHECK(cond) do  { \
    if ( !cond ) { \
        ESP_LOGE(TAG, "Invalid args !(%s)", #cond); \
        return; \
    } \
} while(0) 

#define BU_PARAM_CHECK_RET(cond) do  { \
    if ( !cond ) { \
        ESP_LOGE(TAG, "Invalid args !(%s)", #cond); \
        return ESP_ERR_INVALID_ARGS; \
    } \
} while(0) 

#define BU_ALLOC(size) do { \
    void *ptr = calloc(1, size); \
    ptr; \
} while(0)

#define BU_FREE(ptr) do { \
    if ( ptr ) { \
        free(ptr); \
    } \
} while(0)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_BU_COMMON_H_