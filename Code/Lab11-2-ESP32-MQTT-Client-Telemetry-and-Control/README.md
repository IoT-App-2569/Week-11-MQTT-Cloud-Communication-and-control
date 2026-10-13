# Lab 11-2 – ESP32 MQTT Client: Telemetry และควบคุม LED

โปรเจกต์ ESP-IDF ที่ทำให้ ESP32 เป็น MQTT Client สองทาง
- **Publish** ค่า Potentiometer (ADC) เป็น JSON ทุก 4 วินาที → `iot/node01/telemetry`
- **Subscribe** คำสั่งควบคุม LED ← `iot/node01/cmd/led` (ตอบสถานะที่ `iot/node01/state/led`)
- ส่ง `iot/node01/status = online` (retain) และตั้ง LWT `offline` (retain, QoS 1)

## โครงสร้างไฟล์

| ไฟล์ | หน้าที่ |
|---|---|
| `CMakeLists.txt` | ไฟล์โปรเจกต์ระดับบน |
| `main/CMakeLists.txt` | ลงทะเบียนซอร์สและ `REQUIRES` (mqtt, cjson, esp_hw_support ฯลฯ) |
| `main/idf_component.yml` | Dependency: `espressif/mqtt`, `espressif/cjson` |
| `main/Lab11-2_MQTT_Client.c` | โปรแกรมหลัก |

## 1. ต่อวงจร

| อุปกรณ์ | ขาต่อ ESP32 | ESP32-C3 |
|---|---|---|
| LED (Anode ผ่านตัวต้านทาน 330/470 Ω) | GPIO 2 | GPIO 8 |

> ไม่มี Potentiometer: โค้ดจำลองค่า `adc_raw` ด้วยเลขสุ่ม 0–4095 (`esp_random()`) และคำนวณ `voltage_v` ให้เหมือนเดิม จึงไม่ต้องต่อวงจรส่วนเซนเซอร์

## 2. ตั้งค่า Wi-Fi / Broker ในไฟล์ `.env`

ค่าทั้งหมดอยู่ในไฟล์ `.env` (โฟลเดอร์เดียวกับ README นี้) ไม่ต้องแก้โค้ด C:

```env
WIFI_SSID=Bismarck .
WIFI_PASS=jobshare
MQTT_BROKER_URI=mqtt://10.131.32.114:1883
```

- `MQTT_BROKER_URI` = IP ของคอมพิวเตอร์ที่รัน Broker (`ipconfig` → IPv4 Address ของ Wi-Fi); ถ้า IP เปลี่ยนต้องแก้ไฟล์นี้แล้ว build ใหม่
- ไม่ต้องใส่เครื่องหมายคำพูด ช่องว่างใน SSID ใช้ได้ตามปกติ
- `main/CMakeLists.txt` อ่าน `.env` ตอน build แล้วส่งเป็น `-DWIFI_SSID=...` ให้คอมไพเลอร์
- ถ้าไม่มี `.env` ให้คัดลอกจาก `.env.example`
- `.env` อยู่ใน `.gitignore` จึงไม่ถูก commit ขึ้น Git
- ESP32 และคอมพิวเตอร์ต้องอยู่ Wi-Fi วงเดียวกัน (ESP32 รองรับเฉพาะ 2.4 GHz)

## 3. เตรียม Broker (เครื่องคอมพิวเตอร์)

ใช้ `mosquitto.conf` จาก Lab 11-1 (`listener 1883 0.0.0.0`, `allow_anonymous true`) เพราะ ESP32 ต้องเชื่อมผ่านวง LAN

```powershell
# PowerShell (Admin) ครั้งเดียว: ปิด Windows Service กัน Broker ซ้ำซ้อน
Stop-Service mosquitto; Set-Service mosquitto -StartupType Manual

# เปิดพอร์ต 1883 ให้ Firewall (ครั้งเดียว)
New-NetFirewallRule -DisplayName "Mosquitto 1883" -Direction Inbound -Protocol TCP -LocalPort 1883 -Action Allow
```

