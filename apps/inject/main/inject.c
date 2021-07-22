#include "nvs_flash.h"
#include "bu_espnow.h"
#include "bu_wifi.h"



static const char *TAG = "bu_inject";

#define BU_DELAY 10*1000

#define BU_GATEWAY_NODE_ID  0x80
#define BU_PAYLOAD_SIZE 96
#define BU_RANDOM_MASK 0b01111111

typedef struct {
    uint8_t     node_id;
    uint8_t     data[0];
} bu_inject_data_t;

void inject_task(void *pvParametre) {
    ESP_LOGV(TAG, "inject_task");

    while(1) {

        uint32_t rand_size = esp_random();
        rand_size = rand_size & BU_RANDOM_MASK;

        bu_inject_data_t *payload = (bu_inject_data_t*)calloc(1, sizeof(bu_inject_data_t) + rand_size);

        if ( payload != NULL ) {
            esp_fill_random(payload->data, rand_size);

            payload->node_id = BU_GATEWAY_NODE_ID;

            bu_espnow_write(NULL, payload, sizeof(bu_inject_data_t) + rand_size);
            free(payload);
        }
        vTaskDelay(BU_DELAY / portTICK_PERIOD_MS);
    }
}

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

    xTaskCreate(inject_task, "inject_task", 2048, NULL, 4, NULL);

}