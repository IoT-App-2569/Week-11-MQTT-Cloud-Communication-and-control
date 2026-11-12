/**
 * ==============================================================================
 * Course: Week 11 - MQTT Cloud Communication and Control
 * Lab 11.3: MQTTS Security with TLS & Mutual Authentication (mTLS)
 * Student ID: 67030281
 * Description: Secure MQTTS Client with X.509 Mutual Authentication (Port 8883)
 *              Validates Server Certificate with embedded CA and authenticates
 *              to Broker using embedded Client Certificate and Private Key.
 * ==============================================================================
 */

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
#include "driver/gpio.h"
#include "mqtt_client.h"

// -----------------------------------------------------------------------------
// Network & Broker Configuration
// -----------------------------------------------------------------------------
#define WIFI_SSID           "AIS 4G Hi-Speed Home WiFi_769475"
#define WIFI_PASS           "50769475"
#define MQTTS_BROKER_URI    "mqtts://192.168.1.111:8883" // Secure MQTTS Port 8883

static const char *TAG = "LAB11_3_MQTTS";
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT  BIT0

// -----------------------------------------------------------------------------
// Assembler Symbols Generated Automatically by CMake EMBED_TXTFILES
// -----------------------------------------------------------------------------
extern const uint8_t ca_crt_start[]     asm("_binary_ca_crt_start");
extern const uint8_t ca_crt_end[]       asm("_binary_ca_crt_end");

extern const uint8_t client_crt_start[] asm("_binary_client_crt_start");
extern const uint8_t client_crt_end[]   asm("_binary_client_crt_end");

extern const uint8_t client_key_start[] asm("_binary_client_key_start");
extern const uint8_t client_key_end[]   asm("_binary_client_key_end");

/* ==============================================================================
 * 1. ระบบจัดการ Wi-Fi Station
 * ============================================================================== */
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Wi-Fi Station started, connecting to AP...");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Wi-Fi disconnected! Reconnecting...");
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Wi-Fi Connected! Got IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
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
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    strncpy((char *)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, WIFI_PASS, sizeof(wifi_config.sta.password) - 1);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Wi-Fi Station initialized.");
}

/* ==============================================================================
 * 2. MQTT Event Handler สำหรับจัดการการเชื่อมต่อ TLS / mTLS
 * ============================================================================== */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "=========================================================");
        ESP_LOGI(TAG, "[SUCCESS] MQTTS TLS Handshake Completed Successfully!");
        ESP_LOGI(TAG, "Mutual Authentication (mTLS) Verified with Secure Broker.");
        ESP_LOGI(TAG, "=========================================================");

        // ส่งข้อความความปลอดภัยสูงยืนยันสถานะ Online
        esp_mqtt_client_publish(client, "secure/esp32/status", "SECURE_ONLINE_MTLS", 0, 1, 0);
        ESP_LOGI(TAG, "Published to 'secure/esp32/status': SECURE_ONLINE_MTLS");

        // บอกรับหัวข้อคำสั่งปลอดภัย
        esp_mqtt_client_subscribe(client, "secure/esp32/cmd", 1);
        ESP_LOGI(TAG, "Subscribed to secure command topic: 'secure/esp32/cmd'");
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "MQTT_EVENT_DISCONNECTED: Connection closed by Broker or Network error.");
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "---------------------------------------------------------");
        ESP_LOGI(TAG, "ENCRYPTED DATA RECEIVED VIA TLS TUNNEL:");
        ESP_LOGI(TAG, "  TOPIC:   %.*s", event->topic_len, event->topic);
        ESP_LOGI(TAG, "  PAYLOAD: %.*s", event->data_len, event->data);
        ESP_LOGI(TAG, "---------------------------------------------------------");
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "[ERROR] MQTTS Event Error occurred!");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGE(TAG, "  TLS / Transport Error Code: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGE(TAG, "  Captured TLS Stack Error: 0x%x", event->error_handle->esp_tls_stack_err);
        }
        break;

    default:
        break;
    }
}

/* ==============================================================================
 * 3. ฟังก์ชันหลัก (app_main)
 * ============================================================================== */
void app_main(void)
{
    ESP_LOGI(TAG, "=======================================================");
    ESP_LOGI(TAG, "Starting Lab 11.3: MQTTS Client with mTLS (ID: 67030281)");
    ESP_LOGI(TAG, "Target Chip: %s | Protocol: MQTTS (TLS 1.2+)", CONFIG_IDF_TARGET);
    ESP_LOGI(TAG, "=======================================================");

    // เริ่มต้นระบบจัดเก็บ NVS Flash
    ESP_ERROR_CHECK(nvs_flash_init());

    // เริ่มต้นและรอการเชื่อมต่อ Wi-Fi
    wifi_init_sta();
    ESP_LOGI(TAG, "Waiting for Wi-Fi connection...");
    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Wi-Fi Ready! Preparing mTLS Credentials from Embedded Flash...");

    // ตรวจสอบขนาดของ Certificate ที่ถูกฝังในไบนารี
    size_t ca_len = ca_crt_end - ca_crt_start;
    size_t client_crt_len = client_crt_end - client_crt_start;
    size_t client_key_len = client_key_end - client_key_start;
    ESP_LOGI(TAG, "Embedded Cert Info: CA size=%d B, Client Cert=%d B, Client Key=%d B",
             (int)ca_len, (int)client_crt_len, (int)client_key_len);

    // กำหนดค่าคอนฟิกูเรชันสำหรับการเชื่อมต่อแบบ Mutual TLS
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                .uri = MQTTS_BROKER_URI, // mqtts://192.168.1.111:8883
            },
            .verification = {
                // ติดตั้ง CA Certificate เพื่อตรวจสอบความถูกต้องของ Broker
                .certificate = (const char *)ca_crt_start,
            },
        },
        .credentials = {
            .client_id = "esp32_mtls_client_node",
            .authentication = {
                // ติดตั้ง Client Certificate และ Private Key เพื่อยืนยันตัวตนกับ Broker (mTLS)
                .certificate = (const char *)client_crt_start,
                .key = (const char *)client_key_start,
            },
        },
    };

    ESP_LOGI(TAG, "Starting MQTT Client over TLS -> Connecting to %s", MQTTS_BROKER_URI);
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
    ESP_ERROR_CHECK(esp_mqtt_client_start(client));
}

