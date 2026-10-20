/*
 * ใบงาน 11.2 — ESP32 MQTT Client: Telemetry (Potentiometer -> JSON) + รับคำสั่งควบคุม LED
 * รหัสนักศึกษา 67030011
 *
 * Topic ที่ใช้
 *   iot/node01/status      : "online" (retain, QoS1) / LWT "offline" (retain, QoS1)
 *   iot/node01/telemetry   : JSON ทุก 4 วินาที (QoS0)
 *   iot/node01/cmd/led     : รับคำสั่ง "ON" / "OFF" / {"state": 1|0} (subscribe QoS1)
 *   iot/node01/state/led   : ตอบกลับสถานะ LED หลังทำคำสั่ง (QoS1)
 */
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>
#include <math.h>
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
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "mqtt_client.h"
#include "cJSON.h"
#include "wifi_credentials.h"   // CONFIG_WIFI_SSID / CONFIG_WIFI_PASSWORD (ไฟล์นี้อยู่ใน .gitignore)

#define MQTT_BROKER_URI     "mqtt://172.20.10.2:1883" // IP ของ Mac ที่รัน Mosquitto (อยู่ Hotspot วงเดียวกับบอร์ด)
#define NODE_ID             "esp32-node01"
#define STUDENT_ID          "67030011"

#define TOPIC_STATUS        "iot/node01/status"
#define TOPIC_TELEMETRY     "iot/node01/telemetry"
#define TOPIC_CMD_LED       "iot/node01/cmd/led"
#define TOPIC_STATE_LED     "iot/node01/state/led"

#if CONFIG_IDF_TARGET_ESP32C3
#define LED_PIN             GPIO_NUM_8
#define ADC_CHANNEL         ADC_CHANNEL_0   // GPIO 0
#else
#define LED_PIN             GPIO_NUM_2      // LED บนบอร์ด ESP32 DevKit
#define ADC_CHANNEL         ADC_CHANNEL_6   // GPIO 34
#endif
#define ADC_ATTEN           ADC_ATTEN_DB_12
#define ADC_SAMPLES         8               // เฉลี่ย 8 ครั้งต่อรอบเพื่อลด Noise

static const char *TAG = "LAB11_MQTT";
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT  BIT0

static esp_mqtt_client_handle_t s_mqtt_client = NULL;
static volatile bool s_mqtt_connected = false;
static adc_oneshot_unit_handle_t s_adc_handle = NULL;
static adc_cali_handle_t s_adc_cali = NULL;
static volatile bool s_led_state = false;

/* ========================================================
 * 1. ระบบเชื่อมต่อ Wi-Fi Station
 * ======================================================== */
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *d = (wifi_event_sta_disconnected_t *)event_data;
        ESP_LOGW(TAG, "Wi-Fi disconnected (reason %d), reconnecting...", d->reason);
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
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
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,   // รองรับ Hotspot ที่เปิด WPA2/WPA3
        },
    };
    size_t ssid_len = strlen(CONFIG_WIFI_SSID);
    if (ssid_len > sizeof(wifi_config.sta.ssid)) ssid_len = sizeof(wifi_config.sta.ssid);
    memcpy(wifi_config.sta.ssid, CONFIG_WIFI_SSID, ssid_len);

    size_t pass_len = strlen(CONFIG_WIFI_PASSWORD);
    if (pass_len > sizeof(wifi_config.sta.password)) pass_len = sizeof(wifi_config.sta.password);
    memcpy(wifi_config.sta.password, CONFIG_WIFI_PASSWORD, pass_len);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Wi-Fi Initialization finished. SSID: %s", CONFIG_WIFI_SSID);
}

/* ========================================================
 * 2. ควบคุม LED และถอดรหัสคำสั่ง
 * ======================================================== */
static void led_set(bool on)
{
    s_led_state = on;
    gpio_set_level(LED_PIN, on ? 1 : 0);
    ESP_LOGI(TAG, "LED State set to -> %s", on ? "ON" : "OFF");
}

// คืนค่า 1 = ON, 0 = OFF, -1 = คำสั่งไม่ถูกต้อง
static int parse_led_command(const char *payload, int len)
{
    char buf[64] = {0};
    int n = len < (int)sizeof(buf) - 1 ? len : (int)sizeof(buf) - 1;
    memcpy(buf, payload, n);

    if (strcasecmp(buf, "ON") == 0)  return 1;
    if (strcasecmp(buf, "OFF") == 0) return 0;

    // JSON {"state": 1} / {"state": 0} / {"state": true}
    int result = -1;
    cJSON *root = cJSON_ParseWithLength(buf, n);
    if (root != NULL) {
        cJSON *state = cJSON_GetObjectItemCaseSensitive(root, "state");
        if (cJSON_IsNumber(state)) {
            result = state->valueint ? 1 : 0;
        } else if (cJSON_IsBool(state)) {
            result = cJSON_IsTrue(state) ? 1 : 0;
        }
        cJSON_Delete(root);
    }
    return result;
}

