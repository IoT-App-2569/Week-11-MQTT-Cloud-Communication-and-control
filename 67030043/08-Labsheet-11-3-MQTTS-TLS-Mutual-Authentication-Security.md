# ใบงานการทดลองที่ 11.3
### การสื่อสาร MQTT ความปลอดภัยสูงด้วย TLS และการยืนยันตัวตนสองทาง (mTLS) บน Mosquitto และ ESP-IDF

> [!NOTE] **คำชี้แจง**
> ในใบงานนี้นักศึกษาจะได้เรียนรู้การยกระดับความปลอดภัยของการสื่อสาร IoT สู่ระดับมาตรฐานองค์กร โดยการสร้าง **องค์กรรับรองความถูกต้องส่วนบุคคล (Local Root CA)** ด้วยเครื่องมือ **OpenSSL** จากนั้นสร้างคู่กุญแจและใบรับรองดิจิทัล X.509 ให้กับ Mosquitto Broker และบอร์ด ESP32 เพื่อทำการเชื่อมต่อผ่านโปรโตคอล **MQTTS (พอร์ต 8883)** พร้อมบังคับใช้ระบบ **Mutual Authentication (mTLS)** ซึ่งทั้งสองฝ่ายต้องยืนยันตัวตนซึ่งกันและกันอย่างสมบูรณ์

---

## 1. วัตถุประสงค์การทดลอง
1. สามารถใช้คำสั่ง OpenSSL ในการสร้าง Root CA, Server Certificate, และ Client Certificate ได้
2. สามารถตั้งค่า Mosquitto Broker ให้เปิดพอร์ต 8883 พร้อมบังคับตรวจใบรับรอง Client (`require_certificate true`)
3. สามารถฝังไฟล์ใบรับรองดิจิทัลและกุญแจส่วนตัวลงในเฟิร์มแวร์ ESP-IDF ผ่านคำสั่ง `EMBED_TXTFILES` ของ CMake ได้
4. สามารถกำหนดค่า `esp_mqtt_client_config_t` สำหรับการเชื่อมต่อ MQTTS พร้อมอ้างอิงสัญลักษณ์ตัวแปร Assembler ได้
5. เข้าใจและสามารถวิเคราะห์ขั้นตอน TLS Handshake รวมถึงสาธิตการปฏิเสธการเชื่อมต่อจาก Client ที่ไม่มีใบรับรองถูกต้อง

---

## 2. อุปกรณ์และเครื่องมือที่ใช้
* เครื่องคอมพิวเตอร์ที่ติดตั้ง **OpenSSL** และ **Eclipse Mosquitto**
* บอร์ดไมโครคอนโทรลเลอร์ **ESP32**
* เฟรมเวิร์ก **ESP-IDF v5.x / v6.x** หรือ Docker
* เราเตอร์ Wi-Fi วงแลนเดียวกัน

---

## 3. ขั้นตอนการทดลอง

### กิจกรรมที่ 11-3.1 การสร้างคู่กุญแจและใบรับรองดิจิทัลด้วย OpenSSL

    > [!TIP]
> สำหรับนักศึกษาที่ต้องการศึกษาพื้นฐานความปลอดภัยไซเบอร์, ระบบคู่กุญแจ (Key-Pair), X.509 Certificates และรายละเอียดของแต่ละคำสั่ง OpenSSL สามารถศึกษาเพิ่มเติมได้ที่ **[ภาคผนวกที่ 2 พื้นฐานความปลอดภัยไซเบอร์, กลไกคู่กุญแจ และการใช้งาน OpenSSL สำหรับ IoT](10-Appendix-2-OpenSSL-and-Keys-pair.md)**

สร้างโฟลเดอร์สำหรับเก็บใบรับรอง เช่น `C:\mqtt_certs\` (หรือ `~/mqtt_certs/`) แล้วเปิดหน้าต่าง Terminal ในโฟลเดอร์ดังกล่าว

#### 1. สร้างกุญแจส่วนตัวและใบรับรอง Root CA (ผู้รับรอง)
```bash
# 1.1 สร้างกุญแจส่วนตัว 2048-bit RSA ของ CA
openssl genrsa -out ca.key 2048

