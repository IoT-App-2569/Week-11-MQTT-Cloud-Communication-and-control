# ใบงานการทดลองที่ 11.4
### การสร้างอุปกรณ์ Smart Light สู่ระบบคลาวด์ด้วย ESP RainMaker SDK, BLE Provisioning และการควบคุมผ่านสมาร์ตโฟน

> [!NOTE] **คำชี้แจง**
> ในใบงานนี้นักศึกษาจะได้สร้างผลิตภัณฑ์ IoT สู่ระบบคลาวด์เชิงพาณิชย์จริงโดยใช้แพลตฟอร์ม **ESP RainMaker** ของ Espressif Systems โดยจะได้เขียนโปรแกรมสร้าง **Node** และ **Device (Smart Light)** พร้อมพารามิเตอร์ **Power** และ **Brightness** จากนั้นทำการเปิดใช้งานระบบ **BLE Wi-Fi Provisioning**, การขอรับใบรับรองผ่านระบบ **Assisted Claiming** สแกนเชื่อมต่อผ่านแอปพลิเคชัน **ESP RainMaker บนสมาร์ตโฟน** และควบคุมอุปกรณ์ข้ามเครือข่ายอินเทอร์เน็ตได้จากทุกที่ทั่วโลก

---

## 1. วัตถุประสงค์การทดลอง
1. เข้าใจโครงสร้างและการทำงานของแพลตฟอร์ม ESP RainMaker บนสถาปัตยกรรม ESP-IDF
2. สามารถสร้าง Node, Device, และ Parameters (Power, Brightness) ผ่าน RainMaker Core API ได้
3. สามารถเขียน Write Callback เพื่อรับคำสั่งจากคลาวด์มาปรับระดับสถานะ GPIO ของ LED และรายงานสถานะกลับ (Report State)
4. สามารถเปิดใช้งานบริการระดับระบบ ได้แก่ บริการซิงก์เวลา SNTP และการตั้งเวลาทำงาน (Schedule Service) ได้
5. สามารถทำกระบวนการ Provisioning และ Assisted Claiming ผ่านแอปพลิเคชันสมาร์ตโฟนด้วยบลูทูธ (BLE) ได้สำเร็จ
6. สามารถทดสอบส่งคำสั่งควบคุมผ่านแอปพลิเคชันมือถือ และเครื่องมือบรรทัดคำสั่ง ESP RainMaker CLI ได้

---

## 2. อุปกรณ์และเครื่องมือที่ใช้
* บอร์ดไมโครคอนโทรลเลอร์ **ESP32-C3** หรือ **ESP32**
* หลอดไฟ LED พร้อมตัวต้านทาน 220Ω ต่อเข้ากับ **GPIO 8** (ESP32-C3) หรือ **GPIO 2** (ESP32)
* สายเชื่อมต่อ USB Data Cable
* เครื่องคอมพิวเตอร์พร้อมระบบพัฒนา **ESP-IDF v5.x / v6.x**
* สมาร์ตโฟน (Android หรือ iOS) ที่ติดตั้งแอปพลิเคชัน **ESP RainMaker** และเปิดบลูทูธ/Wi-Fi ไว้
* เราเตอร์ Wi-Fi (2.4 GHz) ที่เชื่อมต่อออกสู่อินเทอร์เน็ตได้จริง

---

## 3. ขั้นตอนการทดลอง (Deconstructed Activities)

### กิจกรรมที่ 11-4.1 การสร้างโปรเจกต์และติดตั้ง Dependency `esp-rainmaker`

#### 1. สร้างโปรเจกต์ใหม่
```powershell
idf.py create-project Lab11-4_RainMaker_Light
cd Lab11-4_RainMaker_Light
idf.py set-target esp32c3   # หรือ esp32
```

#### 2. ติดตั้ง ESP RainMaker Component จาก IDF Component Registry
เพิ่มไลบรารีทางการของ Espressif ผ่านคำสั่ง
```powershell
idf.py add-dependency "espressif/esp-rainmaker"
```

ไฟล์ `main/idf_component.yml` จะได้รับการสร้างและอัปเดตโดยอัตโนมัติ
```yaml
dependencies:
  idf:
    version: ">=5.0.0"
  espressif/esp-rainmaker:
    version: "^3.0.0"
```

---

### กิจกรรมที่ 11-4.2 การเขียนโค้ดเฟิร์มแวร์อุปกรณ์ (`main/main.c`)

