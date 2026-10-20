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

> [!NOTE] **สภาพแวดล้อมที่ใช้ทำการทดลองจริง (รหัสนักศึกษา 67030011)**
> * macOS + **OpenSSL 4.0.3**, Mosquitto 2.0.22, บอร์ด ESP32-D0WD-V3, ESP-IDF v6.0.2
> * IP ของเครื่องที่รัน Broker (Mac บน Hotspot) = **`172.20.10.2`** จึงใช้เป็น CN ของ Server Certificate
> * ไฟล์ทั้งหมดอยู่ใน [Lab11-3_MQTTS_Secure/](Lab11-3_MQTTS_Secure/) — สคริปต์สร้างใบรับรอง [mqtt_certs/gen_certs.sh](Lab11-3_MQTTS_Secure/mqtt_certs/gen_certs.sh), คอนฟิก Broker [mqtt_certs/mosquitto_ssl.conf](Lab11-3_MQTTS_Secure/mqtt_certs/mosquitto_ssl.conf), Log ใน [logs/](Lab11-3_MQTTS_Secure/logs/)
> * ไฟล์กุญแจส่วนตัว `*.key` และ `*.csr` **ไม่ถูก Commit** (อยู่ใน `.gitignore`) — รัน `./gen_certs.sh <IP>` เพื่อสร้างใหม่ก่อน Build


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

**ผลการทดลอง — พบปัญหาของคำสั่งในใบงานกับ OpenSSL รุ่นใหม่**

เมื่อใช้คำสั่งตามใบงานทุกตัวอักษรบน OpenSSL 4.0.3 ใบรับรอง `ca.crt` ที่ได้เป็น **X.509 v3 ที่มีแค่ Subject Key Identifier แต่ไม่มี `basicConstraints = CA:TRUE`** (ตั้งแต่ OpenSSL 3.2 คำสั่ง `x509 -req` ใส่ SKID/AKID ให้อัตโนมัติ) ใบรับรองจึงถูกปฏิเสธว่า "ไม่ใช่ CA" ทั้งใน `openssl verify` (error 79 invalid CA certificate), `mosquitto_pub` (`certificate verify failed`) และ Broker (`tlsv1 alert unknown ca`) — mbedTLS บน ESP32 ก็ตรวจเงื่อนไขเดียวกัน

<p align="center">
<img src="Images/lab11-3-1-labsheet-ca-error.png" alt="labsheet CA command error">
</p>

**วิธีแก้:** ใส่ส่วนขยาย v3 ให้ Root CA ตอนลงนาม ([mqtt_certs/ca_ext.cnf](Lab11-3_MQTTS_Secure/mqtt_certs/ca_ext.cnf))
```bash
openssl x509 -req -in ca.csr -out ca.crt -sha256 -days 3650 -signkey ca.key -extfile ca_ext.cnf -extensions v3_ca
```
```ini
[v3_ca]
basicConstraints = critical, CA:TRUE
keyUsage = critical, keyCertSign, cRLSign
subjectKeyIdentifier = hash
```
หลังแก้ `openssl verify` ผ่านทั้ง `server.crt` และ `client.crt`

<p align="center">
<img src="Images/lab11-3-1-certs.png" alt="certificates verified">
</p>


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

**ผลการทดลอง:** ใช้คอนฟิกเดียวกับใบงาน (Path แบบสัมพัทธ์ เพราะรัน Broker จากโฟลเดอร์ `mqtt_certs/`) และ **ต้องเพิ่ม `allow_anonymous true`** เพราะใน Mosquitto 2.x Listener ที่ประกาศเองจะไม่อนุญาต Client ที่ไม่มี Username โดยปริยาย แม้ผ่าน mTLS แล้วก็ตาม ถ้าไม่ใส่จะได้ `CONNACK (0, 5)` → `Connection Refused: not authorised` (ทางเลือกที่ปลอดภัยกว่าคือ `use_identity_as_username true` ให้ใช้ CN ของใบรับรองเป็น Username แล้วคุมสิทธิ์ด้วย ACL)