# 1.2 สร้างคำร้องขอใบรับรอง (CSR) ของ CA
openssl req -out ca.csr -key ca.key -new -subj "/C=TH/ST=Bangkok/O=IoT Security Lab/CN=IoT Root CA"

# 1.3 ลงนามออกใบรับรอง ca.crt แบบ Self-signed มีอายุ 3650 วัน
openssl x509 -req -in ca.csr -out ca.crt -sha256 -days 3650 -signkey ca.key
```

#### 2. สร้างกุญแจส่วนตัวและใบรับรอง Server (สำหรับ Mosquitto Broker)
> [!IMPORTANT]
> ในคำสั่งด้านล่าง ค่า **CN (Common Name)** ของ Server จะต้องระบุเป็น **หมายเลข IP Address ของเครื่องคอมพิวเตอร์ที่รัน Broker** (เช่น `192.168.1.100`) เพื่อป้องกันข้อผิดพลาด Hostname Mismatch ในขั้นตอน TLS Handshake

```bash
# 2.1 สร้างกุญแจส่วนตัวของ Server
openssl genrsa -out server.key 2048

# 2.2 สร้างคำร้องขอใบรับรอง (เปลี่ยน 192.168.1.100 เป็น IP เครื่องของท่าน)
openssl req -out server.csr -key server.key -new -subj "/C=TH/ST=Bangkok/O=IoT Broker/CN=192.168.1.100"

# 2.3 ใช้ Root CA ลงนามออกใบรับรอง server.crt
openssl x509 -req -in server.csr -out server.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650
```

#### 3. สร้างกุญแจส่วนตัวและใบรับรอง Client (สำหรับ ESP32)
```bash
# 3.1 สร้างกุญแจส่วนตัวของ Client
openssl genrsa -out client.key 2048

# 3.2 สร้างคำร้องขอใบรับรองของ Client
openssl req -out client.csr -key client.key -new -subj "/C=TH/ST=Bangkok/O=IoT Node/CN=esp32-secure-node"

# 3.3 ใช้ Root CA ลงนามออกใบรับรอง client.crt
openssl x509 -req -in client.csr -out client.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650
```

---

### กิจกรรมที่ 11-3.2 การตั้งค่า Mosquitto MQTTS Broker (Port 8883)

สร้างไฟล์คอนฟิก `mosquitto_ssl.conf` ในโฟลเดอร์ `C:\mqtt_certs\`:

```text
# กำหนดให้ฟังพอร์ต 8883 สำหรับการเชื่อมต่อเข้ารหัส TLS
listener 8883 0.0.0.0

# ระบุตำแหน่งไฟล์ใบรับรอง (ใช้ Forward Slash / เสมอ)
cafile C:/mqtt_certs/ca.crt
certfile C:/mqtt_certs/server.crt
keyfile C:/mqtt_certs/server.key

# บังคับการตรวจสอบสิทธิ์สองทาง (Mutual Authentication / mTLS)
require_certificate true

# บังคับใช้ TLS เวอร์ชัน 1.2 ขึ้นไป
tls_version tlsv1.2