เปิดไฟล์ `main/main.c` และแทนที่ด้วยโค้ดสมบูรณ์ต่อไปนี้

```c
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

// RainMaker Headers
#include "esp_rmaker_core.h"
#include "esp_rmaker_standard_types.h"
#include "esp_rmaker_standard_params.h"
#include "esp_rmaker_standard_devices.h"
#include "esp_rmaker_schedule.h"
#include "app_wifi.h"

#if CONFIG_IDF_TARGET_ESP32C3
#define LED_PIN GPIO_NUM_8
#else
#define LED_PIN GPIO_NUM_2
#endif

static const char *TAG = "RMAKER_LIGHT";
static esp_rmaker_device_t *s_light_device = NULL;

/* ========================================================
 * 1. Write Callback ฟังก์ชันรับคำสั่งควบคุมจากคลาวด์
 * ======================================================== */
static esp_err_t write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
                          const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    if (ctx) {
        ESP_LOGI(TAG, "Request received via: %s", esp_rmaker_device_cb_src_to_str(ctx->src));
    }
    const char *device_name = esp_rmaker_device_get_name(device);
    const char *param_name = esp_rmaker_param_get_name(param);

    ESP_LOGI(TAG, "Device: %s | Param: %s", device_name, param_name);

    // ตรวจสอบว่าเป็นคำสั่ง Power (เปิด/ปิด)
    if (strcmp(param_name, ESP_RMAKER_DEF_POWER_NAME) == 0) {
        bool state = val.val.b;
        ESP_LOGI(TAG, "Changing LED state to: %s", state ? "ON" : "OFF");
        gpio_set_level(LED_PIN, state ? 1 : 0);
        
        // รายงานสถานะปัจจุบันกลับสู่คลาวด์ เพื่อให้แอปบนมือถืออัปเดตสถานะทันที
        esp_rmaker_param_update_and_report(param, val);
    }
    // ตรวจสอบว่าเป็นคำสั่ง Brightness (ความสว่าง 0 - 100%)
    else if (strcmp(param_name, ESP_RMAKER_DEF_BRIGHTNESS_NAME) == 0) {
        int brightness = val.val.i;
        ESP_LOGI(TAG, "Changing Brightness to: %d%%", brightness);
        
        // (สามารถนำค่า brightness 0-100 ไปปรับค่า LEDC PWM ในงานจริงได้)
        esp_rmaker_param_update_and_report(param, val);
    }

    return ESP_OK;
}

/* ========================================================
 * 2. ฟังก์ชันหลัก (app_main)
 * ======================================================== */
void app_main(void)
{
    // กำหนดค่า GPIO ควบคุม LED
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);

    // เริ่มต้นระบบจัดเก็บข้อมูล NVS Flash
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // 1. เริ่มต้นระบบ Wi-Fi Provisioning (ผ่าน BLE)
    app_wifi_init();

    // 2. สร้าง Node ในระบบ ESP RainMaker
    esp_rmaker_config_t rainmaker_cfg = {
        .enable_time_sync = true, // เปิดบริการซิงก์เวลาโลกอัตโนมัติ
    };
    esp_rmaker_node_t *node = esp_rmaker_node_init(&rainmaker_cfg, "Smart Light Node", "Lightbulb");
    if (!node) {
        ESP_LOGE(TAG, "Could not initialise node.");
        return;
    }

    // 3. สร้าง Device ประเภท Lightbulb
    s_light_device = esp_rmaker_lightbulb_device_create("Living Room Light", NULL, false);
    // ผูก Callback สำหรับรับคำสั่ง
    esp_rmaker_device_add_cb(s_light_device, write_cb, NULL);

    // 4. เพิ่ม Parameter Brightness (ความสว่างเริ่มต้น 100%)
    esp_rmaker_param_t *brightness_param = esp_rmaker_brightness_param_create(ESP_RMAKER_DEF_BRIGHTNESS_NAME, 100);
    esp_rmaker_device_add_param(s_light_device, brightness_param);

    // 5. บรรจุ Device เข้าสู่ Node
    esp_rmaker_node_add_device(node, s_light_device);

    // 6. เปิดบริการระดับระบบ (System Services)
    esp_rmaker_timezone_service_enable(); // บริการโซนเวลา
    esp_rmaker_schedule_enable();         // บริการตั้งเวลาอัตโนมัติ

    // 7. เริ่มการทำงานของระบบ RainMaker Core
    esp_rmaker_start();

    // 8. เริ่มต้นระบบค้นหาอุปกรณ์และรับการตั้งค่า Wi-Fi (BLE Provisioning)
    app_wifi_start(POP_TYPE_RANDOM);
}
```