<p align="center">
<img src="Images/lab11-3-2-no-allow-anonymous.png" alt="mosquitto 2 not authorised without allow_anonymous">
</p>

<p align="center">
<img src="Images/lab11-3-2-broker-tls.png" alt="TLS broker listening on 8883">
</p>


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

**ผลการทดลอง:**

| การทดสอบ | ผลฝั่ง Client | ผลใน Broker Log |
| :--- | :--- | :--- |
| ส่งครบ `--cafile` + `--cert` + `--key` | Subscriber ได้รับ `secure/test Hello Secure World!` | `New client connected ... on port 8883` ตามปกติ |
| ส่งเฉพาะ `--cafile` (ไม่มีใบรับรอง Client) | `Error: The connection was lost.` | `OpenSSL Error: peer did not return a certificate` → `Client <unknown> disconnected: Protocol error.` |
| ไม่ระบุ `--cafile` เลย | `Error: Protocol error` | `tlsv1 alert unknown ca` (Client ใช้ CA ของระบบ จึงไม่เชื่อถือ IoT Root CA ที่สร้างเอง) |

Client ที่ไม่มีใบรับรองถูกตัดตั้งแต่ขั้น TLS Handshake **ยังไม่ทันส่งแพ็กเก็ต MQTT CONNECT** เลย

<p align="center">
<img src="Images/lab11-3-3-cli-mtls.png" alt="mTLS CLI tests">
</p>

รายละเอียด Session จาก `openssl s_client`: เครื่อง Mac เจรจาได้ **TLSv1.3, TLS_AES_256_GCM_SHA384**, Peer certificate `CN=172.20.10.2`, `Verification: OK`

<p align="center">
<img src="Images/lab11-3-tls-session.png" alt="openssl s_client session">
</p>


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
เปิดไฟล์ `main/CMakeLists.txt` แล้วเพิ่มคำสั่ง `EMBED_TXTFILES`

```cmake
idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    EMBED_TXTFILES 
        "certs/ca.crt"
        "certs/client.crt"
        "certs/client.key"
)
```

**ที่ใช้จริง** ([main/CMakeLists.txt](Lab11-3_MQTTS_Secure/main/CMakeLists.txt)): เพิ่ม `REQUIRES mqtt esp_wifi esp_event nvs_flash esp_driver_gpio` และต้อง `idf.py add-dependency "espressif/mqtt"` เพราะ ESP-IDF v6 แยก `mqtt` ออกเป็น Managed Component แล้ว


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

**โค้ดที่ใช้จริง:** [main/main.c](Lab11-3_MQTTS_Secure/main/main.c) — ตามใบงาน และเพิ่ม: อ่าน Wi-Fi จาก `wifi_credentials.h`, `nvs_flash_init` แบบ erase เมื่อจำเป็น, Log ขนาดใบรับรองที่ฝังไว้, Log `mbedTLS error` + `cert verify flags` ใน `MQTT_EVENT_ERROR` และให้คำสั่ง `ON`/`OFF` ที่ Topic `secure/esp32/cmd` ควบคุม LED (GPIO 2) เพื่อทดสอบการควบคุมผ่านช่องทางเข้ารหัส

Build ได้ .bin ขนาด 0xda600 (894,464 ไบต์)

<p align="center">
<img src="Images/lab11-3-4-build.png" alt="idf.py build lab 11-3">
</p>


---

## 4. การทดสอบและการตรวจสอบผลลัพธ์ (Testing & Verification)

1. แฟลชโปรแกรมลงบนบอร์ด ESP32
   ```powershell
   idf.py -p COMx flash monitor
   ```
2. สังเกตผลในหน้าต่าง Serial Monitor
   * บอร์ดเชื่อมต่อ Wi-Fi สำเร็จ
   * เริ่มกระบวนการจับมือ TLS `Connecting to 192.168.1.100:8883...`
   * หน้าจอรายงาน `MQTTS TLS Handshake Successful! Connected to Secure Broker.`
