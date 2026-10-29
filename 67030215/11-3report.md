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

**ผลการทดลองจริง**

<img width="1461" height="498" alt="image" src="https://github.com/user-attachments/assets/d779314e-e227-4d15-bf76-e380cbd3e2bb" />


<img width="1468" height="743" alt="image" src="https://github.com/user-attachments/assets/ea71e5bb-aaaf-4622-a265-34c1adb601ca" />


ผลที่ได้จริงต่างจากใบงานเล็กน้อย ฝั่ง Publisher ขึ้น `Error: The connection was lost` (ใบงานคาดว่าเป็น `Not authorised` หรือ `ssl handshake failure`) ฝั่ง Broker Log ขึ้น `OpenSSL Error ... peer did not return a certificate` ตามด้วย `disconnected: protocol error.` ซึ่งตรงตามที่คาด ส่วนฝั่ง Subscriber ไม่ได้รับ `Hacker message` ตามที่คาดไว้

**วิเคราะห์ผล**
- Broker ตรวจพบว่า Client ไม่ส่งใบรับรองมาในขั้นตอน TLS Handshake จึงตัดการเชื่อมต่อทันที ผู้บุกรุก**ไม่ได้รับ CONNACK** และไม่สามารถส่งข้อความ MQTT ใดๆ ได้เลย
- ข้อความ error ฝั่ง Client ต่างจากใบงาน เพราะ Mosquitto 2.1.2 / OpenSSL 3.5 ตัดการเชื่อมต่อทันทีหลังพบข้อผิดพลาด ฝั่ง Client จึงเห็นเพียงว่าการเชื่อมต่อขาด แต่ผลลัพธ์เหมือนกันคือ**ถูกปฏิเสธ**
- Client ที่มีใบรับรองถูกต้องเชื่อมต่อได้ด้วย **TLSv1.3** cipher `TLS_AES_256_GCM_SHA384` แม้ตั้งค่า `tls_version tlsv1.2` เพราะค่านี้คือเวอร์ชันขั้นต่ำที่ยอมรับ

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

## ผลการทดลอง

### กิจกรรม 11-3.1 การสร้างใบรับรองด้วย OpenSSL

<img width="1042" height="750" alt="image" src="https://github.com/user-attachments/assets/48fe41ba-c3ba-4f74-a57e-d826057872ec" />


ไฟล์ที่สร้างมี 3 คู่ ได้แก่
- **`ca.crt` / `ca.key`** คือ Root CA ผู้ลงนามรับรอง โดย `ca.crt` ใช้ทั้งฝั่ง Broker และ Client ส่วน `ca.key` ต้องเก็บเป็นความลับ
- **`server.crt` / `server.key`** คือตัวตนของ Broker (CN และ SAN = `172.20.10.4`) ใช้กับ Mosquitto
- **`client.crt` / `client.key`** คือตัวตนของ ESP32 (CN = `esp32-secure-node`) ใช้กับ ESP32 และ CLI ทดสอบ

ผล `openssl verify -CAfile ca.crt server.crt client.crt` ได้ `OK` ทั้งสองใบ แสดงว่าทั้งคู่ลงนามโดย Root CA ของเราจริง

**การปรับปรุงจากใบงาน:** เพิ่มไฟล์ส่วนขยาย (`-extfile`) ให้แต่ละใบรับรอง ได้แก่ `basicConstraints=CA:TRUE` ให้ Root CA, `subjectAltName=IP:172.20.10.4` และ `extendedKeyUsage=serverAuth` ให้ Server และ `extendedKeyUsage=clientAuth` ให้ Client เพราะ mbedTLS ใน ESP-IDF v6 และ OpenSSL 3.x ตรวจสอบข้อมูลส่วนนี้อย่างเข้มงวด

### กิจกรรม 11-3.2 การตั้งค่า Mosquitto MQTTS Broker

<img width="1020" height="528" alt="image" src="https://github.com/user-attachments/assets/d5af7c65-3999-4fd9-85e8-d2e9a029a8f4" />