---

### กิจกรรมที่ 11-4.3 การคอมไพล์ แฟลช และสังเกตการสร้าง QR Code

สั่งคอมไพล์และแฟลชโปรแกรม
```powershell
idf.py build
idf.py -p COMx flash monitor
```

**ผลลัพธ์ใน Serial Monitor**
* บอร์ดจะพิมพ์ **QR Code ในรูปแบบตัวอักษร ASCII** ออกมาทางหน้าจอ พร้อม URL สำหรับเปิดดูรูปภาพ QR Code
* บอร์ดจะเริ่มประกาศสัญญาณบลูทูธ (BLE Advertising) เพื่อรอรับการเชื่อมต่อ

---

### กิจกรรมที่ 11-4.4 การเชื่อมต่อผ่านแอปพลิเคชัน ESP RainMaker บนมือถือ

<p align="center">
  <img src="Images/image13.png" width="280">
  &nbsp;&nbsp;&nbsp;&nbsp;
  <img src="Images/image14.png" width="280">
  &nbsp;&nbsp;&nbsp;&nbsp;
  <img src="Images/image15.png" width="280">
</p>
<p align="center"><b>รูปที่ 11.13</b> ขั้นตอนการเปิดใช้งาน Assisted Claiming และการจับคู่อุปกรณ์บนโทรศัพท์มือถือ</p>

1. **เข้าสู่ระบบ** เปิดแอปพลิเคชัน **ESP RainMaker** บนโทรศัพท์มือถือ เข้าสู่ระบบด้วยบัญชีที่ลงทะเบียนไว้
2. **เพิ่มอุปกรณ์** กดปุ่มเครื่องหมายบวก (`+ Add Device`) บนหน้าจอแอป
3. **สแกน QR Code** นำกล้องสมาร์ตโฟนสแกน QR Code ที่ปรากฏบน Serial Monitor
4. **กระบวนการ Assisted Claiming**
   * หากบอร์ดยังไม่มีใบรับรอง แอปจะขึ้นหน้าต่างยืนยันการขอรับ Certificate ให้กดปุ่มยืนยัน
   * แอปจะดึงใบรับรองจาก RainMaker Cloud แล้วส่งให้ ESP32 ผ่านช่องทาง BLE
5. **กรอกรหัส Wi-Fi** เลือกชื่อ SSID เครือข่าย Wi-Fi 2.4 GHz ในบ้านและกรอกรหัสผ่าน
6. **เสร็จสิ้นการผูกอุปกรณ์** บอร์ดจะเชื่อมต่อ Wi-Fi และผูกตัวเองเข้ากับบัญชีผู้ใช้ แอปจะแสดงการ์ด **"Living Room Light"** ขึ้นมาบนหน้าจอ

---

## 4. การทดสอบและการตรวจสอบผลลัพธ์ (Testing & Verification)

### 1. การควบคุมเปิด-ปิดผ่าน Mobile App
* กดสวิตช์ Power บนหน้าจอแอป สังเกตว่าหลอดไฟ LED บนบอร์ดเปิดและปิดทันทีตามการสั่งการ
* ทดลองเลื่อนแถบสไลเดอร์ Brightness สังเกตข้อความ Log บน Serial Monitor ว่าได้รับค่าความสว่างใหม่กี่เปอร์เซ็นต์

### 2. การควบคุมระยะไกลข้ามอินเทอร์เน็ต (Remote 4G/5G Test)
* **ปิดสัญญาณ Wi-Fi บนสมาร์ตโฟน** แล้วเปิดใช้งานเฉพาะ **สัญญาณอินเทอร์เน็ตมือถือ (4G/5G)**
* กดสั่งเปิด-ปิดหลอดไฟอีกครั้ง สังเกตว่าบอร์ด ESP32 ที่บ้านยังคงตอบสนองต่อคำสั่งได้อย่างรวดเร็ว แสดงถึงพลังของสถาปัตยกรรม Cloud Control

## ผลการทดลอง