3. สังเกตหน้าต่าง Broker Log
   * Mosquitto จะพิมพ์ข้อความยืนยัน `Peer certificate verified: /C=TH/ST=Bangkok/O=IoT Node/CN=esp32-secure-node`
   * แสดงว่า Broker ได้ทำการตรวจความถูกต้องของใบรับรองบอร์ด ESP32 ผ่านเรียบร้อยแล้ว

**ผลการทดลอง:** บอร์ดได้ IP แล้วเริ่ม TLS Handshake ที่ 5,255 ms และเชื่อมต่อสำเร็จที่ 7,135 ms (**Handshake ≈ 1.9 วินาที** เพราะ ESP32 ต้องทำ RSA-2048 ทั้งตรวจใบรับรอง Server และเซ็นด้วย `client.key`) จากนั้นรับคำสั่ง `ON` / `OFF` / `ON` ผ่าน MQTTS แล้ว LED เปลี่ยนตาม

<p align="center">
<img src="Images/lab11-3-5-esp32-mtls-serial.png" alt="ESP32 mTLS serial log">
</p>

ฝั่ง Mac: Subscriber ที่มีใบรับรองได้รับ `secure/esp32/status SECURE_ONLINE_MTLS` จากบอร์ด และ Broker ส่งคำสั่งให้ `esp32_mtls_client_node` ด้วย QoS 1 (ได้ PUBACK กลับทุกครั้ง)

<p align="center">
<img src="Images/lab11-3-5-esp32-mtls-cli.png" alt="ESP32 mTLS broker and CLI">
</p>

> [!NOTE] Mosquitto 2.0.22 **ไม่พิมพ์** บรรทัด `Peer certificate verified: ...` ตามที่ใบงานระบุ (Log ปกติไม่แสดง Subject ของ Client) หลักฐานว่า Broker ตรวจใบรับรองจริงคือการทดสอบในกิจกรรม 11-3.3 ที่ Client ไม่มีใบรับรองถูกปฏิเสธด้วย `peer did not return a certificate` ส่วน ESP32 ผ่านได้ (ถ้าต้องการให้ CN ปรากฏใน Log ใช้ `use_identity_as_username true` แล้ว Client ID/Username จะเป็น `esp32-secure-node`) และ ESP32 เจรจาด้วย **TLS 1.2** เพราะ `CONFIG_MBEDTLS_SSL_PROTO_TLS1_3` ปิดอยู่ใน sdkconfig ค่าเริ่มต้น


---

## 5. คำถามท้ายการทดลอง (Post-Lab Questions)

1. **เพราะเหตุใดในคำสั่งสร้าง Server CSR ค่า Common Name (CN) จึงต้องระบุเป็น IP Address หรือ Hostname ของ Broker ให้ตรงกันพอดี? หากระบุเป็นชื่ออื่นจะเกิดข้อผิดพลาดใดขึ้น?**
   * **คำตอบ:** การยืนยันตัวตน Server ใน TLS มี 2 ขั้น คือ (1) ใบรับรองต้องถูกเซ็นโดย CA ที่ Client เชื่อถือ และ (2) **Hostname Verification** ชื่อในใบรับรอง (SAN `dNSName`/`iPAddress` หรือ CN ถ้าไม่มี SAN) ต้องตรงกับชื่อ/IP ที่ Client ใช้เชื่อมต่อจริง ขั้นที่ 2 ป้องกัน Man-in-the-Middle เพราะถ้าไม่ตรวจชื่อ ใครก็ตามที่มีใบรับรองใด ๆ จาก CA เดียวกัน (เช่น ใบของอุปกรณ์ตัวอื่น) ก็ปลอมตัวเป็น Broker ได้ ESP32 เชื่อมต่อด้วย `mqtts://172.20.10.2:8883` CN จึงต้องเป็น `172.20.10.2` พอดี
   * **ถ้าไม่ตรง:** Handshake ถูกยกเลิกทันทีก่อนส่งข้อมูล MQTT ใด ๆ ทดลองจริงโดยเชื่อมต่อเครื่องเดียวกันด้วย `-h 127.0.0.1` และ `-h localhost` (ไม่ตรงกับ CN=172.20.10.2) ได้ `Error: host name verification failed.` + `OpenSSL Error: certificate verify failed` ทั้งที่ใบรับรองถูกต้องทุกอย่าง ฝั่ง ESP32 (mbedTLS) จะได้ `MBEDTLS_ERR_X509_CERT_VERIFY_FAILED (-0x2700)` พร้อม verify flag `MBEDTLS_X509_BADCERT_CN_MISMATCH (0x04)` และ esp-tls รายงาน `ESP_ERR_MBEDTLS_SSL_HANDSHAKE_FAILED`
   * **ข้อสังเกตเพิ่มเติม:** มาตรฐานปัจจุบันให้ใส่ชื่อใน `subjectAltName` (เช่น `IP:172.20.10.2, DNS:broker.local`) เพราะ Client สมัยใหม่ไม่ดู CN เมื่อมี SAN และเมื่อ IP ของ Broker เปลี่ยน (DHCP ของ Hotspot) ต้องออกใบรับรองใหม่ จึงควรผูกกับ Hostname ที่คงที่แทน IP

