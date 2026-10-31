
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <strings.h>
#include <stdlib.h>

#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

#include "mqtt_client.h"
#include "cJSON.h"

/* ========================================================
 * Configuration
 * ======================================================== */

#define WIFI_SSID           "iPhoneSayparn"
#define WIFI_PASS           "0641474746"

#define MQTT_BROKER_URI     "mqtt://172.20.10.3:1883"

#define MQTT_TOPIC_STATUS   "iot/node01/status"
#define MQTT_TOPIC_TELEMETRY "iot/node01/telemetry"
#define MQTT_TOPIC_LED_CMD  "iot/node01/cmd/led"
#define MQTT_TOPIC_LED_STATE "iot/node01/state/led"

#if CONFIG_IDF_TARGET_ESP32C3
    #define LED_PIN         GPIO_NUM_8
    #define ADC_CHANNEL     ADC_CHANNEL_0
#else
    #define LED_PIN         GPIO_NUM_2
    #define ADC_CHANNEL     ADC_CHANNEL_6
#endif

#define WIFI_CONNECTED_BIT  BIT0

static const char *TAG = "LAB11_MQTT";

static EventGroupHandle_t s_wifi_event_group;

static esp_mqtt_client_handle_t s_mqtt_client = NULL;
static adc_oneshot_unit_handle_t s_adc_handle = NULL;

static bool s_led_state = false;

/* ========================================================
 * 1. Wi-Fi Event Handler
 * ======================================================== */

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data)
{
    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_START) {

        esp_wifi_connect();

    } else if (event_base == WIFI_EVENT &&
               event_id == WIFI_EVENT_STA_DISCONNECTED) {

        ESP_LOGW(TAG, "Wi-Fi disconnected, reconnecting...");

        xEventGroupClearBits(
            s_wifi_event_group,
            WIFI_CONNECTED_BIT
        );

        esp_wifi_connect();

    } else if (event_base == IP_EVENT &&
               event_id == IP_EVENT_STA_GOT_IP) {

        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *)event_data;

        ESP_LOGI(
            TAG,
            "Got IP Address: " IPSTR,
            IP2STR(&event->ip_info.ip)
        );

        xEventGroupSetBits(
            s_wifi_event_group,
            WIFI_CONNECTED_BIT
        );
    }
}

/* ========================================================
 * 2. Initialize Wi-Fi Station
 * ======================================================== */

static void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL,
            &instance_any_id
        )
    );

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            NULL,
            &instance_got_ip
        )
    );

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    size_t ssid_len = strlen(WIFI_SSID);

    if (ssid_len > sizeof(wifi_config.sta.ssid)) {
        ssid_len = sizeof(wifi_config.sta.ssid);
    }

    memcpy(
        wifi_config.sta.ssid,
        WIFI_SSID,
        ssid_len
    );

    size_t pass_len = strlen(WIFI_PASS);

    if (pass_len > sizeof(wifi_config.sta.password)) {
        pass_len = sizeof(wifi_config.sta.password);
    }

    memcpy(
        wifi_config.sta.password,
        WIFI_PASS,
        pass_len
    );

    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_STA)
    );

    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifi_config
        )
    );

    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi Initialization finished.");
}

/* ========================================================
 * 3. MQTT Event Handler
 * ======================================================== */

static void mqtt_event_handler(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data)
{
    esp_mqtt_event_handle_t event =
        (esp_mqtt_event_handle_t)event_data;

    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {

    case MQTT_EVENT_CONNECTED:

        ESP_LOGI(
            TAG,
            "MQTT_EVENT_CONNECTED: Connected to Broker!"
        );

        esp_mqtt_client_publish(
            client,
            MQTT_TOPIC_STATUS,
            "online",
            0,
            1,
            1
        );

        esp_mqtt_client_subscribe(
            client,
            MQTT_TOPIC_LED_CMD,
            1
        );

        ESP_LOGI(
            TAG,
            "Subscribed to: %s",
            MQTT_TOPIC_LED_CMD
        );

        break;

    case MQTT_EVENT_DISCONNECTED:

        ESP_LOGW(TAG, "MQTT_EVENT_DISCONNECTED");

        break;

    case MQTT_EVENT_DATA: {

        ESP_LOGI(TAG, "MQTT_EVENT_DATA received:");

        ESP_LOGI(
            TAG,
            "TOPIC=%.*s",
            event->topic_len,
            event->topic
        );

        ESP_LOGI(
            TAG,
            "DATA=%.*s",
            event->data_len,
            event->data
        );

        /* ตรวจสอบ Topic ให้ตรงกัน */
        if (event->topic_len != strlen(MQTT_TOPIC_LED_CMD) ||
            strncmp(
                event->topic,
                MQTT_TOPIC_LED_CMD,
                event->topic_len
            ) != 0) {

            break;
        }

        /* คัดลอกข้อมูล MQTT ลง Buffer */
        char buf[128] = {0};

        int len = event->data_len;

        if (len >= sizeof(buf)) {
            len = sizeof(buf) - 1;
        }

        memcpy(buf, event->data, len);
        buf[len] = '\0';

        bool new_state = s_led_state;
        bool valid_command = false;

        /* รองรับข้อความ ON / OFF */
        if (strcasecmp(buf, "ON") == 0) {

            new_state = true;
            valid_command = true;

        } else if (strcasecmp(buf, "OFF") == 0) {

            new_state = false;
            valid_command = true;

        } else {

            /*
             * รองรับ JSON:
             * {"state":true}
             * {"state":false}
             * {"state":1}
             * {"state":0}
             */

            cJSON *root = cJSON_Parse(buf);

            if (root != NULL) {

                cJSON *state =
                    cJSON_GetObjectItemCaseSensitive(
                        root,
                        "state"
                    );

                if (cJSON_IsTrue(state)) {

                    new_state = true;
                    valid_command = true;

                } else if (cJSON_IsFalse(state)) {

                    new_state = false;
                    valid_command = true;

                } else if (
                    cJSON_IsNumber(state) &&
                    (state->valuedouble == 0.0 ||
                     state->valuedouble == 1.0)
                ) {

                    new_state = (state->valuedouble == 1.0);
                    valid_command = true;
                }

                cJSON_Delete(root);
            }
        }

        if (valid_command) {

            esp_err_t gpio_result = gpio_set_level(
                LED_PIN,
                new_state ? 1 : 0
            );

            if (gpio_result == ESP_OK) {

                s_led_state = new_state;

                ESP_LOGI(
                    TAG,
                    "LED State set to -> %s",
                    s_led_state ? "ON" : "OFF"
                );

            } else {

                ESP_LOGE(
                    TAG,
                    "Failed to set LED GPIO: %s",
                    esp_err_to_name(gpio_result)
                );
            }

        } else {

            ESP_LOGW(
                TAG,
                "Invalid LED command: %s",
                buf
            );
        }

        /* ส่งสถานะ LED กลับไปยัง Broker */
        char ack_msg[64];

        snprintf(
            ack_msg,
            sizeof(ack_msg),
            "{\"led_state\":%s}",
            s_led_state ? "true" : "false"
        );

        esp_mqtt_client_publish(
            client,
            MQTT_TOPIC_LED_STATE,
            ack_msg,
            0,
            1,
            0
        );

        break;
    }

    default:
        break;
    }
}

