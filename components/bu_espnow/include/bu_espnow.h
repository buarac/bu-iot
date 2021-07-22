#ifndef _BU_ESPNOW_H_
#define _BU_ESPNOW_H_

#include "bu_common.h"

#ifdef __cplusplus
extern "C" {
#endif

//bu_espnow_write();
//bu_espnow_read();


esp_err_t bu_espnow_write(const uint8_t *dest_addr, const void *data, size_t size);
esp_err_t bu_espnow_init();
esp_err_t bu_espnow_deinit();

#ifdef __cplusplus
}
#endif

#endif // _BU_ESPNOW_H_