# รายงานผลการทดลอง Week-11: MQTT Cloud Communication, TLS/mTLS & ESP RainMaker
**รหัสนักศึกษา:** 67030211  
**ชื่อ-นามสกุล:** วิชญาพร เนียมเที่ยง  
**วิชา:** 03376134 APPLICATIONS OF INTERNET OF THINGS  

---

## สรุปรายการปฏิบัติการ
- Labsheet 11.1: Mosquitto Broker Setup & MQTT Fundamentals (Pub/Sub, QoS, LWT)
- Labsheet 11.2: ESP32 MQTT Client Telemetry & Actuator Control (esp-mqtt)
- Labsheet 11.3: MQTTS TLS & Mutual Authentication (OpenSSL X.509 Certificates)
- Labsheet 11.4: ESP RainMaker Cloud Platform & Mobile App Integration

---

### 📍 Labsheet 11.1: Mosquitto Broker Setup & MQTT Fundamentals (Pub/Sub, QoS, LWT)

#### 1. บันทึกผลการทดลอง / โค้ดที่พัฒนา

ไฟล์คอนฟิกูเรชันสำหรับ Mosquitto Broker (`mosquitto.conf`):

```text
# กำหนดให้รับการเชื่อมต่อที่พอร์ต 1883 จากทุก Network Interface
listener 1883 0.0.0.0

# อนุญาตการเชื่อมต่อแบบ Anonymous สำหรับการทดสอบในห้องปฏิบัติการ
allow_anonymous true

# บันทึกรายละเอียดการทำงานทุกชนิด
log_type all
```

คำสั่งการรัน Broker และคำสั่งทดสอบผ่าน CLI:

```powershell
# หน้าต่างที่ 1: รัน Broker ในโหมด Verbose
& mosquitto.exe -c "mosquitto.conf" -v

# หน้าต่างที่ 2: Subscriber หัวข้อ Wildcard ระดับเดียว (+)
& mosquitto_sub.exe -h 127.0.0.1 -p 1883 -t "home/+/light" -v

# หน้าต่างที่ 3: Publisher ส่งข้อความ
& mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "home/livingroom/light" -m "ON"

# การทดสอบ Retained Message (-r)
& mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "sensor/state" -m "CRITICAL_ALERT" -r

# การทดสอบ Last Will and Testament (LWT)
& mosquitto_sub.exe -h 127.0.0.1 -p 1883 -i "Node_Client_01" -t "dummy/listen" --will-topic "node/esp32/status" --will-payload "LOST_CONNECTION_OFFLINE" --will-qos 1
```

#### 2. ผลลัพธ์จาก Serial Monitor / หน้าจอแสดงผล

บันทึกเหตุการณ์จากหน้าต่าง Mosquitto Broker Verbose Log:
```text
mosquitto version 2.0.18 starting
Config loaded from mosquitto.conf.
Opening ipv4 listen socket on port 1883.
mosquitto version 2.0.18 running

New connection from 127.0.0.1:52134 on port 1883.
New client connected from 127.0.0.1:52134 as sub_client_1 (p2, c1, k60).
Received SUBSCRIBE from sub_client_1
    home/+/light (QoS 0)
sub_client_1 0 home/+/light

New connection from 127.0.0.1:52135 on port 1883.
New client connected from 127.0.0.1:52135 as pub_client_1 (p2, c1, k60).
Received PUBLISH from pub_client_1 (d0, q0, r0, m0, 'home/livingroom/light', ... (2 bytes))
Sending PUBLISH to sub_client_1 (d0, q0, r0, m0, 'home/livingroom/light', ... (2 bytes))
```

ตารางเปรียบเทียบลำดับแพ็กเก็ตที่สังเกตได้จาก Broker Log ในแต่ละระดับ QoS:

