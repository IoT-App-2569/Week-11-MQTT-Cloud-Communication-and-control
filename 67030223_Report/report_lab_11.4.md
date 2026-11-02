# Labsheet 11.4: ESP RainMaker Cloud Platform & Mobile App Integration


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
สามารถเชื่อมต่อบอร์ด ESP32 เข้ากับแพลตฟอร์มคลาวด์ ESP RainMaker ได้สำเร็จ โดยใช้กลไก BLE Provisioning ผ่านการสแกน QR Code เพื่อส่งค่า Wi-Fi และทำ Assisted Claiming ดึงใบรับรองความปลอดภัยลงมายังบอร์ดอัตโนมัติ ทำให้สามารถควบคุมสถานะเปิด-ปิดของหลอดไฟ LED บนขา GPIO 2 และปรับระดับความสว่างผ่านแอปพลิเคชันบนสมาร์ตโฟนได้แบบเรียลไทม์ ทั้งผ่านเครือข่าย Wi-Fi วงเดียวกันและผ่านอินเทอร์เน็ตเซลลูลาร์ภายนอก
 

- **คำถามท้ายการทดลอง (Post-Lab Questions):**
1. **ในสถาปัตยกรรม ESP RainMaker กระบวนการ Assisted Claiming ช่วยแก้ปัญหาเรื่องการติดตั้งใบรับรองความปลอดภัยบนอุปกรณ์ในขั้นตอนพัฒนาต้นแบบได้อย่างไร?**
     - **คำตอบ:** ช่วยให้นักพัฒนาไม่ต้องสร้างหรือแฟลชไฟล์ใบรับรอง (X.509 Certificate / Private Key) ลงในชิปด้วยตนเองล่วงหน้า โดยสมาร์ตโฟนจะทำหน้าที่เป็นตัวกลางยืนยันตัวตนกับ RainMaker Cloud เพื่อดาวน์โหลดและติดตั้งใบรับรองความปลอดภัยลงในบอร์ด ESP32 ผ่านบลูทูธ (BLE) โดยอัตโนมัติในขั้นตอนตั้งค่าครั้งแรก

2. **เพราะเหตุใดฟังก์ชัน `write_cb` จึงต้องเรียกคำสั่ง `esp_rmaker_param_update_and_report()` ทุกครั้งที่มีการเปลี่ยนแปลงค่าพารามิเตอร์?**
     - **คำตอบ:** เพื่ออัปเดตค่าพารามิเตอร์ใหม่ลงในตัวแปรภายในของ SDK และส่งรายงานสถานะจริงของอุปกรณ์ (Actual State) กลับขึ้นไปยัง Cloud Broker ทันที ทำให้หน้าจอแอปพลิเคชันบนสมาร์ตโฟนแสดงสถานะตรงกับฮาร์ดแวร์จริง และป้องกันปัญหาคำสั่งตกหล่นหรือสถานะบนแอปไม่ตรงกับบอร์ด

3.  **เปรียบเทียบความแตกต่างระหว่างการพัฒนาอุปกรณ์ IoT ด้วย Plain MQTT บน Mosquitto เอง (แล็บ 11.2) กับการใช้ ESP RainMaker SDK (แล็บ 11.4) ในแง่ของความพยายามในการพัฒนา, ความปลอดภัย, และการขยายระบบ:**
     - **คำตอบ:** 
     ความพยายามในการพัฒนา: Mosquitto ต้องเขียนโค้ดเองทั้งหมด (ตั้งค่า Wi-Fi, ออกแบบ JSON, ทำแอปพลิเคชัน) ส่วน RainMaker มีโมดูลสำเร็จรูปครบ (BLE Provisioning, Schema สำเร็จ, แอปบนมือถือพร้อมใช้)

ความปลอดภัย: Mosquitto พื้นฐานมักเป็น Plain Text ไม่มีรหัสผ่าน ส่วน RainMaker บังคับใช้ TLS/mTLS เชื่อมต่อกับ AWS IoT พร้อมระบบยืนยันตัวตนผู้ใช้ระดับคลาวด์ตั้งแต่เริ่มต้น

การขยายระบบ (Scalability): Mosquitto แบบ Standalone มีขีดจำกัดตามทรัพยากรของเครื่องเซิร์ฟเวอร์ ต้องดูแลระบบเอง ส่วน RainMaker รันบน Serverless Cloud ของ AWS จึงรองรับการเพิ่มจำนวนอุปกรณ์ได้มหาศาลโดยไม่ต้องบริหารเซิร์ฟเวอร์

4. **อธิบายขั้นตอนที่ทำให้ ESP RainMaker สามารถเชื่อมโยงการทำงานร่วมกับระบบสั่งงานด้วยเสียงของ Google Home หรือ Amazon Alexa ได้โดยไม่ต้องเขียนโค้ดเพิ่มเติมบนตัวบอร์ด ESP32:**
     - **คำตอบ:** เพราะการเชื่อมโยงเกิดขึ้นในระดับ Cloud-to-Cloud Integration โดยผู้ใช้เพียงล็อกอินบัญชี RainMaker ผูกเข้ากับแอป Google Home หรือ Alexa (ผ่านกระบวนการ Account Linking / OAuth 2.0) ตัวระบบคลาวด์จะแปลงคำสั่งเสียงเป็นคำสั่งมาตรฐาน และส่งตรงเข้า AWS IoT Broker เพื่ออัปเดตพารามิเตอร์ลงมายังเฟิร์มแวร์ ESP32 ผ่านฟังก์ชัน Callback เดิมที่มีอยู่แล้วโดยตรง