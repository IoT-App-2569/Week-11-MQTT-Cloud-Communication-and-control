# การส่งงานสัปดาห์ที่ 11: MQTT Cloud Communication and Control

* **รหัสนักศึกษา:** 67030011
* **ชื่อ:** Krittanai Boonnoy (กฤตนัย บุญน้อย) — SIET
* **Git Branch:** `HW-67030011`
* **สภาพแวดล้อม:** macOS, ESP-IDF v6.0.2 (target `esp32`, บอร์ด ESP32-D0WD-V3), Mosquitto 2.0.22, OpenSSL 4.0.3
* **เครือข่าย:** iPhone Personal Hotspot `KBBK-IPHONE` (2.4 GHz) — Mac/Broker `172.20.10.2`, ESP32 `172.20.10.6`
* **วงจร:** LED บนบอร์ด GPIO 2, Potentiometer 10k ขากลาง → GPIO 34

---

## สารบัญงานและรายงานผลการทดลอง

### 1. ใบงาน 11.1 — Mosquitto Broker & MQTT Fundamentals
- [06-Labsheet-11-1-Mosquitto-Broker-and-MQTT-Fundamentals.md](06-Labsheet-11-1-Mosquitto-Broker-and-MQTT-Fundamentals.md)
- ทดสอบ Pub/Sub, Wildcard `+`/`#`, QoS 0/1/2 (ตารางลำดับแพ็กเก็ตจาก Broker Log: 2 / 4 / 8 แพ็กเก็ต), Retained Message (`r0` vs `r1`), LWT (SIGKILL vs DISCONNECT)
- คำถามข้อ 1 ทดสอบส่ง SUBSCRIBE `home/#/light` ตรงถึง Broker → `disconnected due to malformed packet`

### 2. ใบงาน 11.2 — ESP32 MQTT Client Telemetry & Control
- [07-Labsheet-11-2-ESP32-MQTT-Client-Telemetry-and-Control.md](07-Labsheet-11-2-ESP32-MQTT-Client-Telemetry-and-Control.md)
- Telemetry JSON ทุก 4 วินาที (ADC Calibration, หมุน Pot ได้ 0.142–3.139 V), คำสั่ง LED แบบ `ON`/`OFF`/JSON, LWT `offline` หลังถอดสาย (90 s = 1.5 × Keep Alive), ทดสอบ Auto-Reconnect ของ esp-mqtt

### 3. ใบงาน 11.3 — MQTTS / TLS Mutual Authentication
- [08-Labsheet-11-3-MQTTS-TLS-Mutual-Authentication-Security.md](08-Labsheet-11-3-MQTTS-TLS-Mutual-Authentication-Security.md)
- พบและแก้ปัญหาคำสั่งสร้าง Root CA กับ OpenSSL ≥ 3.2 (ต้องมี `basicConstraints CA:TRUE`), Mosquitto 2.x ต้อง `allow_anonymous true`, ESP32 เชื่อมต่อ mTLS สำเร็จ (Handshake ≈ 1.9 s), พิสูจน์ว่าดึง `client.key` จาก .bin และจาก Flash ของบอร์ดได้

### 4. ใบงาน 11.4 — ESP RainMaker Smart Light
- [09-Labsheet-11-4-ESP-RainMaker-Cloud-IoT-and-Smart-Device.md](09-Labsheet-11-4-ESP-RainMaker-Cloud-IoT-and-Smart-Device.md)
- ใช้ `espressif/esp_rainmaker` 1.17.0 + `rmaker_app_network`, Brightness ขับด้วย LEDC PWM, BLE Provisioning + Assisted Claiming สำเร็จ (Node ID `UE8xRB8zb33cRLRySRcdEW`), ควบคุม Power/Brightness จากแอปผ่าน RainMaker Cloud ได้ (`Request received via: Cloud`)
- แก้ปัญหา Claim ซ้ำแล้วบอร์ด Crash, iOS Cache ตาราง GATT และ iPhone ที่เป็น Hotspot เอง (ตั้ง Wi-Fi ผ่านคำสั่ง Console `wifi-prov`)

---

## รายการซอร์สโค้ด

| ใบงาน | โปรเจกต์ / ไฟล์ | ไฟล์หลัก |
| :--- | :--- | :--- |
| 11.1 | [Lab11-1_Mosquitto_CLI/](Lab11-1_Mosquitto_CLI/) | [mosquitto.conf](Lab11-1_Mosquitto_CLI/mosquitto.conf), [lab11_1_cli_tests.sh](Lab11-1_Mosquitto_CLI/lab11_1_cli_tests.sh), [raw_invalid_subscribe.py](Lab11-1_Mosquitto_CLI/raw_invalid_subscribe.py) |
| 11.2 | [Lab11-2_MQTT_Client/](Lab11-2_MQTT_Client/) | [main/Lab11-2_MQTT_Client.c](Lab11-2_MQTT_Client/main/Lab11-2_MQTT_Client.c) |
| 11.3 | [Lab11-3_MQTTS_Secure/](Lab11-3_MQTTS_Secure/) | [main/main.c](Lab11-3_MQTTS_Secure/main/main.c), [mqtt_certs/gen_certs.sh](Lab11-3_MQTTS_Secure/mqtt_certs/gen_certs.sh), [mqtt_certs/mosquitto_ssl.conf](Lab11-3_MQTTS_Secure/mqtt_certs/mosquitto_ssl.conf) |
| 11.4 | [Lab11-4_RainMaker_Light/](Lab11-4_RainMaker_Light/) | [main/main.c](Lab11-4_RainMaker_Light/main/main.c), [partitions.csv](Lab11-4_RainMaker_Light/partitions.csv), [sdkconfig.defaults](Lab11-4_RainMaker_Light/sdkconfig.defaults) |

ภาพผลการทดลองทั้งหมดอยู่ใน [Images/](Images/) และ Log ดิบของแต่ละใบงานอยู่ในโฟลเดอร์ `logs/` ของโปรเจกต์นั้น

### วิธี Build / Flash
```bash
cd HW-67030011/Lab11-2_MQTT_Client
cp main/wifi_credentials.h.example main/wifi_credentials.h   # แล้วใส่ SSID / Password (Wi-Fi 2.4 GHz)
idf.py set-target esp32
idf.py -p <PORT> flash monitor
```
* ใบงาน 11.3 ต้องสร้างใบรับรองก่อน: `cd Lab11-3_MQTTS_Secure/mqtt_certs && ./gen_certs.sh <IP ของ Broker>` (สคริปต์คัดลอก `ca.crt`, `client.crt`, `client.key` ไปที่ `main/certs/` ให้)
* `main/wifi_credentials.h`, `*.key` และ `*.csr` อยู่ใน `.gitignore` เพื่อไม่ให้รหัส Wi-Fi และกุญแจส่วนตัวขึ้น GitHub