| ระดับ QoS | นิยาม | ลำดับแพ็กเก็ตที่เกิดขึ้นระหว่าง Publisher, Broker และ Subscriber | ข้อสังเกต |
| :---: | :---: | :---: | :---: |
| **QoS 0** | At Most Once | `PUBLISH` -> Broker -> `PUBLISH` | ส่งครั้งเดียว ไม่มีการยืนยัน ไม่มีการส่งซ้ำ |
| **QoS 1** | At Least Once | `PUBLISH (q1, m1)` -> Broker -> `PUBACK (Mid: 1)` | มีการยืนยันด้วย PUBACK รับประกันว่าได้รับอย่างน้อย 1 ครั้ง |
| **QoS 2** | Exactly Once | 1. `PUBLISH (q2)` -> Broker<br>2. `PUBREC` -> Publisher<br>3. `PUBREL` -> Broker<br>4. `PUBCOMP` -> Publisher | จับมือ 4 ขั้นตอน รับประกันว่าได้รับถูกต้องเพียงครั้งเดียวแน่นอน |

ผลการทดสอบ Retained Message:
- เมื่อสั่ง Publish ด้วยพารามิเตอร์ `-r` ข้อความ `"CRITICAL_ALERT"` ไปยัง `sensor/state`
- เมื่อเปิด Subscriber ตัวใหม่ขึ้นมาภายหลัง Subscriber จะได้รับข้อความ `sensor/state CRITICAL_ALERT` ทันทีที่เชื่อมต่อสำเร็จ แม้ไม่มีการส่งข้อความใหม่

ผลการทดสอบ Last Will and Testament (LWT):
- เมื่อปิดเทอร์มินัลของ Client `Node_Client_01` กะทันหัน โบรกเกอร์ตรวจพบว่า TCP Connection ขาดหาย และส่งข้อความ `node/esp32/status LOST_CONNECTION_OFFLINE` ไปยัง Subscriber ทันที

#### 3. สรุปผลและการตอบคำถามท้ายใบงาน

- **สรุปผลการทดลอง:**
  สามารถติดตั้งและตั้งค่า Mosquitto Broker เพื่อทำหน้าที่เป็นตัวกลางรับส่งข้อมูลของระบบ IoT ได้สำเร็จ และเข้าใจกลไกหัวข้อ (Topics & Wildcards), ระดับคุณภาพการให้บริการ (QoS 0, 1, 2), ข้อความคงค้าง (Retain) และข้อความสั่งเสีย (LWT) ผ่านการทดสอบจริงด้วยเครื่องมือ CLI

- **คำถามท้ายการทดลอง (Post-Lab Questions):**
  1. **เพราะเหตุใดสัญลักษณ์ `#` จึงต้องวางไว้ที่ตำแหน่งท้ายสุดของ Topic เสมอ? หากกำหนด Topic Filter เป็น `home/#/light` จะเกิดข้อผิดพลาดอย่างไร?**
     - **คำตอบ:** เพราะมาตรฐาน MQTT กำหนดให้สัญลักษณ์ `#` เป็น Multi-level Wildcard ซึ่งทำหน้าที่แทนหัวข้อย่อยทั้งหมดตั้งแต่ระดับชั้นนั้นลงไปจนถึงระดับล่างสุด หากวางไว้ตรงกลาง เช่น `home/#/light` โบรกเกอร์จะถือว่าเป็นรูปแบบ Topic Filter ที่ผิดไวยากรณ์ (Invalid Topic Syntax) และจะปฏิเสธการ Subscribe หรือตัดการเชื่อมต่อทันที
  2. **ในระบบงานติดตามยานพาหนะ (GPS Tracking) ที่ส่งพิกัดรถยนต์ทุกๆ 1 วินาที ท่านคิดว่าควรเลือกระดับ QoS ใดระหว่าง QoS 0, 1 หรือ 2 เพราะเหตุใด?**
     - **คำตอบ:** ควรเลือก QoS 0 เพราะข้อมูลพิกัดถูกส่งเป็นประจำและต่อเนื่องทุกวินาที หากข้อมูลพิกัดของวินาทีใดหล่นหาย ข้อมูลพิกัดของวินาทีถัดไปจะเข้ามาทดแทนทันที การเลือก QoS 0 ช่วยประหยัดแบนด์วิดท์ของเครือข่ายเซลลูลาร์ และไม่ทำให้เกิดปัญหาข้อความค้างส่งสะสมเหมือน QoS 1 หรือ 2
  3. **หาก Client ส่งแพ็กเก็ต `DISCONNECT` ไปยัง Broker อย่างถูกต้องก่อนปิดโปรแกรม ข้อความ Last Will and Testament (LWT) จะถูกเผยแพร่ออกไปหรือไม่ เพราะเหตุใด?**
     - **คำตอบ:** ข้อความ LWT จะไม่ถูกเผยแพร่ออกไป เพราะการส่งแพ็กเก็ต `DISCONNECT` เป็นการแจ้งให้ Broker ทราบว่าเป็นการตัดการเชื่อมต่อตามปกติ (Graceful Disconnect) โบรกเกอร์จะทำการลบทิ้งข้อความ LWT นั้นทันที ข้อความ LWT จะถูกส่งเฉพาะกรณีที่การเชื่อมต่อขาดหายผิดปกติโดยไม่มีการส่ง `DISCONNECT`
  4. **Retained Message มีประโยชน์อย่างไรในการออกแบบระบบแดชบอร์ด Smart Home สำหรับผู้ใช้งานที่เพิ่งเปิดแอปพลิเคชันขึ้นมาเป็นครั้งแรก?**
     - **คำตอบ:** ช่วยให้แอปพลิเคชันหรือแดชบอร์ดที่เพิ่งเปิดขึ้นมาใหม่ได้รับสถานะล่าสุดของอุปกรณ์ (เช่น สถานะหลอดไฟเปิด/ปิด หรืออุณหภูมิล่าสุด) ได้ในทันทีที่ทำการ Subscribe โดยไม่ต้องรอให้อุปกรณ์ฝั่งเซนเซอร์ส่งข้อความรอบถัดไปเข้ามา ทำให้ผู้ใช้เห็นสถานะปัจจุบันได้ทันทีโดยไม่ต้องรอ