<p align="center">
<img src="Images/lab11-3-q1-hostname-mismatch.png" alt="hostname mismatch">
</p>

2. **การยืนยันตัวตนแบบ Mutual Authentication (mTLS) แตกต่างจากการใช้ HTTPS ทั่วไปบนเว็บเบราว์เซอร์อย่างไรในประเด็นเรื่องใบรับรองฝั่ง Client?**
   * **คำตอบ:**

     | ประเด็น | HTTPS ทั่วไป (One-way TLS) | mTLS (ใบงานนี้) |
     | :--- | :--- | :--- |
     | ใครแสดงใบรับรอง | Server ฝ่ายเดียว | ทั้ง Server และ Client |
     | ข้อความ Handshake | ไม่มี `CertificateRequest` | Server ส่ง `CertificateRequest` → Client ต้องส่ง `Certificate` + `CertificateVerify` (ลายเซ็นด้วย Private Key เพื่อพิสูจน์ว่าถือกุญแจจริง) |
     | Client ยืนยันตัวตนอย่างไร | ภายหลังในชั้น Application (Username/Password, Cookie, OAuth) หลังช่องทางเข้ารหัสเสร็จแล้ว | ในชั้น TLS เลย ด้วยใบรับรองที่ CA ของเราออกให้ ถ้าไม่มี/ไม่ถูกต้อง ถูกตัดตั้งแต่ Handshake |
     | ใบรับรอง Client มาจากไหน | ไม่ต้องมี | ต้องมี PKI ของตัวเองออกใบให้ทุกอุปกรณ์ (กิจกรรม 11-3.1) |
     | เหมาะกับ | ผู้ใช้ทั่วไปจำนวนมากที่ไม่มีใบรับรอง | อุปกรณ์ IoT ที่ไม่มีคนพิมพ์รหัส ต้องการตัวตนเฉพาะเครื่อง และเพิกถอนรายเครื่องได้ (CRL) |

     ผลทดลองยืนยัน: Client ที่ส่งแค่ `--cafile` (เหมือนเบราว์เซอร์ที่เชื่อถือ Server ได้แต่ไม่มีใบรับรองของตัวเอง) ถูก Broker ตัดด้วย `peer did not return a certificate` ในขณะที่ HTTPS ปกติ Client แบบนี้เข้าได้

