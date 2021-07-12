#include "bu_espnow.h"


static const char *TAG = "bu_espnow";
static uint8_t bcast_mac[ESP_NOW_ETH_ALEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };


esp_err_t bu_espnow_add_peer(const uint8_t *mac_addr) {
    ESP_LOGV(TAG, "adding peer: "MACSTR"", MAC2STR(mac_addr));

    if ( !esp_now_is_peer_exist(mac_addr) ) {
        esp_now_peer_info_t* peer = calloc(1, sizeof(esp_now_peer_info_t));
        if ( peer == NULL ) {
            ESP_LOGW(TAG, "calloc peer fail");
            return ESP_FAIL;
        }
        peer->channel = BU_WIFI_CHANNEL;
        peer->ifidx = BU_WIFI_IF;
        peer->encrypt = false;
        memcpy(peer->peer_addr, mac_addr, ESP_NOW_ETH_ALEN);
        ESP_ERROR_CHECK( esp_now_add_peer(peer));
        free(peer);
        ESP_LOGI(TAG, "New peer "MACSTR" added", MAC2STR(mac_addr));
    }
    return ESP_OK;
}

static void bu_espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
    ESP_LOGV(TAG, "bu_espnow_send_cb");
}

static void bu_espnow_recv_cb(const uint8_t *mac_addr, const uint8_t* data, int len) {
    ESP_LOGV(TAG, "bu_espnow_recv_cb");
}

esp_err_t bu_espnow_init() {
    ESP_LOGV(TAG, "bu_espnow_init");

    ESP_ERROR_CHECK ( esp_now_init() );
    ESP_ERROR_CHECK ( esp_now_register_send_cb(bu_espnow_send_cb));
    ESP_ERROR_CHECK ( esp_now_register_recv_cb(bu_espnow_recv_cb));
    ESP_ERROR_CHECK ( bu_espnow_add_peer(bcast_mac));

    return ESP_OK;
}

esp_err_t bu_espnow_deinit() {
    return ESP_OK;
}

