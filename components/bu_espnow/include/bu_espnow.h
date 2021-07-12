#ifndef _BU_ESPNOW_H_
#define _BU_ESPNOW_H_

#include "bu_common.h"

#ifdef __cplusplus
extern "C" {
#endif

//bu_espnow_write();
//bu_espnow_read();

#define BU_WIFI_CHANNEL 1
#define BU_WIFI_IF      ESP_IF_WIFI_STA
#define BU_WIFI_MODE    WIFI_MODE_STA

esp_err_t bu_espnow_init();
esp_err_t bu_espnow_deinit();

#ifdef __cplusplus
}
#endif

#endif // _BU_ESPNOW_H_