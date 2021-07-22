#include "bu_wifi.h"



static const char *TAG                              = "bu_wifi";


esp_err_t bu_wifi_init() {
    ESP_LOGV(TAG, "bu_wifi_init");

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(BU_WIFI_MODE) );
    ESP_ERROR_CHECK( esp_wifi_start());

    return ESP_OK;
}

esp_err_t bu_wifi_deinit() {
    ESP_LOGV(TAG, "bu_wifi_deinit");

    return ESP_OK;
}