```
listener 8883 0.0.0.0
cafile C:/mqtt_certs/ca.crt
certfile C:/mqtt_certs/server.crt
keyfile C:/mqtt_certs/server.key
require_certificate true
allow_anonymous true
tls_version tlsv1.2
log_type all
```

**การปรับปรุงจากใบงาน:** เพิ่ม `allow_anonymous true` เพราะ Mosquitto 2.x ตั้งค่าเริ่มต้นเป็น `false` ทำให้ปฏิเสธ Client ที่ไม่มี username แม้มีใบรับรองถูกต้อง และเปิด Windows Firewall พอร์ต TCP 8883

### กิจกรรม 11-3.3 การทดสอบด้วย CLI

**1. Client ที่มีใบรับรองถูกต้อง**

<img width="1450" height="447" alt="image" src="https://github.com/user-attachments/assets/7d8192c4-a898-4739-9266-1a8d5b1de2a0" />


Subscriber ได้รับ `secure/test Hello Secure World!` และ Broker Log แสดง `negotiated TLSv1.3 cipher TLS_AES_256_GCM_SHA384`

**2. Client ที่ไม่มีใบรับรอง (ผู้บุกรุก)**

<img width="1461" height="498" alt="image" src="https://github.com/user-attachments/assets/8f7bc8ea-5db6-4306-baa4-37bd92ecc25d" />


<img width="1468" height="743" alt="image" src="https://github.com/user-attachments/assets/60222399-4ca1-48b0-a8fb-02fc531ee765" />


ผลที่ได้จริงต่างจากใบงานเล็กน้อย ฝั่ง Publisher ขึ้น `Error: The connection was lost` (ใบงานคาดว่าเป็น `Not authorised` หรือ `ssl handshake failure`) ฝั่ง Broker Log ขึ้น `OpenSSL Error ... peer did not return a certificate` ตามด้วย `disconnected: protocol error.` ซึ่งตรงตามที่คาด ส่วนฝั่ง Subscriber ไม่ได้รับ `Hacker message` ตามที่คาดไว้

Broker ตัดการเชื่อมต่อตั้งแต่ขั้น TLS Handshake ผู้บุกรุกไม่ได้รับ CONNACK และไม่สามารถส่งข้อความ MQTT ใดๆ ได้ ข้อความ error ฝั่ง Client ต่างจากใบงานเพราะ Mosquitto 2.1.2 / OpenSSL 3.5 ตัดการเชื่อมต่อทันที ฝั่ง Client จึงเห็นเพียงว่าการเชื่อมต่อขาด แต่ผลลัพธ์เหมือนกันคือถูกปฏิเสธ

### กิจกรรม 11-3.4–11-3.5 ESP32 MQTTS Client แบบ mTLS

<img width="876" height="317" alt="image" src="https://github.com/user-attachments/assets/1492b36e-6a91-4af7-bedd-b06f68367cd5" />


<img width="1453" height="745" alt="image" src="https://github.com/user-attachments/assets/b1f40570-ba82-410d-8a35-30a0863de7fe" />


```
New connection from 172.20.10.3:64053 on port 8883.
New client connected from 172.20.10.3:64053 as esp32_mtls_client_node (p4, c1, k120).
Client esp32_mtls_client_node negotiated TLSv1.2 cipher ECDHE-RSA-AES256-GCM-SHA384
Sending CONNACK to esp32_mtls_client_node (0, 0)
Received PUBLISH from esp32_mtls_client_node (... 'secure/esp32/status', ... (18 bytes))
Received SUBSCRIBE from esp32_mtls_client_node
    secure/esp32/cmd (QoS 1)
```

**การรับส่งข้อความสองทางผ่าน mTLS**

<img width="1472" height="490" alt="image" src="https://github.com/user-attachments/assets/f5611a5f-4a2d-4fad-bf2a-f86606ff7cc5" />


