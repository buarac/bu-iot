#include "bu_espnow.h"


#define BU_ESPNOW_SEND_CB_OK_BIT    BIT0
#define BU_ESPNOW_SEND_CB_FAIL_BIT  BIT1
#define BU_ESPNOW_MAGIC_LEN         4
#define BU_ESPNOW_EVENT_QUEUE_SIZE  32


static const char *TAG                              = "bu_espnow";
static uint8_t bu_bcast_mac[ESP_NOW_ETH_ALEN]       = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
static EventGroupHandle_t bu_espnow_event_group     = NULL;
static xQueueHandle bu_espnow_event_queue           = NULL;
static const uint8_t bu_magic[BU_ESPNOW_MAGIC_LEN]  = { 0x55, 0x42, 0x4F, 0x52 };
static bool bu_espnow_init_flag                     = false;

typedef struct {
    uint8_t     magic[BU_ESPNOW_MAGIC_LEN];
    uint8_t     crc;
    uint32_t    handle;
    uint8_t     size;
    uint8_t     seq;
    uint8_t     payload[0];
} __attribute__((packed)) bu_espnow_head_data_t;

typedef struct {
    uint8_t                 addr[ESP_NOW_ETH_ALEN];
    uint8_t                 size;
    bu_espnow_head_data_t   data[0];
} bu_espnow_event_data_t;


esp_err_t bu_espnow_add_peer(const uint8_t *mac_addr) {
    ESP_LOGV(TAG, "bu_espnow_add_peer");

    esp_err_t ret;

    if ( esp_now_is_peer_exist(mac_addr) ) {
        ret = esp_now_del_peer(mac_addr);
        if ( ret != ESP_OK ) {
            ESP_LOGE(TAG, "esp_now_del_peer fail, ret: %d", ret);
            return ESP_OK;
        }
    }

    esp_now_peer_info_t peer;

    peer.channel    = BU_WIFI_CHANNEL;
    peer.encrypt    = false;
    peer.ifidx      = BU_WIFI_IF;
    memcpy(peer.peer_addr, mac_addr, ESP_NOW_ETH_ALEN);

    ret = esp_now_add_peer(&peer);
    BU_ERROR_CHECK (ret==ESP_OK, ret, "Add peer to peer list fail");

    return ESP_OK;
}

static void bu_espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
    ESP_LOGV(TAG, "bu_espnow_send_cb");

    BU_PARAM_CHECK(mac_addr);

    if ( status == ESP_NOW_SEND_SUCCESS ) {
        xEventGroupSetBits ( bu_espnow_event_group, BU_ESPNOW_SEND_CB_OK_BIT );
    }
    else {
        xEventGroupSetBits ( bu_espnow_event_group, BU_ESPNOW_SEND_CB_FAIL_BIT );
    }
}

static void bu_espnow_recv_cb(const uint8_t *mac_addr, const uint8_t* data, int len) {
    ESP_LOGV(TAG, "bu_espnow_recv_cb");

    BU_PARAM_CHECK(mac_addr);
    BU_PARAM_CHECK(data);
    BU_PARAM_CHECK((len > 0));

    //ESP_LOGI(TAG, "recv len = %d", len);
 //   bu_espnow_event_data_t *evt = (bu_espnow_event_data_t*)BU_ALLOC(sizeof(bu_espnow_event_data_t) + len);
    bu_espnow_event_data_t *evt = (bu_espnow_event_data_t*)calloc(1, (sizeof(bu_espnow_event_data_t) + len));
    if ( !evt ) {
        ESP_LOGW(TAG, "failed to alloc memory");
        return;
    }

    evt->size = len;
    memcpy ( evt->data, data, len);
    memcpy ( evt->addr, mac_addr, ESP_NOW_ETH_ALEN);

    if ( xQueueSend(bu_espnow_event_queue, &evt, 0) != pdPASS ) {
        ESP_LOGW(TAG, "send to event queue failed");
        BU_FREE(evt);
    }

}

void bu_espnow_print_head_data(bu_espnow_head_data_t *head) {
        ESP_LOGW(TAG, "magic  : %x%x%x%x", head->magic[0], head->magic[1], head->magic[2], head->magic[3] );
        ESP_LOGW(TAG, "handle : %zu", head->handle);
        ESP_LOGW(TAG, "crc    : %d ", head->crc); //, (%d)", head->crc, comp_crc);
        ESP_LOGW(TAG, "size   : %d", head->size);
        ESP_LOGW(TAG, "seq    : %d", head->seq);
        ESP_LOGW(TAG, "payload:");
        ESP_LOG_BUFFER_HEXDUMP(TAG, head->payload, head->size, ESP_LOG_WARN);
}

esp_err_t bu_espnow_parse_head_data(bu_espnow_head_data_t *head) {
    ESP_LOGV(TAG, "bu_espnow_parse_head_data");

    esp_err_t ret = ESP_OK;

/*
typedef struct {
    uint8_t     magic[BU_ESPNOW_MAGIC_LEN];
    uint8_t     crc;
    uint32_t    handle;
    uint8_t     size;
    uint8_t     seq;
    uint8_t     payload[0];
} __attribute__((packed)) bu_espnow_head_data_t;
*/

    if ( memcmp(head->magic, bu_magic, BU_ESPNOW_MAGIC_LEN) != 0 ) {
        ESP_LOGE(TAG, "invalid magic number: found %x%x%x%x, expected %x%x%x%x", 
            head->magic[0], head->magic[1], head->magic[2], head->magic[3] , 
            bu_magic[0], bu_magic[1], bu_magic[2], bu_magic[3] 
        );
        ret = ESP_FAIL;
        goto EXIT;
    }

    uint8_t crc = esp_crc8_le(UINT8_MAX, (uint8_t*)head->payload, head->size);
    if ( crc != head->crc ) {
        ESP_LOGE(TAG, "invalid crc: found %x, computed: %x", head->crc, crc);
        ret = ESP_FAIL;
        goto EXIT;
    }

EXIT:
    return ret;
}

