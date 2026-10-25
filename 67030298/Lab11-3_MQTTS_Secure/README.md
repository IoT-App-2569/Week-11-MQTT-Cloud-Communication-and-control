# Lab 11.3: MQTTS TLS Mutual Authentication (mTLS) Security

โปรเจกต์สาธิตการสื่อสาร MQTT ความปลอดภัยสูงระดับองค์กรด้วยการเข้ารหัส MQTTS (พอร์ต 8883) พร้อมบังคับใช้การตรวจสอบสิทธิ์สองทาง (Mutual Authentication / mTLS) ระหว่าง Mosquitto Broker และบอร์ด ESP32

## โครงสร้างโปรเจกต์
```
Lab11-3_MQTTS_Secure/
├── CMakeLists.txt
├── mosquitto_ssl.conf
├── README.md
├── scripts/
│   ├── generate_certs.ps1
│   └── generate_certs.sh
└── main/
    ├── CMakeLists.txt
    ├── main.c
    └── certs/
        ├── ca.crt
        ├── client.crt
        └── client.key
```

## ขั้นตอนการทดลอง

### 1. สร้างกุญแจและใบรับรองดิจิทัล (OpenSSL)
เปิด PowerShell ในโฟลเดอร์ `scripts/` แล้วรันสคริปต์ (ระบุ IP เครื่องคอมพิวเตอร์ที่รัน Broker):
```powershell
cd scripts
.\generate_certs.ps1 -BrokerIP "192.168.1.100"
```
สคริปต์จะสร้าง `ca.crt`, `server.crt`, `server.key`, `client.crt`, `client.key` และคัดลอกไฟล์ที่จำเป็นลงใน `main/certs/` ให้อัตโนมัติ

### 2. รัน Mosquitto MQTTS Broker (Port 8883)
คัดลอก `ca.crt`, `server.crt`, `server.key` จาก `scripts/certs_out/` มาไว้ที่โฟลเดอร์เดียวกับ `mosquitto_ssl.conf` หรือระบุ Full Path ใน `mosquitto_ssl.conf` จากนั้นรัน:
```powershell
& "C:\Program Files\mosquitto\mosquitto.exe" -c "mosquitto_ssl.conf" -v
```

### 3. ทดสอบ CLI บนคอมพิวเตอร์
```powershell
# Subscriber (ถูกต้อง):
mosquitto_sub -h 192.168.1.100 -p 8883 -t "secure/test" --cafile ca.crt --cert client.crt --key client.key -v

# Publisher (ถูกต้อง):
mosquitto_pub -h 192.168.1.100 -p 8883 -t "secure/test" -m "Hello Secure World!" --cafile ca.crt --cert client.crt --key client.key

# Publisher (Unauthorized Attack - ไม่มี Client Cert):
mosquitto_pub -h 192.168.1.100 -p 8883 -t "secure/test" -m "Hacker message" --cafile ca.crt
```

### 4. คอมไพล์และแฟลช ESP32 เฟิร์มแวร์
แก้ไข `WIFI_SSID`, `WIFI_PASS` และ `MQTTS_BROKER_URI` ใน `main/main.c` ให้ตรงกับ IP ของเครื่อง แล้วสั่ง:
```powershell
idf.py set-target esp32c3   # หรือ esp32
idf.py build
idf.py -p COMx flash monitor
```