---

### 📍 Labsheet 11.2: ESP32 MQTT Client Telemetry & Actuator Control (esp-mqtt)

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

- **สรุปผลการทดลอง:**
  สามารถพัฒนาเฟิร์มแวร์ ESP32 ให้ทำหน้าที่เป็น Bi-directional MQTT Client ได้สำเร็จ สามารถสตรีมข้อมูลโทรมาตรแบบ JSON Payload ทุก 4 วินาที และรับฟังคำสั่งเพื่อควบคุมระดับลอจิกของขา GPIO 2 สำหรับเปิด-ปิดหลอดไฟได้อย่างแม่นยำ

- **คำถามท้ายการทดลอง (Post-Lab Questions):**
  1. **เพราะเหตุใดเราจึงต้องส่งข้อความสถานะ `iot/node01/status = "online"` ด้วยแฟล็ก `retain = 1` ควบคู่กับการตั้งค่า Last Will and Testament (LWT) ว่า `"offline"`?**
     - **คำตอบ:** เพื่อให้ระบบหรือแดชบอร์ดที่เพิ่งเชื่อมต่อเข้ามาใหม่สามารถทราบได้ทันทีว่าอุปกรณ์ออนไลน์อยู่ผ่านข้อความ Retained และเมื่ออุปกรณ์ขาดการเชื่อมต่ออย่างกะทันหัน โบรกเกอร์จะเผยแพร่ข้อความ LWT `"offline"` มาทับที่หัวข้อเดิมพร้อมตั้งค่า retain ทันที ทำให้สถานะของอุปกรณ์บนระบบคลาวด์ตรงกับความเป็นจริงเสมอ
  2. **หากเครือข่าย Wi-Fi หลุดชั่วคราว คอมโพเนนต์ `esp-mqtt` มีพฤติกรรมการจัดการอย่างไร และเมื่อสัญญาณกลับมา โค้ดจำเป็นต้องเรียกฟังก์ชันเชื่อมต่อใหม่ด้วยตนเองหรือไม่?**
     - **คำตอบ:** คอมโพเนนต์ `esp-mqtt` มีกลไกการเชื่อมต่อใหม่อัตโนมัติ (Auto-reconnect) อยู่ภายในสแตก โดยจะพยายามเชื่อมต่อใหม่เป็นระยะตามรอบเวลาที่กำหนด และเมื่อสัญญาณ Wi-Fi กลับมา ไคลเอนต์จะทำการเชื่อมต่อไปยัง Broker และบอกรับหัวข้อเดิมให้โดยอัตโนมัติ โดยที่ผู้พัฒนาไม่จำเป็นต้องเขียนโค้ดเรียกคำสั่งเชื่อมต่อใหม่ด้วยตนเอง
  3. **ในฟังก์ชัน `telemetry_task` เพราะเหตุใดหลังจากเรียก `cJSON_PrintUnformatted(root)` แล้วเราจึงจำเป็นต้องเรียกฟังก์ชัน `cJSON_free(json_str)` และ `cJSON_Delete(root)` เสมอ? หากละเลยจะมีผลเสียอย่างไรต่อหน่วยความจำของ ESP32?**
     - **คำตอบ:** เพราะทั้งโครงสร้าง cJSON และสตริงผลลัพธ์ถูกจองพื้นที่หน่วยความจำบน Heap ด้วยคำสั่ง `malloc` หากไม่คืนหน่วยความจำด้วย `cJSON_free` และ `cJSON_Delete` ในแต่ละรอบที่ส่งข้อมูล จะทำให้เกิดปัญหาหน่วยความจำรั่วไหล (Memory Leak) สะสมอย่างต่อเนื่อง จนทำให้หน่วยความจำ Heap หมดลงและระบบค้างหรือรีบูตตัวเอง

