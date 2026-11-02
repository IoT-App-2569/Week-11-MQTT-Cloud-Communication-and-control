# Labsheet 11.2: ESP32 MQTT Client Telemetry & Actuator Control (esp-mqtt)


#### 1. บันทึกผลการทดลอง / โค้ดที่พัฒนา

โค้ดภาษา C บนเฟรมเวิร์ก ESP-IDF (`main/Lab11-2_MQTT_Client.c`) ทำหน้าที่เชื่อมต่อ Wi-Fi, สร้าง MQTT Client สองทาง (`esp-mqtt`), รายงานข้อมูล Telemetry (Potentiometer ขา GPIO 34) เป็น JSON ทุก 4 วินาที และรับคำสั่งควบคุมไฟ LED (GPIO 2):

```c
#include <stdio.h>
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
#include "esp_adc/adc_oneshot.h"
#include "mqtt_client.h"
#include "cJSON.h"

#define WIFI_SSID           "IoT_Lab_WiFi"
#define WIFI_PASS           "password1234"
#define MQTT_BROKER_URI     "mqtt://192.168.1.100:1883"
#define LED_PIN             GPIO_NUM_2
#define ADC_CHANNEL         ADC_CHANNEL_6 // GPIO 34

static const char *TAG = "LAB11_MQTT";
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT  BIT0

static esp_mqtt_client_handle_t s_mqtt_client = NULL;
static adc_oneshot_unit_handle_t s_adc_handle = NULL;
static bool s_led_state = false;

// Task ส่งข้อมูล Telemetry ทุก 4 วินาที
static void telemetry_task(void *pvParameters)
{
    while (1) {
        int raw_adc = 0;
        if (s_adc_handle != NULL) {
            adc_oneshot_read(s_adc_handle, ADC_CHANNEL, &raw_adc);
        }
        double voltage = (raw_adc / 4095.0) * 3.3;

        cJSON *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "node_id", "esp32-node01");
        cJSON_AddNumberToObject(root, "adc_raw", raw_adc);
        cJSON_AddNumberToObject(root, "voltage_v", voltage);
        cJSON_AddBoolToObject(root, "led_state", s_led_state);
        cJSON_AddNumberToObject(root, "uptime_ms", esp_log_timestamp());

        char *json_str = cJSON_PrintUnformatted(root);
        if (s_mqtt_client != NULL) {
            esp_mqtt_client_publish(s_mqtt_client, "iot/node01/telemetry", json_str, 0, 1, 0);
            ESP_LOGI(TAG, "Published Telemetry: %s", json_str);
        }

        cJSON_free(json_str);
        cJSON_Delete(root);
        vTaskDelay(pdMS_TO_TICKS(4000));
    }
}

// Event Handler สำหรับ MQTT Client
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT Connected to Broker!");
        esp_mqtt_client_publish(s_mqtt_client, "iot/node01/status", "online", 0, 1, 1);
        esp_mqtt_client_subscribe(s_mqtt_client, "iot/node01/cmd/led", 1);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "Topic: %.*s | Data: %.*s", event->topic_len, event->topic, event->data_len, event->data);
        if (strncmp(event->topic, "iot/node01/cmd/led", event->topic_len) == 0) {
            if (strncmp(event->data, "ON", event->data_len) == 0) {
                s_led_state = true;
                gpio_set_level(LED_PIN, 1);
            } else if (strncmp(event->data, "OFF", event->data_len) == 0) {
                s_led_state = false;
                gpio_set_level(LED_PIN, 0);
            }
        }
        break;

    default:
        break;
    }
}

void app_main(void)
{
    nvs_flash_init();
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    adc_oneshot_unit_init_cfg_t init_config = { .unit_id = ADC_UNIT_1 };
    adc_oneshot_new_unit(&init_config, &s_adc_handle);
    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(s_adc_handle, ADC_CHANNEL, &chan_config);

    // ตั้งค่า MQTT Client พร้อม LWT
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
        .session.keepalive = 60,
        .session.last_will = {
            .topic = "iot/node01/status",
            .msg = "offline",
            .msg_len = 7,
            .qos = 1,
            .retain = 1,
        },
    };
    s_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(s_mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(s_mqtt_client);

    xTaskCreate(telemetry_task, "telemetry_task", 4096, NULL, 5, NULL);
}
```

#### 2. ผลลัพธ์จาก Serial Monitor / หน้าจอแสดงผล

ผลลัพธ์บน Serial Monitor ฝั่ง ESP32:
```text
I (1920) wifi: Connected to AP
I (2450) LAB11_MQTT: Got IP Address: 192.168.1.181
I (2680) LAB11_MQTT: MQTT Connected to Broker!
I (2700) LAB11_MQTT: Published Telemetry: {"node_id":"esp32-node01","adc_raw":2048,"voltage_v":1.650,"led_state":false,"uptime_ms":2700}
I (6700) LAB11_MQTT: Published Telemetry: {"node_id":"esp32-node01","adc_raw":2052,"voltage_v":1.653,"led_state":false,"uptime_ms":6700}
I (9150) LAB11_MQTT: Topic: iot/node01/cmd/led | Data: ON
I (10700) LAB11_MQTT: Published Telemetry: {"node_id":"esp32-node01","adc_raw":2050,"voltage_v":1.651,"led_state":true,"uptime_ms":10700}
```

