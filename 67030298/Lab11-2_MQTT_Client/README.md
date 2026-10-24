# Lab 11.2: ESP32 MQTT Client Telemetry and Control

โปรเจกต์ ESP-IDF สำหรับบอร์ด ESP32 / ESP32-C3 ทำงานเป็น Bi-directional MQTT Client:
- Publish ค่าอนาล็อก (Potentiometer) เป็น JSON Telemetry สู่หัวข้อ `iot/node01/telemetry` ทุกๆ 4 วินาที
- Subscribe รอรับคำสั่งควบคุม LED สู่หัวข้อ `iot/node01/cmd/led` ("ON", "OFF" หรือ `{"state": 1/0}`) และรายงานสถานะกลับที่ `iot/node01/state/led`
- รายงานสถานะออนไลน์ `iot/node01/status` = `online` (Retain=1) และตั้ง LWT = `offline`

## โครงสร้างโปรเจกต์
```
Lab11-2_MQTT_Client/
├── CMakeLists.txt
├── README.md
└── main/
    ├── CMakeLists.txt
    ├── idf_component.yml
    └── Lab11-2_MQTT_Client.c
```

## การกำหนดค่าและคอมไพล์
1. แก้ไขไฟล์ `main/Lab11-2_MQTT_Client.c`:
   - `WIFI_SSID`: ชื่อ Wi-Fi
   - `WIFI_PASS`: รหัสผ่าน Wi-Fi
   - `MQTT_BROKER_URI`: IP ของคอมพิวเตอร์ที่รัน Mosquitto Broker เช่น `mqtt://192.168.1.100:1883`

2. คอมไพล์และแฟลช:
```powershell
idf.py set-target esp32c3   # หรือ esp32
idf.py build
idf.py -p COMx flash monitor
```

## คำสั่งทดสอบ CLI

### ตรวจสอบ Telemetry
```powershell
mosquitto_sub -h <BROKER_IP> -p 1883 -t "iot/node01/telemetry" -v
```

### สั่งเปิด-ปิดไฟ LED
```powershell
# เปิดไฟ
mosquitto_pub -h <BROKER_IP> -p 1883 -t "iot/node01/cmd/led" -m "ON" -q 1

# ปิดไฟ
mosquitto_pub -h <BROKER_IP> -p 1883 -t "iot/node01/cmd/led" -m "OFF" -q 1

# สั่งด้วย JSON
mosquitto_pub -h <BROKER_IP> -p 1883 -t "iot/node01/cmd/led" -m "{\"state\": 1}" -q 1
```