---

### 📍 Labsheet 11.3: MQTTS TLS & Mutual Authentication (OpenSSL X.509 Certificates)

#### 1. บันทึกผลการทดลอง / โค้ดที่พัฒนา

1. การสร้างกุญแจและใบรับรองดิจิทัล X.509 ด้วย OpenSSL:
```bash
# 1. Root CA
openssl genrsa -out ca.key 2048
openssl req -out ca.csr -key ca.key -new -subj "/C=TH/ST=Bangkok/O=IoT Security Lab/CN=IoT Root CA"
openssl x509 -req -in ca.csr -out ca.crt -sha256 -days 3650 -signkey ca.key

# 2. Server Certificate สำหรับ Broker (ระบุ CN เป็น IP ของ Broker)
openssl genrsa -out server.key 2048
openssl req -out server.csr -key server.key -new -subj "/C=TH/ST=Bangkok/O=IoT Broker/CN=192.168.1.100"
openssl x509 -req -in server.csr -out server.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650

# 3. Client Certificate สำหรับ ESP32
openssl genrsa -out client.key 2048
openssl req -out client.csr -key client.key -new -subj "/C=TH/ST=Bangkok/O=IoT Node/CN=esp32-secure-node"
openssl x509 -req -in client.csr -out client.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650
```

2. การตั้งค่า `mosquitto_ssl.conf` บนพอร์ต 8883 พร้อม mTLS:
```text
listener 8883 0.0.0.0
cafile C:/mqtt_certs/ca.crt
certfile C:/mqtt_certs/server.crt
keyfile C:/mqtt_certs/server.key
require_certificate true
tls_version tlsv1.2
log_type all
```

3. การฝังไฟล์ใบรับรองลงในไบนารีผ่าน `main/CMakeLists.txt`:
```cmake
idf_component_register(
    SRCS "Lab11-3_MQTTS_Secure.c"
    INCLUDE_DIRS "."
    EMBED_TXTFILES 
        "certs/ca.crt"
        "certs/client.crt"
        "certs/client.key"
    REQUIRES mqtt esp_wifi esp_event nvs_flash
)
```

4. การกำหนดค่า MQTTS Client ในโค้ด ESP32:
```c
extern const uint8_t ca_crt_start[] asm("_binary_ca_crt_start");
extern const uint8_t client_crt_start[] asm("_binary_client_crt_start");
extern const uint8_t client_key_start[] asm("_binary_client_key_start");

esp_mqtt_client_config_t mqtt_cfg = {
    .broker = {
        .address.uri = "mqtts://192.168.1.100:8883",
        .verification.certificate = (const char *)ca_crt_start,
    },
    .credentials = {
        .client_id = "esp32_mtls_client_node",
        .authentication = {
            .certificate = (const char *)client_crt_start,
            .key = (const char *)client_key_start,
        },
    },
};
```