3. **หากมีผู้ไม่หวังดีขโมยไฟล์เฟิร์มแวร์ไบนารี (`.bin`) ของบอร์ด ESP32 ไปได้ เขาจะสามารถกู้คืนไฟล์ `client.key` ได้หรือไม่ และในระบบ IoT เชิงพาณิชย์เรามีฮาร์ดแวร์ความปลอดภัยใด (เช่น ESP32 Secure Boot / Flash Encryption / Digital Signature Peripheral) มาช่วยแก้ไขปัญหานี้?**
   * **คำตอบ: กู้คืนได้ทั้งไฟล์** เพราะ `EMBED_TXTFILES` นำข้อความ PEM ไปวางใน `.rodata` ของเฟิร์มแวร์ตรง ๆ โดยไม่เข้ารหัส ทดลองจริง:
     * ค้นใน `build/Lab11-3_MQTTS_Secure.bin` ด้วย `grep -abo` พบ `-----BEGIN PRIVATE KEY-----` 2 ตำแหน่ง ตำแหน่งแรก (63968) เป็นสตริงค่าคงที่ของตัว Parser PEM ใน mbedTLS ส่วนตำแหน่งที่สอง **offset 77577** คือกุญแจจริง ตัดมา 1,704 ไบต์แล้ว SHA-256 **ตรงกับ `client.key` ทุกไบต์** (`aa8574e2…6c8645`)
     * อ่านจากบอร์ดจริงผ่าน USB ด้วย `esptool read-flash 0x22000 0x2000` ก็พบกุญแจที่ Flash address `0x022F09` และ SHA-256 ตรงกันเช่นกัน ([logs/11-3_q3_key_extraction.txt](Lab11-3_MQTTS_Secure/logs/11-3_q3_key_extraction.txt)) แปลว่าแค่เข้าถึงตัวบอร์ดได้ก็ขโมยกุญแจได้ ไม่ต้องมีไฟล์ .bin
     * `espefuse summary` ของบอร์ดนี้: `FLASH_CRYPT_CNT = 0` (ไม่เข้ารหัส Flash), `ABS_DONE_0/1 = False` (ไม่มี Secure Boot), `JTAG_DISABLE = False`, `DISABLE_DL_ENCRYPT/DECRYPT = False` จึงไม่มีอะไรกันเลย
     * ผลกระทบ: ผู้โจมตีนำ `client.crt` + `client.key` ไปปลอมตัวเป็นบอร์ด (Clone) เชื่อมต่อ Broker เพื่อส่งข้อมูลปลอมหรือ Subscribe ดักคำสั่ง ซึ่ง Broker แยกไม่ออก

<p align="center">
<img src="Images/lab11-3-q3-key-in-bin.png" alt="private key found in firmware bin and efuse status">
</p>

   * **ฮาร์ดแวร์ความปลอดภัยที่ใช้แก้:**
     * **Flash Encryption:** เข้ารหัสเนื้อหา Flash ด้วย AES (คีย์อยู่ใน eFuse BLOCK1 ที่ Software อ่านไม่ได้) ถอดรหัสแบบโปร่งใสเฉพาะตอน CPU อ่านผ่าน Cache ดังนั้น `read-flash` หรือ .bin ที่หลุดออกไปจะเป็นข้อมูลที่อ่านไม่ออก (โหมด Release ปิดการเข้าถึงผ่าน UART ด้วย `DISABLE_DL_ENCRYPT/DECRYPT`)
     * **Secure Boot V2:** ROM ตรวจลายเซ็น (RSA-3072/ECDSA) ของ Bootloader และ App ก่อนรัน ป้องกันผู้โจมตีแฟลชเฟิร์มแวร์ของตัวเองเพื่ออ่านกุญแจจาก Flash ที่ถอดรหัสแล้ว ต้องใช้คู่กับ Flash Encryption
     * **Digital Signature (DS) Peripheral** (ESP32-S2/S3/C3/C6/H2 แต่ **ไม่มีใน ESP32 รุ่นแรกที่ใช้ทดลอง**): เก็บ Private Key ในรูปที่เข้ารหัสด้วย HMAC Key ใน eFuse และให้ฮาร์ดแวร์เซ็น TLS แทน Software จะไม่เห็นกุญแจจริงเลย (esp-tls รองรับผ่าน `ds_data` และพาร์ทิชัน `esp_secure_cert`) สำหรับ ESP32 รุ่นแรกอาจใช้ Secure Element ภายนอก เช่น ATECC608A
     * นอกจากนี้ต้องใช้ **ใบรับรองแยกรายเครื่อง** (กุญแจหลุด 1 เครื่องไม่กระทบทั้งระบบ), เพิกถอนด้วย CRL ที่ Broker, ปิด JTAG และ **ไม่เก็บกุญแจใน Git** (งานนี้จึงใส่ `*.key` ไว้ใน `.gitignore`)
