# Lab 11.1: Mosquitto Broker and MQTT Fundamentals

โฟลเดอร์นี้ประกอบด้วยไฟล์คอนฟิกูเรชันและสคริปต์สำหรับการทดลองรัน Broker และทดสอบคำสั่ง CLI (Pub/Sub, QoS, Retain, LWT)

## รายการไฟล์
- `mosquitto.conf`: ไฟล์ตั้งค่า Mosquitto Broker (Port 1883, allow_anonymous true, log_type all)
- `run_broker.ps1`: สคริปต์ PowerShell ช่วยปิด Windows Service ที่ซ้ำซ้อนและเริ่มรัน Broker แบบ Verbose

## สรุปคำสั่ง CLI สำหรับการทดลองใน Lab 11.1

### 1. เริ่มต้น Broker (เทอร์มินัล 1)
```powershell
.\run_broker.ps1
# หรือ
& mosquitto.exe -c "mosquitto.conf" -v
```

### 2. ทดสอบ Basic Pub/Sub และ Wildcards (เทอร์มินัล 2 & 3)
```powershell
# Subscriber:
& mosquitto_sub.exe -h 127.0.0.1 -p 1883 -t "home/livingroom/light" -v
& mosquitto_sub.exe -h 127.0.0.1 -p 1883 -t "home/+/light" -v
& mosquitto_sub.exe -h 127.0.0.1 -p 1883 -t "home/#" -v

# Publisher:
& mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "home/livingroom/light" -m "ON"
& mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "home/kitchen/light" -m "OFF"
& mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "home/bedroom/balcony/light" -m "ON"
```

### 3. ทดสอบ QoS 0, QoS 1, QoS 2
```powershell
# QoS 0:
& mosquitto_sub.exe -h 127.0.0.1 -t "test/qos" -q 0 -v
& mosquitto_pub.exe -h 127.0.0.1 -t "test/qos" -m "QoS 0 Message" -q 0

# QoS 1:
& mosquitto_sub.exe -h 127.0.0.1 -t "test/qos" -q 1 -v
& mosquitto_pub.exe -h 127.0.0.1 -t "test/qos" -m "QoS 1 Message" -q 1

# QoS 2:
& mosquitto_sub.exe -h 127.0.0.1 -t "test/qos" -q 2 -v
& mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "test/qos" -m "QoS 2 Message" -q 2
```

### 4. ทดสอบ Retained Messages
```powershell
# ส่ง Retained Message:
& mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "sensor/state" -m "CRITICAL_ALERT" -r

# ตรวจสอบว่า Subscriber ใหม่ได้รับทันที:
& mosquitto_sub.exe -h 127.0.0.1 -p 1883 -t "sensor/state" -v

# ล้าง Retained Message:
& mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "sensor/state" -n -r
```

### 5. ทดสอบ Last Will and Testament (LWT)
```powershell
# Terminal 2 (Monitor):
& mosquitto_sub.exe -h 127.0.0.1 -p 1883 -t "node/esp32/status" -v

# Terminal 3 (ESP32 Simulator with LWT):
& mosquitto_sub.exe -h 127.0.0.1 -p 1883 -i "Node_Client_01" -t "dummy/listen" --will-topic "node/esp32/status" --will-payload "LOST_CONNECTION_OFFLINE" --will-qos 1

# Terminal 4: ส่งสถานะ ONLINE
& mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "node/esp32/status" -m "ONLINE"

# สังเกตผลเมื่อ Force Close หน้าต่าง Terminal 3
```
