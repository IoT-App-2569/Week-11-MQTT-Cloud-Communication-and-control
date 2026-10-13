# Lab 11-4 – ESP RainMaker Smart Light (BLE Provisioning + Cloud Control)

สร้าง Node + Device (Lightbulb) พร้อมพารามิเตอร์ Power / Brightness บน **ESP RainMaker**
ทำ BLE Wi-Fi Provisioning และ Assisted Claiming ผ่านแอป แล้วควบคุม LED จากที่ไหนก็ได้ผ่านคลาวด์

## โครงสร้างไฟล์

| ไฟล์ | หน้าที่ |
|---|---|
| `main/main.c` | โปรแกรมหลัก (Node, Device, write callback, schedule/timezone) |
| `main/idf_component.yml` | Dependency: `espressif/esp_rainmaker`, `espressif/rmaker_app_network` |
| `sdkconfig.defaults` | เปิด BLE (NimBLE), flash 4MB และใช้ `partitions.csv` |
| `partitions.csv` | ตารางพาร์ติชันที่มี `fctry` (NVS ของ RainMaker) + ota_0/ota_1 |

## ข้อแตกต่างจากใบงาน (ต้องรู้)

| ใบงาน | โค้ดนี้ | เหตุผล |
|---|---|---|
| `idf.py add-dependency "espressif/esp-rainmaker"` | `espressif/esp_rainmaker` | ชื่อจริงใน Component Registry ใช้ขีดล่าง (ชื่อแบบมีขีดกลางหาไม่พบ) |
| `#include "app_wifi.h"` | `#include "app_network.h"` | `app_wifi` เป็นโค้ดตัวอย่างในรีโป RainMaker ไม่ได้อยู่ใน registry ตัวที่ใช้แทนคือคอมโพเนนต์ `espressif/rmaker_app_network` |
| `app_wifi_init()` / `app_wifi_start(POP_TYPE_RANDOM)` | `app_network_init()` / `app_network_start(POP_TYPE_RANDOM)` | API ใหม่ของคอมโพเนนต์ข้างบน |

## 0. เตรียมครั้งเดียว

- มีบัญชี ESP RainMaker (สมัครในแอป **ESP RainMaker** บนมือถือ) และมือถือเปิด Bluetooth + Wi-Fi/4G
- ใช้ Wi-Fi **2.4 GHz** ที่ออกอินเทอร์เน็ตได้จริง (ESP32 ไม่รองรับ 5 GHz)
- บอร์ด ESP32 (LED ที่ GPIO 2) หรือ ESP32-C3 (LED ที่ GPIO 8) ต่อ LED + R 220 Ω

## 1. Build และ Flash

เปิด ESP-IDF PowerShell แล้ว (ใช้ `-B` โฟลเดอร์สั้นเพื่อเลี่ยงปัญหา path ยาวบน Windows):

```powershell
cd C:\IOT\Week-11-MQTT-Cloud-Communication-and-control\Code\Lab11-4-ESP-RainMaker-Cloud-IoT-and-Smart-Device

idf.py -B C:\b\l114 set-target esp32        # ESP32-C3 ใช้ esp32c3
idf.py -B C:\b\l114 build
idf.py -B C:\b\l114 -p COM3 erase-flash     # ครั้งแรก/เปลี่ยนโปรเจกต์ เพื่อล้างค่า Wi-Fi เก่า (ไม่บังคับ)
idf.py -B C:\b\l114 -p COM3 flash monitor
```

ครั้งแรก component manager จะดาวน์โหลด `esp_rainmaker`, `rmaker_app_network` และ dependency ต่างๆ (ใช้เวลาหลายนาที)

## 2. สังเกต Serial Monitor (11-4.3)