void bu_espnow_event_task(void *pvParameter) {
    ESP_LOGV(TAG, "bu_espnow_event_task");

    bu_espnow_event_data_t *pEvt;

    while ( xQueueReceive(bu_espnow_event_queue, &pEvt, portMAX_DELAY) == pdTRUE ) {
        ESP_LOGW(TAG, ">>>>> New ESPNOW Event from "MACSTR"", MAC2STR(pEvt->addr));

        bu_espnow_head_data_t *head = (bu_espnow_head_data_t*)pEvt->data;
        bu_espnow_parse_head_data(head);
        bu_espnow_print_head_data(head);

        free(pEvt);

    }

}


esp_err_t bu_espnow_write(const uint8_t *dest_addr, const void *data, size_t size) {
    ESP_LOGV(TAG, "bu_espnow_write");

    if ( !data || (size<=0) ) {
        ESP_LOGE(TAG, "Write args error, addr: %p, data: %p, size: %d", dest_addr, data, size);
        return ESP_FAIL;
    }

    esp_err_t ret               = ESP_OK;
    bu_espnow_head_data_t *head = (bu_espnow_head_data_t*)calloc(1, sizeof(bu_espnow_head_data_t) + size);

    memcpy(head->magic, bu_magic, BU_ESPNOW_MAGIC_LEN);
    head->seq       = 0;
    head->handle    = esp_random();
    head->size      = size;
    head->seq       = 0;
    head->crc       = esp_crc8_le(UINT8_MAX, (uint8_t*)data, size);
    //head->crc       = 10;
    memcpy( head->payload, data, size);

    uint8_t *addr = dest_addr;

    if ( dest_addr == NULL ) {
        addr = bu_bcast_mac;
    }

    ESP_LOGW(TAG, "sending data to "MACSTR" for %d byte(s)", MAC2STR(addr), size);

    ret = esp_now_send(addr, (uint8_t*)head, sizeof(bu_espnow_head_data_t) + size);
    if ( ret != ESP_OK ) {
        ESP_LOGE(TAG, "esp_now_send fail: %d, %s", ret, esp_err_to_name(ret));
        goto EXIT;
    }

    EventBits_t uxBits = xEventGroupWaitBits(bu_espnow_event_group, BU_ESPNOW_SEND_CB_OK_BIT | BU_ESPNOW_SEND_CB_FAIL_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
    //ESP_LOGI(TAG, "uxBits = %d, 0b%b", uxBits, uxBits);
    if ((uxBits & BU_ESPNOW_SEND_CB_OK_BIT) != BU_ESPNOW_SEND_CB_OK_BIT) {
        ret = ESP_FAIL;
        ESP_LOGE(TAG, "Wait send ack fail");
        goto EXIT;
    }

EXIT: 
    free(head);
    return ESP_OK;
}

esp_err_t bu_espnow_init() {
    ESP_LOGV(TAG, "bu_espnow_init");

    if ( bu_espnow_init_flag ) {
        return ESP_OK;
    }

    // event group
    bu_espnow_event_group = xEventGroupCreate();
    BU_ERROR_CHECK(bu_espnow_event_group, ESP_FAIL, "create event group fail");

    // event queue & task
    bu_espnow_event_queue = xQueueCreate(BU_ESPNOW_EVENT_QUEUE_SIZE, sizeof(bu_espnow_event_data_t*));
    BU_ERROR_CHECK(bu_espnow_event_queue, ESP_FAIL, "create event queue fail");
    xTaskCreate(bu_espnow_event_task, "bu_espnow_event_task", 2048, NULL, 4, NULL);



    ESP_ERROR_CHECK ( esp_now_init() );
    ESP_ERROR_CHECK ( esp_now_register_send_cb(bu_espnow_send_cb));
    ESP_ERROR_CHECK ( esp_now_register_recv_cb(bu_espnow_recv_cb));
    ESP_ERROR_CHECK ( bu_espnow_add_peer(bu_bcast_mac));


    bu_espnow_init_flag = true;
    return ESP_OK;
}

esp_err_t bu_espnow_deinit() {
    ESP_LOGV(TAG, "bu_espnow_deinit");

    BU_ERROR_CHECK(!bu_espnow_init_flag, ESP_FAIL, "ESPNOW is not initialized");

    // TODO vider la file 
    vQueueDelete(bu_espnow_event_queue);
    bu_espnow_event_queue = NULL;

    vEventGroupDelete(bu_espnow_event_group);
    bu_espnow_event_group = NULL;

    ESP_ERROR_CHECK( esp_now_unregister_recv_cb());
    ESP_ERROR_CHECK( esp_now_unregister_send_cb());
    ESP_ERROR_CHECK( esp_now_deinit());

    bu_espnow_init_flag = false;

    return ESP_OK;
}