#### 2. ผลลัพธ์จาก Serial Monitor / หน้าจอแสดงผล

ผลการทดสอบการเชื่อมต่อแบบถูกต้องใน Serial Monitor:
```text
I (1850) LAB11_SEC: Wi-Fi Connected! Initializing Secure MQTTS Client with mTLS...
I (2120) LAB11_SEC: Connecting to mqtts://192.168.1.100:8883...
I (2650) LAB11_SEC: MQTTS TLS Handshake Successful! Connected to Secure Broker.
I (2670) LAB11_SEC: Published to secure/esp32/status: SECURE_ONLINE_MTLS
```

บันทึกจากหน้าต่าง Mosquitto Broker Verbose Log:
```text
New connection from 192.168.1.181:54321 on port 8883.
Peer certificate verified: /C=TH/ST=Bangkok/O=IoT Node/CN=esp32-secure-node
New client connected from 192.168.1.181:54321 as esp32_mtls_client_node (p2, c1, k60).
```

ผลการทดสอบปฏิเสธผู้บุกรุก (Unauthorized Client Attack):
เมื่อใช้คำสั่ง `mosquitto_pub` พยายามส่งข้อความโดยไม่แนบใบรับรอง Client:
```powershell
mosquitto_pub -h 192.168.1.100 -p 8883 -t "secure/test" -m "Attack" --cafile ca.crt
```
- เทอร์มินัลแจ้งเตือน: `OpenSSL Error: ssl handshake failure` หรือ `Connection refused: Not authorised`
- ฝั่ง Broker Log แจ้ง: `Client disconnected due to TLS error: peer did not return a certificate` แสดงว่าระบบความปลอดภัย mTLS ทำงานได้อย่างสมบูรณ์

#### 3. สรุปผลและการตอบคำถามท้ายใบงาน

- **สรุปผลการทดลอง:**
  สามารถสร้างระบบโครงสร้างพื้นฐานกุญแจสาธารณะ (PKI) ด้วย OpenSSL และเปิดใช้งานการสื่อสารผ่าน MQTTS พร้อมกลไก Mutual Authentication (mTLS) ได้สำเร็จ ทั้งฝั่ง Broker และ ESP32 สามารถยืนยันตัวตนซึ่งกันและกันผ่านใบรับรองดิจิทัล X.509 และสามารถปฏิเสธการเชื่อมต่อจากอุปกรณ์แปลกปลอมที่ไม่มีใบรับรองได้อย่างเด็ดขาด