### กิจกรรม 11-4.1 การสร้างโปรเจกต์และติดตั้ง Dependency

| รายการ | ค่าที่ใช้จริง |
|:---|:---|
| ESP-IDF | v6.0.2 |
| Target | ESP32 |
| Component หลัก | `espressif/esp_rainmaker` ^1.17.0 |
| Component Wi-Fi Provisioning | `espressif/rmaker_app_network` ^1.4.4 |
| Flash / Partition | 4 MB, `ota_0` + `ota_1` (1600K) และ `fctry` ที่ 0x340000 |

### กิจกรรม 11-4.3 การแฟลชและ QR Code

<img width="943" height="683" alt="image" src="https://github.com/user-attachments/assets/d70b4401-1f6b-4c5f-8874-aa2240dc1fcd" />


บอร์ดพิมพ์ QR Code พร้อมข้อมูล `{"ver":"v1","name":"PROV_51681e","username":"wifiprov","pop":"6c38a8d3","transport":"ble"}` และเริ่ม BLE Advertising รอการจับคู่ โดยมี Node ID คือ `SFyfCRezgekerpXw4se7yf`

### กิจกรรม 11-4.4 การจับคู่ผ่านแอป ESP RainMaker

<img width="868" height="1887" alt="image" src="https://github.com/user-attachments/assets/a8fce9dc-7724-410c-98e1-b1d329cc14db" />

ลำดับเหตุการณ์จาก Serial Monitor

| ขั้นตอน | Log ที่สังเกตได้ |
|:---|:---|
| 1. แอปเชื่อมต่อ BLE และสร้าง Session เข้ารหัส (Security 2) | `app_network: Secured session established!` |
| 2. ยืนยันความเป็นเจ้าของ Node (Challenge-Response) | `esp_rmaker_chal_resp: Challenge-Response handler completed successfully` |
| 3. รับข้อมูล Wi-Fi จากแอปทาง BLE | `app_wifi: Received Wi-Fi credentials SSID: IPK_` |
| 4. เชื่อมต่อ Wi-Fi | `sta ip: 172.20.10.3` และ `Provisioning successful` |
| 5. เชื่อมต่อคลาวด์ AWS IoT Core ผ่าน MQTTS | `Connecting to a1p72mufdu6064-ats.iot.us-east-1.amazonaws.com` → `Certificate validated` → `MQTT Connected` |
| 6. ผูก Node กับบัญชีผู้ใช้ | `User node mapping done` |
| 7. รายงานสถานะเริ่มต้นขึ้นคลาวด์ | `Reporting params (init): {"Living Room Light":{"Power":false,"Brightness":100},...}` |
| 8. ปิด BLE คืนหน่วยความจำ | `Provisioning stopped` / `BTDM memory released` |

---

## การทดสอบและการตรวจสอบผลลัพธ์

### 1. การควบคุมผ่าน Mobile App

<img width="868" height="1887" alt="image" src="https://github.com/user-attachments/assets/74a698ea-079b-4cf3-904a-5ac6f2ae0fa2" />


<img width="961" height="702" alt="image" src="https://github.com/user-attachments/assets/f71fd298-c6e5-44cc-88ab-8b83405567a1" />


<img width="1108" height="1477" alt="image" src="https://github.com/user-attachments/assets/43d266a5-dba4-412f-babc-c0624908d794" />


| การกระทำในแอป | Log บน Serial Monitor | ผลที่บอร์ด |
|:---|:---|:---|
| เปิดสวิตช์ Power | `Received params: {"Living Room Light":{"Power":true}}` → `Changing LED state to: ON` → `Reporting params` | LED ติด |
| ปิดสวิตช์ Power | `Received params: {"Living Room Light":{"Power":false}}` → `Changing LED state to: OFF` → `Reporting params` | LED ดับ |
| เลื่อน Brightness | `Changing Brightness to: ..%` | รับค่าได้ แต่ความสว่างไม่เปลี่ยน เพราะโค้ดยังไม่ได้ใช้ LEDC PWM |

### 2. การควบคุมระยะไกลข้ามอินเทอร์เน็ต

มือถือเชื่อมต่ออินเทอร์เน็ตผ่าน 4G และปิด Bluetooth ระหว่างทดสอบ ทุกคำสั่งแสดง `Request received via: Cloud` ยืนยันว่าคำสั่งเดินทางจากแอป → RainMaker Cloud (AWS IoT Core, us-east-1) → MQTT → ESP32 ไม่ได้ส่งตรงผ่านเครือข่ายท้องถิ่น