<img width="1478" height="457" alt="image" src="https://github.com/user-attachments/assets/fdbfa774-fd51-4096-8d77-f883b18f5852" />


- ESP32 → คอมพิวเตอร์: Subscriber ได้รับ `secure/esp32/status SECURE_ONLINE_MTLS`
- คอมพิวเตอร์ → ESP32: Monitor แสดง `SECURE DATA RECEIVED` และ `PAYLOAD: Hello ESP32 via mTLS`

**เปรียบเทียบการเชื่อมต่อ**

เมื่อเทียบกัน คอมพิวเตอร์ (mosquitto CLI) เชื่อมต่อด้วย TLSv1.3 cipher `TLS_AES_256_GCM_SHA384` Keep Alive 60 วินาที และต่อติดแทบทันที ส่วน ESP32 (esp-mqtt + mbedTLS) เชื่อมต่อด้วย TLSv1.2 cipher `ECDHE-RSA-AES256-GCM-SHA384` Keep Alive 120 วินาที และใช้เวลาต่อประมาณ 8 วินาที (MQTT ธรรมดาต่อได้ทันที)

ESP32 ใช้ TLSv1.2 เพราะค่าเริ่มต้นของ mbedTLS ใน ESP-IDF ไม่ได้เปิด TLS 1.3 แต่ยังปลอดภัยสูงเพราะใช้ ECDHE (Forward Secrecy) ร่วมกับ AES-256-GCM และผ่านเงื่อนไข `tls_version tlsv1.2` ซึ่งเป็นเวอร์ชันขั้นต่ำที่ยอมรับ ส่วนเวลาเชื่อมต่อที่นานขึ้นเกิดจาก ESP32 ต้องคำนวณ RSA 2048 บิต ทั้งตรวจใบรับรองของ Broker และลงนามยืนยันตัวตนด้วย `client.key`

### ปัญหาที่พบและการแก้ไข

**ปัญหาที่ 1: `idf.py build` ไม่ผ่านด้วย CMakeLists ตามใบงาน** สาเหตุคือ ESP-IDF v6 แยก mqtt เป็น component ภายนอก และ `create-project` ตั้งชื่อไฟล์ตามชื่อโปรเจกต์ แก้โดยรัน `idf.py add-dependency "espressif/mqtt"` แก้ `SRCS "Lab11-3_MQTTS_Secure.c"` และเพิ่ม `REQUIRES`

**ปัญหาที่ 2: `undefined reference to app_main`** สาเหตุคือไฟล์โค้ดยังไม่ได้บันทึกเนื้อหา แก้โดยวางโค้ดลงไฟล์ที่ถูกต้องและบันทึกก่อน build

---
## 5. คำถามท้ายการทดลอง (Post-Lab Questions)