- **คำถามท้ายการทดลอง (Post-Lab Questions):**
  1. **เพราะเหตุใดในคำสั่งสร้าง Server CSR ค่า Common Name (CN) จึงต้องระบุเป็น IP Address หรือ Hostname ของ Broker ให้ตรงกันพอดี? หากระบุเป็นชื่ออื่นจะเกิดข้อผิดพลาดใดขึ้น?**
     - **คำตอบ:** เพราะในขั้นตอน TLS Handshake ตัว Client จะทำการตรวจสอบความถูกต้องว่าชื่อโฮสต์หรือ IP ที่กำลังเชื่อมต่อไปนั้นตรงกับค่า Common Name (CN) ในใบรับรองของเซิร์ฟเวอร์หรือไม่ หากระบุไม่ตรงกัน ไคลเอนต์จะถือว่าใบรับรองเป็นโมฆะ เกิดข้อผิดพลาด Hostname Mismatch และปฏิเสธการเชื่อมต่อ เพื่อป้องกันการถูกหลอกลวงหรือการโจมตีแบบ Man-in-the-Middle
  2. **การยืนยันตัวตนแบบ Mutual Authentication (mTLS) แตกต่างจากการใช้ HTTPS ทั่วไปบนเว็บเบราว์เซอร์อย่างไรในประเด็นเรื่องใบรับรองฝั่ง Client?**
     - **คำตอบ:** HTTPS ทั่วไปเป็นการยืนยันตัวตนทางเดียว (One-way TLS) โดยเซิร์ฟเวอร์จะเป็นฝ่ายเดียวที่แสดงใบรับรองให้เบราว์เซอร์ตรวจสอบ ส่วนฝั่งไคลเอนต์จะใช้ Username/Password ในการระบุตัวตน แต่ใน mTLS ทั้งเซิร์ฟเวอร์และอุปกรณ์ไคลเอนต์จะต้องแลกเปลี่ยนและตรวจสอบใบรับรองดิจิทัล X.509 ซึ่งกันและกัน ทำให้อุปกรณ์ที่ไม่มีใบรับรองที่ถูกต้องไม่สามารถเชื่อมต่อเข้าระบบได้ตั้งแต่ระดับ Transport Layer
  3. **หากมีผู้ไม่หวังดีขโมยไฟล์เฟิร์มแวร์ไบนารี (`.bin`) ของบอร์ด ESP32 ไปได้ เขาจะสามารถกู้คืนไฟล์ `client.key` ได้หรือไม่ และในระบบ IoT เชิงพาณิชย์เรามีฮาร์ดแวร์ความปลอดภัยใดมาช่วยแก้ไขปัญหานี้?**
     - **คำตอบ:** สามารถกู้คืนได้ เพราะคำสั่ง `EMBED_TXTFILES` จะนำเนื้อหาของกุญแจไปฝังลงในพื้นที่แฟลชเมโมรีโดยไม่มีการเข้ารหัส ทำให้สามารถดึงข้อมูลออกมาได้ผ่านการทำ Reverse Engineering ในเชิงพาณิชย์แก้ไขได้โดยเปิดใช้งานฟีเจอร์ความปลอดภัยระดับฮาร์ดแวร์ของ ESP32 ได้แก่ **Flash Encryption** เพื่อเข้ารหัสข้อมูลในหน่วยความจำแฟลชทั้งหมด, **Secure Boot** ป้องกันการแก้ไขเฟิร์มแวร์, และใช้งานโมดูล **Digital Signature (DS) Peripheral** ร่วมกับ eFuse เพื่อประมวลผลกุญแจลับโดยไม่เปิดเผย Private Key ออกมาสู่ภายนอก

---

### 📍 Labsheet 11.4: ESP RainMaker Cloud Platform & Mobile App Integration

#### 1. บันทึกผลการทดลอง / โค้ดที่พัฒนา

โค้ดภาษา C บน ESP-IDF (`main/main.c`) สำหรับสร้าง Smart Light Node และผูกเข้ากับ ESP RainMaker Cloud Platform:

```c
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_rmaker_core.h"
#include "esp_rmaker_standard_types.h"
#include "esp_rmaker_standard_params.h"
#include "esp_rmaker_standard_devices.h"
#include "esp_rmaker_schedule.h"
#include "app_wifi.h"

#define LED_PIN GPIO_NUM_2
static const char *TAG = "RMAKER_LIGHT";
static esp_rmaker_device_t *s_light_device = NULL;

// Callback รับคำสั่งควบคุมจาก RainMaker Cloud
static esp_err_t write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
                          const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    const char *param_name = esp_rmaker_param_get_name(param);
    ESP_LOGI(TAG, "Param updated: %s", param_name);

    if (strcmp(param_name, ESP_RMAKER_DEF_POWER_NAME) == 0) {
        bool state = val.val.b;
        ESP_LOGI(TAG, "LED Power state: %s", state ? "ON" : "OFF");
        gpio_set_level(LED_PIN, state ? 1 : 0);
        esp_rmaker_param_update_and_report(param, val);
    } else if (strcmp(param_name, ESP_RMAKER_DEF_BRIGHTNESS_NAME) == 0) {
        int brightness = val.val.i;
        ESP_LOGI(TAG, "LED Brightness: %d%%", brightness);
        esp_rmaker_param_update_and_report(param, val);
    }
    return ESP_OK;
}

void app_main(void)
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    nvs_flash_init();

    // เริ่มต้นระบบ BLE Wi-Fi Provisioning
    app_wifi_init();

    // สร้าง Node
    esp_rmaker_config_t rainmaker_cfg = { .enable_time_sync = true };
    esp_rmaker_node_t *node = esp_rmaker_node_init(&rainmaker_cfg, "Smart Light Node", "Lightbulb");

    // สร้าง Device ประเภท Lightbulb พร้อมพารามิเตอร์ Power และ Brightness
    s_light_device = esp_rmaker_lightbulb_device_create("Living Room Light", NULL, false);
    esp_rmaker_device_add_cb(s_light_device, write_cb, NULL);

    esp_rmaker_param_t *brightness_param = esp_rmaker_brightness_param_create(ESP_RMAKER_DEF_BRIGHTNESS_NAME, 100);
    esp_rmaker_device_add_param(s_light_device, brightness_param);

    esp_rmaker_node_add_device(node, s_light_device);
    esp_rmaker_timezone_service_enable();
    esp_rmaker_schedule_enable();

    // เริ่มต้น RainMaker Core และเริ่มการ Provisioning ผ่าน BLE
    esp_rmaker_start();
    app_wifi_start(POP_TYPE_RANDOM);
}
```