### 3. ปัญหาที่พบและการแก้ไข

| ปัญหา | สาเหตุ | วิธีแก้ |
|:---|:---|:---|
| `add-dependency "espressif/esp-rainmaker"` ไม่พบ component | ชื่อและเวอร์ชันในใบงานไม่ตรงกับ Component Registry ปัจจุบัน | ใช้ `espressif/esp_rainmaker` ^1.17.0 |
| ไม่พบ `app_wifi.h` | ฟังก์ชัน Provisioning ถูกแยกเป็น component และเปลี่ยนชื่อ | เพิ่ม `espressif/rmaker_app_network` และใช้ `app_network.h`, `app_network_init()`, `app_network_start()` |
| ต้องกำหนด Partition Table เอง | RainMaker ต้องใช้พื้นที่สำหรับ OTA และพาร์ติชัน `fctry` เก็บข้อมูลประจำเครื่อง | สร้าง `partitions.csv` และ `sdkconfig.defaults` (Flash 4 MB, เปิด BLE แบบ NimBLE) |
| แอปค้างที่ "Confirming Node Association" และขึ้น `ESPSessionError` | `***ERROR*** A stack overflow in task nimble_host` ขณะบอร์ดคำนวณลายเซ็น ECDSA ในขั้น Challenge-Response ทำให้บอร์ดรีบูต | เพิ่ม `CONFIG_BT_NIMBLE_HOST_TASK_STACK_SIZE=8192` และตั้งค่าตามตัวอย่างทางการ เช่น `CONFIG_BTDM_CTRL_MODE_BLE_ONLY=y`, `CONFIG_MBEDTLS_DYNAMIC_BUFFER=y` |
| `NimBLE: ATT write: invalid handle` | iPhone จำโครงสร้าง GATT ของบอร์ดจากการเชื่อมต่อครั้งก่อน (GATT Cache) | รีสตาร์ต iPhone ก่อนจับคู่ใหม่ |

---

---

### 3. (ทางเลือกขั้นสูง) การทดสอบผ่าน RainMaker CLI บนคอมพิวเตอร์

เปิดเทอร์มินัลใหม่บนคอมพิวเตอร์

```powershell
# ติดตั้งเครื่องมือ CLI
pip install esp-rainmaker-cli

# เข้าสู่ระบบ
rainmaker.py login

# ดึงรายชื่ออุปกรณ์และ Node ID
rainmaker.py getnodes

# สั่งเปิดหลอดไฟผ่าน Command Line
rainmaker.py setparams --nodeid <YOUR_NODE_ID> '{"Living Room Light": {"Power": true}}'

# สั่งปิดหลอดไฟผ่าน Command Line
rainmaker.py setparams --nodeid <YOUR_NODE_ID> '{"Living Room Light": {"Power": false}}'
```

---

## 5. คำถามท้ายการทดลอง (Post-Lab Questions)
1. ในสถาปัตยกรรม ESP RainMaker กระบวนการ **Assisted Claiming** ช่วยแก้ปัญหาเรื่องการติดตั้งใบรับรองความปลอดภัย (Security Certificates) บนอุปกรณ์ในขั้นตอนพัฒนาต้นแบบได้อย่างไร?
```
อุปกรณ์ทุกตัวต้องมี **ใบรับรอง X.509 และกุญแจส่วนตัวเฉพาะตัว** จึงจะเชื่อมต่อ AWS IoT Core ของ RainMaker ด้วย MQTTS ได้ (หลักการเดียวกับ mTLS ในแล็บ 11.3) วิธีดั้งเดิมคือผู้ผลิตต้องสร้างใบรับรองและแฟลชลงพาร์ติชัน `fctry` ของทีละเครื่องผ่านสาย USB ด้วยเครื่องมือเฉพาะ ซึ่งยุ่งยากในขั้นพัฒนาต้นแบบ

Assisted Claiming ใช้**แอปบนมือถือเป็นตัวกลาง** แทน
1. ESP32 สร้างคู่กุญแจและคำร้องขอใบรับรอง (CSR) ขึ้นเองภายในบอร์ด
2. ส่ง CSR ให้แอปผ่าน BLE ที่เข้ารหัสแล้ว
3. แอปซึ่งล็อกอินบัญชีผู้ใช้และมีอินเทอร์เน็ต ส่ง CSR ไปให้ RainMaker Cloud ลงนาม
4. แอปส่งใบรับรองที่ได้กลับไปให้ ESP32 ทาง BLE และบอร์ดบันทึกลงหน่วยความจำ

ข้อดีคือนักพัฒนาไม่ต้องจัดการใบรับรองเอง **กุญแจส่วนตัวไม่เคยออกจากตัวบอร์ด** และอุปกรณ์ที่ยังไม่มีอินเทอร์เน็ตก็ขอใบรับรองได้ตั้งแต่ขั้นจับคู่ครั้งแรก จากการทดลอง RainMaker เวอร์ชันปัจจุบันยังเพิ่มขั้น **Challenge-Response** ให้บอร์ดลงนามด้วยกุญแจ ECDSA เพื่อพิสูจน์ต่อแอปว่าเป็นเจ้าของ Node ID จริงก่อนผูกกับบัญชี (`esp_rmaker_user_node_auth: ECDSA key found`)

```