**1. เพราะเหตุใดในคำสั่งสร้าง Server CSR ค่า Common Name (CN) จึงต้องระบุเป็น IP Address หรือ Hostname ของ Broker ให้ตรงกันพอดี? หากระบุเป็นชื่ออื่นจะเกิดข้อผิดพลาดใดขึ้น?**
```
เพราะ Client จะตรวจว่าที่อยู่ที่ตัวเองเชื่อมต่ออยู่ ตรงกับชื่อที่ถูกรับรองไว้ในใบรับรองของ Server หรือไม่ เพื่อให้มั่นใจว่ากำลังคุยกับ Broker ตัวจริง ไม่ใช่เครื่องอื่นที่แอบอ้างหรือดักกลางทาง (Man-in-the-Middle) ถ้า CN ไม่ตรงกับ IP หรือ Hostname ที่ใช้เชื่อมต่อ จะเกิดข้อผิดพลาด Hostname Mismatch ทำให้ TLS Handshake ล้มเหลวและเชื่อมต่อไม่สำเร็จ แม้ใบรับรองจะลงนามโดย CA ที่ถูกต้องก็ตาม ในทางปฏิบัติปัจจุบันระบบจะตรวจจากช่อง Subject Alternative Name (SAN) เป็นหลัก ดังนั้นควรใส่ IP หรือ Hostname ลงใน SAN ด้วย ซึ่งตรงกับที่ผมปรับในการทดลองนี้
```
**2. การยืนยันตัวตนแบบ Mutual Authentication (mTLS) แตกต่างจากการใช้ HTTPS ทั่วไปบนเว็บเบราว์เซอร์อย่างไรในประเด็นเรื่องใบรับรองฝั่ง Client?**
```
HTTPS ทั่วไปเป็นการยืนยันตัวตนทางเดียว มีเพียงเว็บเซิร์ฟเวอร์ที่ต้องแสดงใบรับรองให้เบราว์เซอร์ตรวจ ส่วนฝั่งผู้ใช้ไม่ต้องมีใบรับรองใดๆ และไปพิสูจน์ตัวตนภายหลังด้วยรหัสผ่านหรือวิธีอื่น แต่ mTLS ทั้งสองฝ่ายต้องแสดงใบรับรองให้กันตรวจ Client ต้องมีใบรับรองและกุญแจส่วนตัวของตัวเอง และ Broker จะตรวจว่าใบรับรองนั้นลงนามโดย CA ที่เชื่อถือหรือไม่ ถ้าไม่มีหรือไม่ถูกต้อง การเชื่อมต่อจะถูกตัดตั้งแต่ขั้น Handshake ก่อนส่งข้อความ MQTT ใดๆ ได้ ซึ่งเหมาะกับระบบ IoT ที่อุปกรณ์ทุกตัวต้องพิสูจน์ตัวตนได้ด้วยตัวเองโดยไม่มีคนพิมพ์รหัสผ่าน
```
**3. หากมีผู้ไม่หวังดีขโมยไฟล์เฟิร์มแวร์ไบนารี (`.bin`) ของบอร์ด ESP32 ไปได้ เขาจะสามารถกู้คืนไฟล์ `client.key` ได้หรือไม่ และในระบบ IoT เชิงพาณิชย์เรามีฮาร์ดแวร์ความปลอดภัยใด (เช่น ESP32 Secure Boot / Flash Encryption / Digital Signature Peripheral) มาช่วยแก้ไขปัญหานี้?**
```
กู้คืนได้ เพราะในใบงานนี้ฝัง `client.key` ลงในเฟิร์มแวร์เป็นข้อความธรรมดา (Plain Text) ผ่าน `EMBED_TXTFILES` ผู้ที่ได้ไฟล์ `.bin` หรืออ่านข้อมูลจาก Flash ตรงๆ จึงค้นหาข้อความ `-----BEGIN PRIVATE KEY-----` แล้วดึงกุญแจออกมาได้ จากนั้นนำไปปลอมเป็นอุปกรณ์ตัวจริงเชื่อมต่อกับ Broker ได้ ซึ่งยอมรับได้ในห้องปฏิบัติการ แต่ไม่ปลอดภัยพอสำหรับงานจริง ระบบเชิงพาณิชย์จึงใช้ฮาร์ดแวร์ช่วยป้องกันดังนี้ **Flash Encryption** เข้ารหัสข้อมูลใน Flash ทั้งหมด ทำให้อ่านไฟล์ที่ดึงออกมาไม่ได้ **Secure Boot** ตรวจลายเซ็นเฟิร์มแวร์ก่อนรัน ป้องกันการแก้ไขหรือใส่เฟิร์มแวร์ปลอม และ **Digital Signature Peripheral (DS)** เก็บกุญแจส่วนตัวไว้ในฮาร์ดแวร์ที่โค้ดอ่านออกมาตรงๆ ไม่ได้ ใช้ได้แค่สั่งให้ลงนามแทน ทำให้ขโมยกุญแจไม่ได้แม้ได้เฟิร์มแวร์ไป นอกจากนี้ควรให้แต่ละอุปกรณ์มีใบรับรองของตัวเองเพื่อเพิกถอนเฉพาะตัวที่ถูกขโมยได้
```