# บันทึกรายละเอียดการทำงาน
log_type all
```

สั่งรัน Mosquitto ใน **เทอร์มินัลที่ 1**
```powershell
& "C:\Program Files\mosquitto\mosquitto.exe" -c "C:\mqtt_certs\mosquitto_ssl.conf" -v
```

---

### กิจกรรมที่ 11-3.3 การทดสอบความปลอดภัยผ่านเครื่องมือ CLI

#### 1. การทดสอบแบบถูกต้อง (ส่งทั้ง ca.crt, client.crt, client.key)
เปิด **เทอร์มินัลที่ 2** แล้วสั่ง Subscribe
```powershell
mosquitto_sub -h 192.168.1.100 -p 8883 -t "secure/test" --cafile ca.crt --cert client.crt --key client.key -v
```
เปิด **เทอร์มินัลที่ 3** แล้วสั่ง Publish
```powershell
mosquitto_pub -h 192.168.1.100 -p 8883 -t "secure/test" -m "Hello Secure World!" --cafile ca.crt --cert client.crt --key client.key
```
*สังเกตผล* ในเทอร์มินัลที่ 2 จะได้รับข้อความสำเร็จ และใน Broker Log จะแสดงกระบวนการจับมือ TLS สำเร็จ

#### 2. การทดสอบการปฏิเสธผู้บุกรุก (Unauthorized Client Attack)
ทดลองใช้ `mosquitto_pub` ส่งข้อความ **โดยไม่ส่งใบรับรอง Client** (ส่งเฉพาะ cafile)
```powershell
mosquitto_pub -h 192.168.1.100 -p 8883 -t "secure/test" -m "Hacker message" --cafile ca.crt
```
**ผลลัพธ์ที่คาดหวัง**
* ในเทอร์มินัลจะแจ้งเตือนข้อผิดพลาด: `Error: Connection refused: Not authorised` หรือ `OpenSSL Error: ssl handshake failure`
* ในหน้าต่าง Broker Log จะแจ้ง: `Client disconnected due to TLS error: peer did not return a certificate`
* แสดงว่าระบบปฏิเสธการเชื่อมต่อจากอุปกรณ์แปลกปลอมได้อย่างสมบูรณ์

---

### กิจกรรมที่ 11-3.4 การสร้างโปรเจกต์ ESP-IDF และการฝัง Certificate ผ่าน CMake

#### 1. สร้างโปรเจกต์ใหม่
```powershell
idf.py create-project Lab11-3_MQTTS_Secure
cd Lab11-3_MQTTS_Secure
idf.py set-target esp32c3   # หรือ esp32
```

#### 2. การคัดลอกไฟล์ใบรับรองเข้าสู่โฟลเดอร์ `main/certs/`
สร้างโฟลเดอร์ `main/certs/` แล้วคัดลอกไฟล์ 3 ไฟล์จากโฟลเดอร์ `C:\mqtt_certs\` มาใส่
1. `ca.crt` (ใบรับรอง Root CA)
2. `client.crt` (ใบรับรองของ ESP32)
3. `client.key` (กุญแจส่วนตัวของ ESP32)

#### 3. การแก้ไขไฟล์ `main/CMakeLists.txt`
เปิดไฟล์ `main/CMakeLists.txt` แล้วระบุ dependencies และเพิ่มคำสั่ง `EMBED_TXTFILES`

```cmake
idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES mqtt esp_wifi esp_event nvs_flash esp_netif driver esp_driver_gpio
    EMBED_TXTFILES 
        "certs/ca.crt"
        "certs/client.crt"
        "certs/client.key"
)
```

#### 4. เพิ่ม MQTT component สำหรับ ESP-IDF 5.3 ขึ้นไป
สร้างไฟล์ `main/idf_component.yml` เพื่อให้ Component Manager ดาวน์โหลด MQTT component:

```yaml
dependencies:
  idf:
    version: ">=5.3"
  espressif/mqtt: "*"
```

> [!WARNING]
> ไฟล์ `client.key` เป็นกุญแจส่วนตัว ห้ามเผยแพร่หรือ commit ขึ้น repository ให้เก็บไฟล์ certificate ไว้ใน `main/certs/` เฉพาะเครื่องที่ใช้ build เท่านั้น

---

### กิจกรรมที่ 11-3.5 การเขียนโค้ดโปรแกรมหลัก (`main/main.c`)

เปิดไฟล์ `main/main.c` และบันทึกโค้ดต่อไปนี้

```c
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