- มี **QR Code แบบ ASCII** และ URL สำหรับเปิดรูป QR
- ข้อความ `Provisioning started` / BLE advertising รอการเชื่อมต่อ
- ถ้า QR ใน terminal สแกนไม่ติด ให้เปิด URL ที่แสดงบนเบราว์เซอร์แล้วสแกนจากหน้าจอแทน

## 3. Provisioning ผ่านแอป (11-4.4)

1. เปิดแอป **ESP RainMaker** → Login
2. กด **Add Device** → สแกน QR Code จาก Serial Monitor
3. อนุญาตขอรับใบรับรอง (**Assisted Claiming**) – แอปดึง Certificate จาก RainMaker Cloud แล้วส่งให้บอร์ดผ่าน BLE
4. เลือก SSID 2.4 GHz และกรอกรหัสผ่าน
5. แอปแสดงการ์ด **Living Room Light**

## 4. ทดสอบ

| ทดสอบ | วิธี | ผลที่คาดหวัง |
|---|---|---|
| Power | กดสวิตช์ในแอป | LED ติด/ดับ และ Serial แสดง `Changing LED state to: ON/OFF` |
| Brightness | เลื่อนสไลเดอร์ | Serial แสดง `Changing Brightness to: NN%` |
| Remote | ปิด Wi-Fi มือถือ ใช้ 4G/5G | บอร์ดยังตอบสนอง (ผ่านคลาวด์) |

> LED ในโค้ดนี้เป็น GPIO ดิจิทัล: Power เปิด/ปิดได้ ส่วน Brightness แสดงเฉพาะค่าใน log (ตามใบงาน) ถ้าต้องการหรี่จริงให้เพิ่ม LEDC PWM

### CLI (ทางเลือก)

```powershell
pip install esp-rainmaker-cli
rainmaker.py login
rainmaker.py getnodes
rainmaker.py setparams --nodeid <YOUR_NODE_ID> '{"Living Room Light": {"Power": true}}'
rainmaker.py setparams --nodeid <YOUR_NODE_ID> '{"Living Room Light": {"Power": false}}'
```

บน PowerShell ถ้า JSON ไม่ผ่าน (เครื่องหมายคำพูดหาย) ให้ใช้รูปแบบ `'{\"Living Room Light\": {\"Power\": true}}'`

## Troubleshooting

| อาการ | สาเหตุ / วิธีแก้ |
|---|---|
| ไม่พบ `espressif/esp-rainmaker` | ใช้ชื่อ `espressif/esp_rainmaker` (ขีดล่าง) |
| `app_wifi.h: No such file` | ใช้ `app_network.h` + `espressif/rmaker_app_network` (ดูตารางด้านบน) |
| แอปไม่เจออุปกรณ์ | เปิด Bluetooth/Location บนมือถือ, อยู่ใกล้บอร์ด, กด EN/RST แล้วสแกน QR ใหม่ |
| Provisioning แล้วต่อ Wi-Fi ไม่ได้ | SSID เป็น 5 GHz หรือรหัสผ่านผิด → `erase-flash` แล้วทำใหม่ |
| `esp_rmaker_fctry: NVS Flash init failed` / `Failed to initialise storage` / `Could not initialise node` | ตารางพาร์ติชันไม่มี `fctry` → ต้องใช้ `partitions.csv` ของโปรเจกต์นี้ (ลบ `sdkconfig` แล้ว `reconfigure`/`build` ใหม่ และ flash ใหม่ รวมถึงตารางพาร์ติชัน) |
| ขนาด app ใหญ่เกิน partition | `partitions.csv` ให้ ota_0/ota_1 ละ ~1.8 MB สำหรับ flash 4MB (ลบ `sdkconfig` แล้ว build ใหม่) |
| ต้องการล้างการผูกอุปกรณ์/ค่า Wi-Fi | `idf.py -B C:\b\l114 -p COM3 erase-flash` แล้ว flash ใหม่ (โค้ดนี้ไม่มีปุ่ม reset) และลบอุปกรณ์ออกจากแอปด้วย |