```powershell
# Terminal 1: เปิด Broker
Set-ExecutionPolicy -Scope Process Bypass
cd C:\IOT\Week-11-MQTT-Cloud-Communication-and-control\Code\Lab11-1-Mosquitto-Broker-and-MQTT-Fundamentals
.\lab11-1.ps1 broker
```

## 4. Build และ Flash

เปิด ESP-IDF PowerShell (หรือ ESP-IDF Terminal) แล้ว:

```powershell
cd C:\IOT\Week-11-MQTT-Cloud-Communication-and-control\Code\Lab11-2-ESP32-MQTT-Client-Telemetry-and-Control

idf.py set-target esp32        # ESP32-C3 ใช้ esp32c3
idf.py build
idf.py -p COM3 flash monitor   # เปลี่ยน COM3 เป็นพอร์ตของบอร์ด
```

ครั้งแรก `idf.py build` จะดาวน์โหลด `espressif/mqtt` และ `espressif/cjson` อัตโนมัติจาก `idf_component.yml`
(ออกจาก monitor ด้วย `Ctrl+]`)

ที่ Serial Monitor ควรเห็น:

```text
I (...) LAB11_MQTT: Got IP Address: 192.168.1.xxx
I (...) LAB11_MQTT: MQTT_EVENT_CONNECTED: Connected to Broker!
I (...) LAB11_MQTT: Subscribed to: iot/node01/cmd/led
I (...) LAB11_MQTT: Telemetry Published: {"node_id":"esp32-node01",...}
```

## 5. ทดสอบ (เปิดเทอร์มินัลใหม่บนคอมพิวเตอร์)

แทน `192.168.1.xxx` ด้วย IP ของเครื่อง (หรือใช้ `127.0.0.1` เมื่อปิด Windows Service แล้ว)

### 5.1 ดู Telemetry

```powershell
mosquitto_sub -h 192.168.1.xxx -p 1883 -t "iot/node01/telemetry" -v
```

ได้ JSON ทุก 4 วินาที — `adc_raw` (สุ่ม 0–4095) และ `voltage_v` จะเปลี่ยนค่าทุกรอบ

### 5.2 สั่ง LED

```powershell
mosquitto_pub -h 192.168.1.xxx -p 1883 -t "iot/node01/cmd/led" -m "ON"  -q 1
mosquitto_pub -h 192.168.1.xxx -p 1883 -t "iot/node01/cmd/led" -m "OFF" -q 1
mosquitto_pub -h 192.168.1.xxx -p 1883 -t "iot/node01/cmd/led" -m '{"state": 1}' -q 1
```

> ใน PowerShell 5.1 การส่ง JSON ให้ใช้รูปแบบ `-m '{\"state\": 1}'` (มี `\"` ภายใน single quote) เพื่อให้เครื่องหมายคำพูดไม่หาย

LED จะติด/ดับทันที และ Telemetry รอบถัดไปมี `"led_state"` ตรงกัน

### 5.3 ดูสถานะ / ACK (เสริม)

```powershell
mosquitto_sub -h 192.168.1.xxx -p 1883 -t "iot/node01/#" -v
```

- `iot/node01/status online` (retain) ปรากฏทันทีที่ Subscribe
- กด reset หรือถอดไฟบอร์ด → หลัง keepalive หมด Broker ส่ง `iot/node01/status offline` (LWT)

## Troubleshooting

| อาการ | สาเหตุ / วิธีแก้ |
|---|---|
| ESP32 ไม่ได้ IP | SSID/รหัสผ่านผิด หรือเป็น Wi-Fi 5 GHz |
| `MQTT_EVENT_DISCONNECTED` ซ้ำๆ | IP Broker ผิด, Broker ไม่ได้รัน, หรือ Firewall บล็อกพอร์ต 1883 |
| `mosquitto_pub` ส่งแล้ว LED ไม่ติด | Windows Service `mosquitto` ยังรัน (split-brain) → ปิดตามข้อ 3 และใช้ IP LAN เดียวกับที่ ESP32 ใช้ |
| Build ไม่พบ `mqtt_client.h` | ยังไม่ได้ดาวน์โหลด dependency → รัน `idf.py reconfigure` ขณะต่ออินเทอร์เน็ต |
