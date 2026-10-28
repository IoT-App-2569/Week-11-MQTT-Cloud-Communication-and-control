# Lab 11-3 – MQTTS (TLS) และ Mutual Authentication (mTLS)

สร้าง Root CA / Server / Client certificate ด้วย OpenSSL → ตั้ง Mosquitto ให้เปิดพอร์ต **8883** พร้อมบังคับ
`require_certificate true` → ให้ ESP32 เชื่อมต่อด้วย `mqtts://` โดยฝัง `ca.crt`, `client.crt`, `client.key` ลงในเฟิร์มแวร์

## โครงสร้างไฟล์

| ไฟล์ | หน้าที่ |
|---|---|
| `lab11-3.ps1` | สคริปต์ช่วยรันทุกขั้นตอน (สร้างใบรับรอง, Broker, ทดสอบ CLI) |
| `.env` / `.env.example` | `WIFI_SSID`, `WIFI_PASS`, `MQTTS_BROKER_URI` ของ ESP32 |
| `certs/` | (สร้างโดยสคริปต์ ไม่ commit) `ca.*`, `server.*`, `client.*`, `mosquitto_ssl.conf` |
| `main/certs/` | (สร้างโดยสคริปต์ ไม่ commit) `ca.crt`, `client.crt`, `client.key` ที่ฝังลงเฟิร์มแวร์ |
| `main/main.c` | โปรแกรม ESP32 (Wi-Fi + MQTTS mTLS) |
| `main/CMakeLists.txt` | `EMBED_TXTFILES` ฝังใบรับรอง + อ่านค่า `.env` |

## 0. เตรียมครั้งเดียว

1. ติดตั้ง Mosquitto และ OpenSSL (Git for Windows มี `openssl.exe` อยู่แล้ว สคริปต์หาให้เอง)
2. PowerShell **Admin**: ปิด Windows Service และเปิดพอร์ต 8883

```powershell
Stop-Service mosquitto; Set-Service mosquitto -StartupType Manual
New-NetFirewallRule -DisplayName "Mosquitto 8883" -Direction Inbound -Protocol TCP -LocalPort 8883 -Action Allow
```

3. ทุกหน้าต่าง PowerShell:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
cd C:\IOT\Week-11-MQTT-Cloud-Communication-and-control\Code\Lab11-3-MQTTS-TLS-Mutual-Authentication-Security
```

## 1. กิจกรรม 11-3.1 – สร้างใบรับรอง

```powershell
.\lab11-3.ps1 certs -BrokerIp 10.131.32.114    # IP ของคอมพิวเตอร์ (ดูจาก ipconfig)
.\lab11-3.ps1 show                              # แสดง CN / อายุ / ผู้ลงนาม และตรวจ chain
```

- ค่าเริ่มต้นของ `-BrokerIp` คือ `10.131.32.114` ถ้า IP เครื่องเปลี่ยน ให้รัน `certs` ใหม่ **และ** แก้ `MQTTS_BROKER_URI` ใน `.env` ให้เป็น IP เดียวกัน
- ใบรับรองเซิร์ฟเวอร์ใส่ทั้ง **CN** และ **SAN (IP)** เป็น IP ของ Broker เพื่อให้ตรวจ hostname ผ่านทั้ง OpenSSL และ mbedTLS
- ใช้ X.509 v3 extension (`CA:TRUE` สำหรับ CA, `serverAuth` / `clientAuth`) ซึ่งต่างจากคำสั่งในใบงานเล็กน้อย เพื่อให้ mbedTLS ใน ESP-IDF 6 ยอมรับ

## 2. กิจกรรม 11-3.2 – เปิด Broker MQTTS (Terminal 1)

```powershell
.\lab11-3.ps1 broker
```

สคริปต์ใช้ `certs\mosquitto_ssl.conf` ที่สร้างให้ (พอร์ต 8883, `require_certificate true`, `tls_version tlsv1.2`)

> Mosquitto 2.x ปฏิเสธ client ที่ไม่มี username/password ด้วย `Connection Refused: not authorised` แม้ใบรับรองถูกต้อง
> จึงเพิ่ม `use_identity_as_username true` และ `allow_anonymous true` ลงในคอนฟิก (ใบรับรอง client ยังถูกบังคับเสมอ)

## 3. กิจกรรม 11-3.3 – ทดสอบ CLI

| Terminal | คำสั่ง | ผลที่คาดหวัง |
|---|---|---|
| 2 | `.\lab11-3.ps1 sub` | รอรับข้อความ |
| 3 | `.\lab11-3.ps1 pub` | Terminal 2 ได้ `secure/test Hello Secure World!` |
| 3 | `.\lab11-3.ps1 attack` | **ถูกปฏิเสธ** (ไม่ส่ง client cert) – `The connection was lost` / `ssl handshake failure` และ Broker log แจ้ง TLS error |
| 3 | `.\lab11-3.ps1 plain` | ต่อพอร์ต 1883 ไม่ได้ (Broker ไม่ฟังพอร์ตนี้) – `Connection refused` |

ข้อความ error ที่ Terminal 3 และ Broker log อาจต่างกันเล็กน้อยตามเวอร์ชัน OpenSSL ให้แคปตามที่เห็นจริง

## 4. กิจกรรม 11-3.4/5 – ESP32

1. ตรวจ `.env` (SSID, รหัสผ่าน, `MQTTS_BROKER_URI=mqtts://<IP>:8883`)
2. ต้องมี `main/certs/ca.crt`, `client.crt`, `client.key` (สร้างโดย `.\lab11-3.ps1 certs`)
3. Build และ flash (ใช้ `-B` โฟลเดอร์สั้นเพื่อเลี่ยงปัญหา path ยาวบน Windows):