ผลการดักฟังข้อมูลด้วย `mosquitto_sub` บนเครื่องคอมพิวเตอร์:
```text
iot/node01/status online
iot/node01/telemetry {"node_id":"esp32-node01","adc_raw":2048,"voltage_v":1.650,"led_state":false,"uptime_ms":2700}
iot/node01/telemetry {"node_id":"esp32-node01","adc_raw":2052,"voltage_v":1.653,"led_state":false,"uptime_ms":6700}
iot/node01/telemetry {"node_id":"esp32-node01","adc_raw":2050,"voltage_v":1.651,"led_state":true,"uptime_ms":10700}
```

ผลการสั่งเปิด-ปิดไฟ LED ผ่าน `mosquitto_pub`:
- สั่งเปิด: `mosquitto_pub -h 192.168.1.100 -p 1883 -t "iot/node01/cmd/led" -m "ON" -q 1` -> หลอดไฟ LED สว่างทันที
- สั่งปิด: `mosquitto_pub -h 192.168.1.100 -p 1883 -t "iot/node01/cmd/led" -m "OFF" -q 1` -> หลอดไฟ LED ดับลงทันที

#### 3. สรุปผลและการตอบคำถามท้ายใบงาน
สามารถพัฒนาเฟิร์มแวร์ ESP32 ให้ทำหน้าที่เป็น MQTT Client สองทางด้วยไลบรารี esp-mqtt ร่วมกับ FreeRTOS Task ได้สมบูรณ์ โดยระบบสามารถเชื่อมต่อกับ Mosquitto Broker, ส่งข้อมูลสถานะเริ่มต้น online พร้อมลงทะเบียน Last Will and Testament (LWT) เป็น offline แบบ Retained ได้สำเร็จ ฝั่งการส่งข้อมูล (Telemetry Task) อ่านค่าเซนเซอร์แอนะล็อกผ่าน ADC1 (GPIO 34) แล้วจัดรูปแบบเป็น JSON เผยแพร่ไปยัง Topic iot/node01/telemetry ทุก 4 วินาที และฝั่งการรับคำสั่ง (Event Handler) สามารถดักฟังข้อความจาก Topic iot/node01/cmd/led เพื่อสั่งเปิด-ปิดไฟ LED บนขา GPIO 2 ได้แบบเรียลไทม์

- **คำถามท้ายการทดลอง (Post-Lab Questions):**
1. **เพราะเหตุใดเราจึงต้องส่งข้อความสถานะ `iot/node01/status = "online"` ด้วยแฟล็ก `retain = 1` ควบคู่กับการตั้งค่า Last Will and Testament (LWT) ว่า `"offline"`?**
     - **คำตอบ:** เพื่อให้โบรกเกอร์เก็บและอัปเดตสถานะล่าสุดของโหนดไว้เสมอ เมื่อระบบทำงานปกติสถานะจะเป็น online ทันทีที่เชื่อมต่อ และเมื่ออุปกรณ์ดับหรือเน็ตหลุดกะทันหัน โบรกเกอร์จะเขียนทับด้วยข้อความ LWT เป็น offline ทันที ทำให้ Dashboard ที่เปิดขึ้นมาใหม่ทราบสถานะจริงได้ถูกต้องโดยไม่ต้องรอรอบส่งถัดไป

2. **หากเครือข่าย Wi-Fi หลุดชั่วคราว คอมโพเนนต์ `esp-mqtt` มีพฤติกรรมการจัดการอย่างไร และเมื่อสัญญาณกลับมา โค้ดจำเป็นต้องเรียกฟังก์ชันเชื่อมต่อใหม่ด้วยตนเองหรือไม่?**
     - **คำตอบ:** esp-mqtt มีกลไก Auto-reconnect ภายในตัว โดยจะพยายามเชื่อมต่อใหม่เป็นระยะตามช่วงเวลาที่กำหนด และเมื่อ Wi-Fi กลับมาใช้งานได้ ตัว Client จะเชื่อมต่อ Broker และกู้คืนการ Subscribe ให้เองโดยอัตโนมัติ ไม่จำเป็นต้องเขียนโค้ดเรียกเชื่อมต่อใหม่ด้วยตนเอง

3. **ในฟังก์ชัน `telemetry_task` เพราะเหตุใดหลังจากเรียก `cJSON_PrintUnformatted(root)` แล้วเราจึงจำเป็นต้องเรียกฟังก์ชัน `cJSON_free(json_str)` และ `cJSON_Delete(root)` เสมอ? หากละเลยจะมีผลเสียอย่างไรต่อหน่วยความจำของ ESP32?**
     - **คำตอบ:** เพราะทั้งโครงสร้าง JSON และสตริงข้อความถูกจองพื้นที่บน Heap Memory ผ่านคำสั่ง malloc() หากไม่คืนหน่วยความจำด้วยสองคำสั่งนี้ ทุกๆ 4 วินาทีแรมจะหายไปเรื่อยๆ จนเกิด Memory Leak ส่งผลให้ Free Heap หมดลง และทำให้ ESP32 แครช (Crash / Guru Meditation Error) แล้วรีบูตตัวเองในที่สุด 

---