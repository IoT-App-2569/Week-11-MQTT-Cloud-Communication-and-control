#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "mqtt_client.h"

#define WIFI_SSID           "Kirinya20_2.4G"
#define WIFI_PASS           "Kirinya200345"
#define MQTTS_BROKER_URI    "mqtts://192.168.1.36:8883" // พอร์ต 8883 ของเครื่อง Mac

static const char *TAG = "MQTTS_SECURE";
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT  BIT0

// สัญลักษณ์ Assembler ที่สร้างขึ้นโดยอัตโนมัติจากคำสั่ง EMBED_TXTFILES ใน CMake
extern const uint8_t ca_crt_start[] asm("_binary_ca_crt_start");
extern const uint8_t ca_crt_end[]   asm("_binary_ca_crt_end");

extern const uint8_t client_crt_start[] asm("_binary_client_crt_start");
extern const uint8_t client_crt_end[]   asm("_binary_client_crt_end");

extern const uint8_t client_key_start[] asm("_binary_client_key_start");
extern const uint8_t client_key_end[]   asm("_binary_client_key_end");

/* ========================================================
 * 1. ระบบจัดการ Wi-Fi
 * ======================================================== */
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

/* ========================================================
 * 2. MQTT Event Handler
 * ======================================================== */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTTS TLS Handshake Successful! Connected to Secure Broker.");
        
        // ส่งข้อความความปลอดภัยสูง
        esp_mqtt_client_publish(client, "secure/esp32/status", "SECURE_ONLINE_MTLS", 0, 1, 0);
        esp_mqtt_client_subscribe(client, "secure/esp32/cmd", 1);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "SECURE DATA RECEIVED:");
        ESP_LOGI(TAG, "TOPIC: %.*s", event->topic_len, event->topic);
        ESP_LOGI(TAG, "PAYLOAD: %.*s", event->data_len, event->data);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTTS Error Event occurred");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGE(TAG, "TLS / Transport Error Code: 0x%x", event->error_handle->esp_tls_last_esp_err);
        }
        break;

    default:
        break;
    }
}

/* ========================================================
 * 3. ฟังก์ชันหลัก (app_main)
 * ======================================================== */
void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    wifi_init_sta();
    ESP_LOGI(TAG, "Waiting for Wi-Fi connection...");
    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Wi-Fi Connected! Initializing Secure MQTTS Client with mTLS...");

    // กำหนดค่า MQTTS Client แบบ Mutual TLS
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                .uri = MQTTS_BROKER_URI,
            },
            .verification = {
                // ติดตั้ง CA Certificate เพื่อตรวจสอบ Broker
                .certificate = (const char *)ca_crt_start,
            },
        },
        .credentials = {
            .client_id = "esp32_mtls_client_node",
            .authentication = {
                // ติดตั้ง Client Certificate และ Private Key เพื่อยืนยันตัวตนกับ Broker
                .certificate = (const char *)client_crt_start,
                .key = (const char *)client_key_start,
            },
        },
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}