2. เพราะเหตุใดฟังก์ชัน `write_cb` จึงต้องเรียกคำสั่ง `esp_rmaker_param_update_and_report()` ทุกครั้งที่มีการเปลี่ยนแปลงค่าพารามิเตอร์?
```
ฟังก์ชันนี้ทำ 2 อย่าง คือ อัปเดตค่าในหน่วยความจำของบอร์ด และ รายงานค่าปัจจุบันกลับขึ้นคลาวด์ ซึ่งจำเป็นเพราะ
- ยืนยันสถานะจริง: คำสั่งจากแอปเป็นเพียง "ค่าที่ต้องการ" การรายงานกลับคือการยืนยันว่าบอร์ดเปลี่ยนสถานะสำเร็จแล้ว หากสั่งไม่สำเร็จหรือบอร์ดปรับค่าเอง (เช่น จำกัดช่วง) แอปจะแสดงค่าที่ถูกต้อง
- ซิงก์ทุกช่องทาง: คลาวด์เก็บสถานะล่าสุดของ Node ไว้ ผู้ใช้อื่นที่แชร์อุปกรณ์ แดชบอร์ด ระบบ Schedule หรือ Alexa/Google จะเห็นสถานะเดียวกันทั้งหมด
- ป้องกันสถานะไม่ตรงกัน: ถ้าไม่รายงาน แอปอาจย้อนกลับไปแสดงค่าเดิม หรือเปิดแอปครั้งต่อไปจะเห็นสถานะผิด แม้ LED จะเปลี่ยนแล้ว

จากการทดลองจะเห็นบรรทัด `Reporting params: {"Living Room Light":{"Power":true}}` ตามหลังทุกคำสั่ง ซึ่งเป็นหลักการเดียวกับแล็บ 11.2 ที่ ESP32 ตอบกลับสถานะ LED ทาง topic `iot/node01/state/led`

```
3. เปรียบเทียบความแตกต่างระหว่างการพัฒนาอุปกรณ์ IoT ด้วย **Plain MQTT บน Mosquitto เอง (ในแล็บ 11.2)** กับการใช้ **ESP RainMaker SDK (ในแล็บ 11.4)** ในแง่ของความพยายามในการพัฒนา (Development Effort), ความปลอดภัย (Security), และการขยายระบบ (Scalability)

