#ifndef _BU_WIFI_H_
#define _BU_WIFI_H_

#include "bu_common.h"

#ifdef __cplusplus
extern "C" {
#endif


esp_err_t bu_wifi_init();
esp_err_t bu_wifi_deinit();

#ifdef __cplusplus
}
#endif

#endif // _BU_WIFI_H_