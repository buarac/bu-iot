#include "nvs_flash.h"
#include "bu_espnow.h"
#include "bu_wifi.h"



static const char *TAG = "bu_gateway";

void app_main(void) {
    ESP_LOGV(TAG, "app_main");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );


    ESP_ERROR_CHECK(bu_wifi_init());
    ESP_ERROR_CHECK(bu_espnow_init());

}