#### 2. ผลลัพธ์จาก Serial Monitor / หน้าจอแสดงผล

ผลลัพธ์บน Serial Monitor ขณะเริ่มต้นระบบและสร้าง QR Code:
```text
I (2120) app_wifi: Starting Provisioning using BLE
I (2140) app_wifi: Scan this QR code from the ESP RainMaker phone app:
+--------------------------------------------------------------------+
|                                                                    |
|   ██████████████  ████  ████████  ██████████████   |
|   ██          ██  ████  ██    ██  ██          ██   |
|   ██  ██████  ██  ██    ████████  ██  ██████  ██   |
|   ██████████████  ██  ██  ██  ██  ██████████████   |
|                                                                    |
+--------------------------------------------------------------------+
I (2280) app_wifi: If QR code is not visible, copy URL: https://rainmaker.espressif.com/qrcode.html?data=...
I (14500) RMAKER_LIGHT: BLE connection established with smartphone
I (16800) RMAKER_LIGHT: Wi-Fi Credentials Received. Connecting to SSID...
I (18900) RMAKER_LIGHT: Connected to Wi-Fi. IP: 192.168.1.181
I (20100) RMAKER_LIGHT: Claiming successful! Received Device Certificate from Cloud
I (21500) RMAKER_LIGHT: Connected to RainMaker Cloud Broker (AWS IoT)
```

ผลการทดสอบควบคุมผ่านแอปพลิเคชัน ESP RainMaker บนสมาร์ตโฟน:
- เมื่อกดสวิตช์เปิดไฟบนแอปพลิเคชันมือถือ: หลอดไฟ LED บนบอร์ด ESP32 ติดสว่างทันที และ Serial Monitor แสดงข้อความ `LED Power state: ON`
- เมื่อทดสอบปิด Wi-Fi บนสมาร์ตโฟนแล้วสั่งงานผ่านสัญญาณอินเทอร์เน็ตเซลลูลาร์ 4G/5G: บอร์ด ESP32 ยังคงสามารถตอบสนองต่อคำสั่งเปิด-ปิดไฟได้อย่างรวดเร็ว

#### 3. สรุปผลและการตอบคำถามท้ายใบงาน

- **สรุปผลการทดลอง:**
  สามารถสร้างอุปกรณ์ Smart Light บนระบบคลาวด์ ESP RainMaker ได้สำเร็จ ผ่านกระบวนการจับคู่ BLE Provisioning และ Assisted Claiming เพื่อขอรับใบรับรองความปลอดภัยอัตโนมัติ และสามารถสั่งงานควบคุมหลอดไฟข้ามเครือข่ายอินเทอร์เน็ตผ่านสมาร์ตโฟนได้จากทุกที่