#define WIFI_SSID           "YOUR_WIFI_SSID"
#define WIFI_PASS           "YOUR_WIFI_PASSWORD"
#define MQTTS_BROKER_URI    "mqtts://192.168.1.100:8883" // พอร์ต 8883

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
```

---

## 4. การทดสอบและการตรวจสอบผลลัพธ์ (Testing & Verification)

![alt text](img/image-12.png)
![alt text](img/image-13.png)
![alt text](img/image-14.png)
![alt text](img/image-15.png)
1. แฟลชโปรแกรมลงบนบอร์ด ESP32
   ```powershell
   idf.py -p COMx flash monitor
   ```
   
2. สังเกตผลในหน้าต่าง Serial Monitor
   * บอร์ดเชื่อมต่อ Wi-Fi สำเร็จ
   * เริ่มกระบวนการจับมือ TLS `Connecting to 192.168.1.100:8883...`
   * หน้าจอรายงาน `MQTTS TLS Handshake Successful! Connected to Secure Broker.`
   ![alt text](img/image-16.png)
3. สังเกตหน้าต่าง Broker Log
   * Mosquitto จะพิมพ์ข้อความยืนยัน `Peer certificate verified: /C=TH/ST=Bangkok/O=IoT Node/CN=esp32-secure-node`
   * แสดงว่า Broker ได้ทำการตรวจความถูกต้องของใบรับรองบอร์ด ESP32 ผ่านเรียบร้อยแล้ว
![alt text](img/image-17.png)
---

## 5. คำถามท้ายการทดลอง (Post-Lab Questions)

1. เพราะเหตุใดในคำสั่งสร้าง Server CSR ค่า **Common Name (CN)** จึงต้องระบุเป็น IP Address หรือ Hostname ของ Broker ให้ตรงกันพอดี? หากระบุเป็นชื่ออื่นจะเกิดข้อผิดพลาดใดขึ้น?

   **ตอบ:** ใบรับรองดิจิทัลทำหน้าที่ผูก "ตัวตน" (ชื่อ) เข้ากับ "กุญแจสาธารณะ" ตอน TLS handshake ฝั่ง Client (ESP32) จะตรวจสอบว่าชื่อในใบรับรองของเซิร์ฟเวอร์ (CN หรือ Subject Alternative Name: SAN) ตรงกับ host ที่ตั้งใจเชื่อมต่อ (IP หรือ Hostname ใน `MQTTS_BROKER_URI`) หรือไม่ เรียกว่า **Hostname Verification** ซึ่งป้องกันการโจมตีแบบ **Man-in-the-Middle (MITM)** เพราะหากไม่ตรวจ ผู้โจมตีสามารถนำใบรับรองที่ถูกต้องของเซิร์ฟเวอร์อื่น (ที่ CA เดียวกันออกให้) มาสวมรอยเป็น Broker ได้

   หากระบุชื่ออื่น การตรวจสอบชื่อจะล้มเหลว (hostname/CN mismatch) TLS handshake ไม่สำเร็จ และการเชื่อมต่อถูกตัดก่อนส่งข้อมูล MQTT ใด ๆ ฝั่ง ESP32 จะเกิด `MQTT_EVENT_ERROR` ประเภท `MQTT_ERROR_TYPE_TCP_TRANSPORT` (เช่น `ESP_ERR_MBEDTLS_SSL_HANDSHAKE_FAILED` หรือ `X509 - Certificate verification failed, CN mismatch`)

   หมายเหตุ: ปัจจุบันมาตรฐานแนะนำให้ใส่ชื่อ/IP ใน **SAN** เป็นหลัก และเมื่อใช้ IP Address ต้องระบุเป็น `IP:` ใน SAN

2. การยืนยันตัวตนแบบ **Mutual Authentication (mTLS)** แตกต่างจากการใช้ **HTTPS** ทั่วไปบนเว็บเบราว์เซอร์อย่างไรในประเด็นเรื่องใบรับรองฝั่ง Client?

   **ตอบ:**

   | ประเด็น | HTTPS ทั่วไป (One-way TLS) | Mutual TLS (mTLS) |
   |---|---|---|
   | การพิสูจน์ตัวตน | เซิร์ฟเวอร์พิสูจน์ตัวตนต่อ Client ฝั่งเดียว | ทั้งสองฝ่ายพิสูจน์ตัวตนต่อกัน |
   | ใบรับรองฝั่ง Client | ไม่จำเป็น | จำเป็น ต้องมี `client.crt` และ `client.key` ที่ออกโดย CA ที่เซิร์ฟเวอร์เชื่อถือ |
   | การยืนยันตัวตนผู้ใช้ | ทำในชั้นแอปพลิเคชัน เช่น username/password, token | ทำในชั้น TLS ด้วยใบรับรอง ก่อนถึงชั้นแอปพลิเคชัน |
   | การตรวจสอบของเซิร์ฟเวอร์ | ไม่ตรวจว่าใครเชื่อมต่อเข้ามา | ส่ง `CertificateRequest` และตรวจ `client.crt` กับ CA ของตน หากไม่ผ่านจะปฏิเสธการเชื่อมต่อ |
   | กรณีใช้งานทั่วไป | เว็บไซต์สาธารณะ | IoT, machine-to-machine, microservices |

   สรุป: ใน HTTPS ทั่วไปเบราว์เซอร์ตรวจสอบเพียงว่าเซิร์ฟเวอร์เป็นของจริง ส่วนใน mTLS Broker จะตรวจสอบด้วยว่าอุปกรณ์ที่เชื่อมต่อได้รับอนุญาต (มี `client.crt` ที่ลงนามโดย CA เดียวกัน และพิสูจน์การครอบครอง `client.key` ผ่านขั้นตอน CertificateVerify) อุปกรณ์ที่ไม่มีใบรับรองจึงเชื่อมต่อไม่ได้แม้จะรู้ที่อยู่ของ Broker

3. หากมีผู้ไม่หวังดีขโมยไฟล์เฟิร์มแวร์ไบนารี (`.bin`) ของบอร์ด ESP32 ไปได้ เขาจะสามารถกู้คืนไฟล์ `client.key` ได้หรือไม่ และในระบบ IoT เชิงพาณิชย์เรามีฮาร์ดแวร์ความปลอดภัยใด (เช่น ESP32 Secure Boot / Flash Encryption / Digital Signature Peripheral) มาช่วยแก้ไขปัญหานี้?

   **ตอบ:** **กู้คืนได้** หากไม่มีมาตรการป้องกัน เพราะ `client.key` ถูกฝังในเฟิร์มแวร์เป็นข้อความธรรมดา (plain text) ผ่าน `EMBED_TXTFILES` และเก็บอยู่ใน Flash ผู้โจมตีสามารถอ่าน Flash ด้วย `esptool.py read_flash` แล้วค้นหาข้อความ `-----BEGIN PRIVATE KEY-----` ก็จะได้ Private Key ครบถ้วน จากนั้นปลอมตัวเป็นอุปกรณ์เชื่อมต่อกับ Broker ได้ (Device Impersonation) และหากทุกบอร์ดใช้ Private Key ร่วมกัน การแก้ไขจะยากมาก

   ฮาร์ดแวร์/กลไกความปลอดภัยของ ESP32 ที่ช่วยแก้ปัญหา:

   - **Flash Encryption:** เข้ารหัสเนื้อหาใน Flash ด้วยกุญแจ AES ที่เก็บใน eFuse (ซอฟต์แวร์อ่านไม่ได้) แม้อ่าน Flash ออกมาก็ได้เพียงข้อมูลที่เข้ารหัส
   - **Secure Boot (v2):** ตรวจลายเซ็นดิจิทัลของ Bootloader และเฟิร์มแวร์ก่อนรัน ป้องกันการแทนที่เฟิร์มแวร์ด้วยโค้ดอันตรายที่อ่านและส่ง Private Key ออกไป
   - **Digital Signature (DS) Peripheral:** เก็บ Private Key ในรูปแบบเข้ารหัส และให้ฮาร์ดแวร์คำนวณลายเซ็น RSA ระหว่าง TLS handshake โดยตรง ซอฟต์แวร์จึงไม่สามารถเข้าถึง Private Key แบบ plain text ได้ (ใช้ร่วมกับ HMAC key ใน eFuse) เหมาะที่สุดกับกรณีนี้
   - **NVS Encryption / HMAC-based key protection:** เก็บใบรับรองและกุญแจใน NVS partition แบบเข้ารหัส และ provision แยกต่อบอร์ด แทนการฝังในไบนารี
   - **eFuse Read/Write Protect และการปิด JTAG / UART download mode:** ป้องกันการอ่านกุญแจและการดึง Flash ผ่านช่องทางดีบัก

   แนวปฏิบัติเชิงพาณิชย์เพิ่มเติม: ออก Client Certificate **แยกต่อบอร์ด** (unique per-device identity) เพื่อให้เพิกถอน (revoke) ได้เฉพาะอุปกรณ์ที่ถูกโจมตีผ่าน CRL/OCSP โดยไม่กระทบอุปกรณ์อื่น