```powershell
idf.py set-target esp32          # ESP32-C3 ใช้ esp32c3
idf.py -B C:\b\l113 build
idf.py -B C:\b\l113 -p COM3 flash monitor
```

ที่ Serial Monitor ควรเห็น:

```text
MQTTS_SECURE: Got IP Address: 10.131.x.x
MQTTS_SECURE: Wi-Fi Connected! Initializing Secure MQTTS Client with mTLS...
MQTTS_SECURE: MQTTS TLS Handshake Successful! Connected to Secure Broker.
```

ที่ Broker log ควรเห็นการเชื่อมต่อของ client `esp32_mtls_client_node` (username = `esp32-secure-node`)
ทดสอบสั่งบอร์ดได้ด้วย:

```powershell
$env:Path += ";C:\Program Files\mosquitto"
mosquitto_sub -h 10.131.32.114 -p 8883 -t "secure/esp32/#" --cafile certs\ca.crt --cert certs\client.crt --key certs\client.key -v
mosquitto_pub -h 10.131.32.114 -p 8883 -t "secure/esp32/cmd" -m "hello" --cafile certs\ca.crt --cert certs\client.crt --key certs\client.key
```

## Troubleshooting

| อาการ | สาเหตุ / วิธีแก้ |
|---|---|
| `Connection Refused: not authorised` | คอนฟิกไม่มี `allow_anonymous true` / `use_identity_as_username true` → รัน `certs` ใหม่ |
| `certificate verify failed` / hostname mismatch | IP ใน `-BrokerIp`, CN/SAN ของ server.crt และ `MQTTS_BROKER_URI` ไม่ตรงกัน |
| ESP32 `TLS / Transport Error Code: 0x...` | ใบรับรองไม่ได้ฝัง/ผิดชุด (flash ก่อน `certs` ใหม่) → รัน `certs`, build, flash ใหม่ |
| ESP32 เห็น `certificate is not yet valid` | นาฬิกาบอร์ดยังไม่ซิงก์ (1970) – ใบรับรองสร้างวันนี้ ลบ `-days`/รอ SNTP หรือสร้างใหม่ย้อนวัน (ดูหมายเหตุ) |
| `mosquitto.exe` ฟ้องพอร์ต 8883 ถูกใช้งาน | มี Broker เก่าค้าง → `Get-Process mosquitto \| Stop-Process` |
| Build ฟ้องไม่พบ `certs/ca.crt` | ยังไม่ได้รัน `.\lab11-3.ps1 certs` |

> **หมายเหตุเรื่องเวลา:** ESP-IDF ตรวจช่วงเวลาของใบรับรองด้วยนาฬิกาบอร์ด ถ้าบอร์ดไม่ได้ซิงก์เวลา (ค่าเริ่มต้น 1970)
> การตรวจ `notBefore` อาจล้มเหลว ใน mbedTLS ของ ESP-IDF ปกติถือว่าเวลาของระบบถูก build time เป็นค่าเริ่มต้นจึงมักผ่าน
> หากเจอ error นี้ให้แจ้ง จะเพิ่มการซิงก์เวลา SNTP ให้