/* ========================================================
 * 3. การจัดการ MQTT Event Handler
 * ======================================================== */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        s_mqtt_connected = true;
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED: Connected to Broker!");

        // ส่งข้อความสถานะ Online ทันทีที่เชื่อมต่อ (QoS = 1, Retain = 1) ทับค่า LWT "offline" ที่อาจค้างอยู่
        esp_mqtt_client_publish(client, TOPIC_STATUS, "online", 0, 1, 1);

        // บอกรับหัวข้อคำสั่งควบคุม LED
        esp_mqtt_client_subscribe(client, TOPIC_CMD_LED, 1);
        ESP_LOGI(TAG, "Subscribed to: %s", TOPIC_CMD_LED);
        break;

    case MQTT_EVENT_DISCONNECTED:
        s_mqtt_connected = false;
        ESP_LOGW(TAG, "MQTT_EVENT_DISCONNECTED (esp-mqtt จะเชื่อมต่อใหม่เองอัตโนมัติ)");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA received:");
        ESP_LOGI(TAG, "TOPIC=%.*s", event->topic_len, event->topic);
        ESP_LOGI(TAG, "DATA=%.*s", event->data_len, event->data);

        // เทียบความยาวด้วย ไม่ให้ Topic ที่สั้นกว่า (เช่น "iot/node01/cmd") ผ่านการตรวจแบบ prefix
        if (event->topic_len == (int)strlen(TOPIC_CMD_LED) &&
            strncmp(event->topic, TOPIC_CMD_LED, event->topic_len) == 0) {
            int cmd = parse_led_command(event->data, event->data_len);
            if (cmd < 0) {
                ESP_LOGW(TAG, "Unknown LED command, state unchanged");
            } else {
                led_set(cmd == 1);
            }

            // ตอบกลับสถานะปัจจุบัน (Acknowledge State)
            char ack_msg[64];
            snprintf(ack_msg, sizeof(ack_msg), "{\"led_state\": %s}", s_led_state ? "true" : "false");
            esp_mqtt_client_publish(client, TOPIC_STATE_LED, ack_msg, 0, 1, 0);
        }
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGE(TAG, "Transport error: esp_err=0x%x, socket errno=%d",
                     event->error_handle->esp_tls_last_esp_err,
                     event->error_handle->esp_transport_sock_errno);
        } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            ESP_LOGE(TAG, "Connection refused, return code=0x%x", event->error_handle->connect_return_code);
        }
        break;

    default:
        break;
    }
}

/* ========================================================
 * 4. ADC + Calibration (แปลงค่า raw เป็น mV จาก eFuse ของชิป)
 * ======================================================== */
static void adc_init(void)
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &s_adc_handle));

    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc_handle, ADC_CHANNEL, &chan_config));

    esp_err_t ret = ESP_ERR_NOT_SUPPORTED;
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .chan = ADC_CHANNEL,
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret = adc_cali_create_scheme_curve_fitting(&cali_config, &s_adc_cali);
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret = adc_cali_create_scheme_line_fitting(&cali_config, &s_adc_cali);
#endif
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "ADC calibration enabled");
    } else {
        s_adc_cali = NULL;
        ESP_LOGW(TAG, "ADC calibration not available, using raw/4095*3.3 V");
    }
}

/* ========================================================
 * 5. FreeRTOS Task: ส่ง Telemetry เป็นระยะ
 * ======================================================== */
static void telemetry_task(void *pvParameters)
{
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(4000)); // ส่งข้อมูลทุก 4 วินาที

        if (s_mqtt_client == NULL || !s_mqtt_connected) {
            ESP_LOGW(TAG, "MQTT not connected, skip telemetry");
            continue;
        }

        int sum = 0;
        for (int i = 0; i < ADC_SAMPLES; i++) {
            int raw = 0;
            adc_oneshot_read(s_adc_handle, ADC_CHANNEL, &raw);
            sum += raw;
        }
        int adc_raw = sum / ADC_SAMPLES;

        int voltage_mv = 0;
        if (s_adc_cali == NULL || adc_cali_raw_to_voltage(s_adc_cali, adc_raw, &voltage_mv) != ESP_OK) {
            voltage_mv = (int)lround(adc_raw * 3300.0 / 4095.0);
        }
        double voltage_v = voltage_mv / 1000.0;

        // สร้าง JSON Payload
        cJSON *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "node_id", NODE_ID);
        cJSON_AddStringToObject(root, "student_id", STUDENT_ID);
        cJSON_AddNumberToObject(root, "adc_raw", adc_raw);
        cJSON_AddNumberToObject(root, "voltage_v", voltage_v);
        cJSON_AddBoolToObject(root, "led_state", s_led_state);
        cJSON_AddNumberToObject(root, "uptime_ms", esp_log_timestamp());

        char *json_str = cJSON_PrintUnformatted(root);
        if (json_str != NULL) {
            esp_mqtt_client_publish(s_mqtt_client, TOPIC_TELEMETRY, json_str, 0, 0, 0);
            ESP_LOGI(TAG, "Telemetry Published: %s", json_str);
            free(json_str);          // cJSON_PrintUnformatted จองด้วย malloc
        }
        cJSON_Delete(root);          // คืนหน่วยความจำของทุก Node ใน JSON Tree
    }
}

/* ========================================================
 * 6. ฟังก์ชันหลัก (app_main)
 * ======================================================== */
void app_main(void)
{
    // กำหนดค่า NVS Flash
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // กำหนดค่า GPIO สำหรับ LED
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);

    // กำหนดค่า ADC สำหรับอ่าน Potentiometer
    adc_init();

    // เริ่มการเชื่อมต่อ Wi-Fi และรอจนกว่าจะได้ IP
    wifi_init_sta();
    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);

    // ตั้งค่า MQTT Client
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                .uri = MQTT_BROKER_URI,
            },
        },
        .credentials = {
            .client_id = NODE_ID,
        },
        .session = {
            .keepalive = 60,
            .last_will = {
                .topic = TOPIC_STATUS,
                .msg = "offline",
                .msg_len = 7,
                .qos = 1,
                .retain = 1,
            },
        },
    };

    s_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(s_mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(s_mqtt_client);
    ESP_LOGI(TAG, "MQTT client started, broker: %s", MQTT_BROKER_URI);

    // สร้าง Telemetry Task
    xTaskCreate(telemetry_task, "telemetry_task", 4096, NULL, 5, NULL);
}