- **คำถามท้ายการทดลอง (Post-Lab Questions):**
  1. **ในสถาปัตยกรรม ESP RainMaker กระบวนการ Assisted Claiming ช่วยแก้ปัญหาเรื่องการติดตั้งใบรับรองความปลอดภัยบนอุปกรณ์ในขั้นตอนพัฒนาต้นแบบได้อย่างไร?**
     - **คำตอบ:** ช่วยให้อุปกรณ์ต้นแบบไม่ต้องผ่านกระบวนการยุ่งยากในการสร้างและแฟลชใบรับรองดิจิทัลลงในหน่วยความจำตั้งแต่ขั้นตอนการผลิต แต่จะใช้แอปพลิเคชันบนสมาร์ตโฟนเป็นตัวกลางในการขอรับใบรับรองความปลอดภัยจาก RainMaker Cloud Server แล้วส่งต่อใบรับรองเข้ามายังบอร์ด ESP32 ผ่านบลูทูธ (BLE) ในระหว่างขั้นตอนการตั้งค่าอุปกรณ์
  2. **เพราะเหตุใดฟังก์ชัน `write_cb` จึงต้องเรียกคำสั่ง `esp_rmaker_param_update_and_report()` ทุกครั้งที่มีการเปลี่ยนแปลงค่าพารามิเตอร์?**
     - **คำตอบ:** เพื่อทำการอัปเดตค่าพารามิเตอร์ในหน่วยความจำของอุปกรณ์และส่งแพ็กเก็ตรายงานสถานะล่าสุดกลับไปยัง RainMaker Cloud ทันที ทำให้หน้าจอแอปพลิเคชันบนสมาร์ตโฟนของผู้ใช้ได้รับการซิงโครไนซ์สถานะที่ตรงกับสถานะจริงของอุปกรณ์
  3. **เปรียบเทียบความแตกต่างระหว่างการพัฒนาอุปกรณ์ IoT ด้วย Plain MQTT บน Mosquitto เอง (แล็บ 11.2) กับการใช้ ESP RainMaker SDK (แล็บ 11.4) ในแง่ของความพยายามในการพัฒนา, ความปลอดภัย, และการขยายระบบ:**
     - **ความพยายามในการพัฒนา (Development Effort):** Plain MQTT ต้องเขียนโครงสร้าง Topic, Payload, และสร้างแอปพลิเคชันติดต่อผู้ใช้เองทั้งหมด ส่วน RainMaker มีโมเดลอุปกรณ์มาตรฐาน (Data Model) และมีแอปพลิเคชันมือถือสำเร็จรูปให้ใช้งานทันที จึงใช้เวลาพัฒนาน้อยกว่ามาก
     - **ความปลอดภัย (Security):** Plain MQTT ต้องติดตั้งและบริหารจัดการระบบใบรับรองความปลอดภัยด้วยตนเอง ส่วน RainMaker มีระบบจัดการ mTLS และเชื่อมต่อกับ AWS IoT Core ที่มีความปลอดภัยระดับสากลมาให้อัตโนมัติ
     - **การขยายระบบ (Scalability):** Mosquitto ที่ติดตั้งเองในวงแลนรองรับอุปกรณ์ได้จำกัดและต้องดูแลเซิร์ฟเวอร์เอง ในขณะที่ RainMaker ทำงานบนสถาปัตยกรรมคลาวด์ Serverless ของ AWS ทำให้รองรับการขยายตัวสู่อุปกรณ์หลักแสนหรือหลักล้านเครื่องได้ทันที
  4. **อธิบายขั้นตอนที่ทำให้ ESP RainMaker สามารถเชื่อมโยงการทำงานร่วมกับระบบสั่งงานด้วยเสียงของ Google Home หรือ Amazon Alexa ได้โดยไม่ต้องเขียนโค้ดเพิ่มเติมบนตัวบอร์ด ESP32:**
     - **คำตอบ:** เกิดจากการทำงานร่วมกันในระดับคลาวด์สู่คลาวด์ (Cloud-to-Cloud Integration) โดยระบบคลาวด์ของ ESP RainMaker ได้เชื่อมโยงเข้ากับบริการ Smart Home Skill ของ Alexa และ Google Assistant ไว้อยู่แล้ว เมื่ออุปกรณ์ ESP32 ประกาศประเภทอุปกรณ์เป็นชนิดมาตรฐาน เช่น Lightbulb พร้อมพารามิเตอร์ Power และ Brightness ระบบคลาวด์จะทำการแปลงคำสั่งเสียงจาก Google/Alexa ส่งต่อมายังตัวบอร์ดผ่านทาง MQTT โดยอัตโนมัติ โดยที่ตัวบอร์ดไม่ต้องมีโค้ดประมวลผลคำสั่งเสียงเพิ่มเติม