/* ========================================================
 * 4. Telemetry Task
 * ======================================================== */

static void telemetry_task(void *pvParameters)
{
    while (1) {

        vTaskDelay(pdMS_TO_TICKS(4000));

        if (s_mqtt_client == NULL) {
            continue;
        }

        /* อ่านค่า ADC */
        int adc_raw = 0;

        if (s_adc_handle != NULL) {

            esp_err_t adc_result = adc_oneshot_read(
                s_adc_handle,
                ADC_CHANNEL,
                &adc_raw
            );

            if (adc_result != ESP_OK) {

                ESP_LOGW(
                    TAG,
                    "ADC read failed: %s",
                    esp_err_to_name(adc_result)
                );
            }
        }

        /* คำนวณแรงดันโดยประมาณ */
        float voltage =
            ((float)adc_raw / 4095.0f) * 3.3f;

        /* สร้าง JSON Telemetry */
        cJSON *root = cJSON_CreateObject();

        if (root == NULL) {
            ESP_LOGE(TAG, "Failed to create telemetry JSON");
            continue;
        }

        cJSON_AddStringToObject(
            root,
            "node_id",
            "esp32-node01"
        );

        cJSON_AddNumberToObject(
            root,
            "adc_raw",
            adc_raw
        );

        cJSON_AddNumberToObject(
            root,
            "voltage_v",
            voltage
        );

        cJSON_AddBoolToObject(
            root,
            "led_state",
            s_led_state
        );

        cJSON_AddNumberToObject(
            root,
            "uptime_ms",
            (double)(esp_timer_get_time() / 1000)
        );

        char *json_str = cJSON_PrintUnformatted(root);

        if (json_str != NULL) {

            int msg_id = esp_mqtt_client_publish(
                s_mqtt_client,
                MQTT_TOPIC_TELEMETRY,
                json_str,
                0,
                0,
                0
            );

            if (msg_id >= 0) {

                ESP_LOGI(
                    TAG,
                    "Telemetry Published: %s",
                    json_str
                );

            } else {

                ESP_LOGW(
                    TAG,
                    "Failed to publish telemetry"
                );
            }

            free(json_str);
        }

        cJSON_Delete(root);
    }
}

/* ========================================================
 * 5. Main Application
 * ======================================================== */

void app_main(void)
{
    /* Initialize NVS */
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {

        ESP_ERROR_CHECK(nvs_flash_erase());

        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    /* Initialize LED GPIO */
    ESP_ERROR_CHECK(gpio_reset_pin(LED_PIN));

    ESP_ERROR_CHECK(
        gpio_set_direction(
            LED_PIN,
            GPIO_MODE_OUTPUT
        )
    );

    ESP_ERROR_CHECK(
        gpio_set_level(
            LED_PIN,
            0
        )
    );

    /* Initialize ADC */
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(
            &init_config,
            &s_adc_handle
        )
    );

    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            s_adc_handle,
            ADC_CHANNEL,
            &chan_config
        )
    );

    /* Connect Wi-Fi */
    wifi_init_sta();

    xEventGroupWaitBits(
        s_wifi_event_group,
        WIFI_CONNECTED_BIT,
        pdFALSE,
        pdTRUE,
        portMAX_DELAY
    );

    /* Configure MQTT */
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                .uri = MQTT_BROKER_URI,
            },
        },

        .session = {
            .keepalive = 60,

            .last_will = {
                .topic = MQTT_TOPIC_STATUS,
                .msg = "offline",
                .msg_len = 7,
                .qos = 1,
                .retain = 1,
            },
        },
    };

    s_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

    ESP_ERROR_CHECK(
        esp_mqtt_client_register_event(
            s_mqtt_client,
            ESP_EVENT_ANY_ID,
            mqtt_event_handler,
            NULL
        )
    );

    ESP_ERROR_CHECK(
        esp_mqtt_client_start(s_mqtt_client)
    );

    /* Start Telemetry Task */
    BaseType_t task_result = xTaskCreate(
        telemetry_task,
        "telemetry_task",
        4096,
        NULL,
        5,
        NULL
    );

    if (task_result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create telemetry task");
    }
}