| ประเด็น | Plain MQTT + Mosquitto (11.2) | ESP RainMaker SDK (11.4) |
|:---|:---|:---|
| **ความพยายามในการพัฒนา** | ต้องตั้ง Broker เอง ออกแบบ Topic, JSON Payload, LWT, การตอบกลับสถานะ และเขียนแอปฝั่งผู้ใช้เองทั้งหมด แต่ยืดหยุ่นและเข้าใจทุกชั้น | ประกาศ Node, Device, Param ด้วย API มาตรฐานไม่กี่บรรทัด ได้แอปมือถือ, Schedule, OTA ทันที แต่ต้องตั้งค่าระบบ (Partition, BLE, หน่วยความจำ) ให้ถูกต้อง |
| **การตั้งค่าเครือข่าย** | Hard-code ชื่อและรหัส Wi-Fi และ IP ของ Broker ในโค้ด ต้องแฟลชใหม่เมื่อเปลี่ยน | BLE Provisioning ผ่านแอป ผู้ใช้ทั่วไปตั้งค่าเองได้ ไม่ต้องแก้โค้ด |
| **ความปลอดภัย** | ใช้พอร์ต 1883 แบบไม่เข้ารหัส และ `allow_anonymous true` ใครอยู่วงเดียวกันก็ดักฟังหรือสั่งงานได้ (ต้องทำ TLS/mTLS เองแบบแล็บ 11.3) | บังคับ MQTTS พร้อมใบรับรองเฉพาะเครื่อง, Provisioning เข้ารหัสด้วย Security 2, ยืนยันเจ้าของด้วย Challenge-Response และสิทธิ์ผูกกับบัญชีผู้ใช้ |
| **การขยายระบบ** | Broker บนคอมเครื่องเดียว ใช้ได้เฉพาะวง LAN หากจะรองรับอุปกรณ์จำนวนมากหรือใช้ผ่านอินเทอร์เน็ต ต้องจัดการเซิร์ฟเวอร์, IP สาธารณะ, ใบรับรอง และการดูแลระบบเอง | ทำงานบน AWS IoT Core รองรับอุปกรณ์จำนวนมากและเข้าถึงได้จากทั่วโลก มี OTA, การแชร์อุปกรณ์ และเชื่อมต่อ Alexa/Google ในตัว แต่ขึ้นกับแพลตฟอร์มของผู้ให้บริการ |
| **เหมาะกับ** | การเรียนรู้กลไกโปรโตคอล ระบบปิดในโรงงาน หรือที่ต้องควบคุมข้อมูลเอง | ผลิตภัณฑ์ผู้บริโภคที่ต้องออกสู่ตลาดเร็ว |

4. อธิบายขั้นตอนที่ทำให้ ESP RainMaker สามารถเชื่อมโยงการทำงานร่วมกับระบบสั่งงานด้วยเสียงของ **Google Home** หรือ **Amazon Alexa** ได้โดยไม่ต้องเขียนโค้ดเพิ่มเติมบนตัวบอร์ด ESP32
```
การเชื่อมต่อเกิดขึ้นในระดับ**คลาวด์ต่อคลาวด์ (Cloud-to-Cloud) ไม่ได้เกิดที่ตัวบอร์ด
1. ใช้ Device Type และ Parameter มาตรฐาน: โค้ดสร้างอุปกรณ์ด้วย `esp_rmaker_lightbulb_device_create()` และ `esp_rmaker_brightness_param_create()` ซึ่งมีชนิดมาตรฐาน `esp.device.lightbulb`, `esp.param.power`, `esp.param.brightness` RainMaker Cloud จึงรู้ว่าอุปกรณ์นี้คือหลอดไฟที่เปิด-ปิดและหรี่ได้
2. RainMaker Cloud แปลงเป็นรูปแบบของผู้ช่วยเสียง: Espressif ทำ Alexa Smart Home Skill และ Google Smart Home Action ไว้แล้ว โดยแมปชนิดมาตรฐานเป็น Trait/Capability ของแต่ละแพลตฟอร์ม เช่น Power → OnOff, Brightness → Brightness
3. ผู้ใช้เชื่อมบัญชี (Account Linking): ในแอป Alexa หรือ Google Home ให้เพิ่ม Skill/บริการ **ESP RainMaker** แล้วล็อกอินด้วยบัญชีเดียวกับแอป RainMaker
4. ค้นหาอุปกรณ์ (Discovery): Alexa/Google ดึงรายการอุปกรณ์จาก RainMaker Cloud จะเห็น "Living Room Light" ทันที
5. สั่งงานด้วยเสียง: เช่น "Alexa, turn on Living Room Light" คำสั่งเดินทาง Alexa Cloud → RainMaker Cloud → MQTT → ESP32 แล้วเข้า `write_cb` เดียวกับที่ใช้รับคำสั่งจากแอป บอร์ดรายงานสถานะกลับด้วย `esp_rmaker_param_update_and_report()` ผู้ช่วยเสียงจึงเห็นสถานะล่าสุด

เนื่องจากบอร์ดคุยกับ RainMaker Cloud ด้วยรูปแบบเดิมเสมอ การเพิ่ม Alexa หรือ Google จึงไม่ต้องแก้เฟิร์มแวร์ ขอเพียงใช้ Device Type และ Parameter มาตรฐานตั้งแต่ต้น
```
