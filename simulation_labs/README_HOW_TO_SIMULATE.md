# 🚀 คู่มือการรันการทดลองจำลองบน VS Code (Simulation Labs Guide)

โครงสร้างโฟลเดอร์นี้ถูกสร้างขึ้นเพื่อให้นักศึกษาสามารถทำการทดลอง จำลองการทำงาน และเก็บภาพผลลัพธ์พร้อมข้อมูลการทดลองได้ทั้งหมดผ่านหน้าต่าง Terminal ใน Visual Studio Code โดยตรง

```text
simulation_labs/
├── Lab11-1_MQTT_Fundamentals/         (แล็บ 11.1: Broker, Pub/Sub, QoS, Retain, LWT)
│   ├── mosquitto.conf                 - ไฟล์คอนฟิกของ Broker
│   ├── 1_start_broker.ps1             - สคริปต์เปิดรัน Mosquitto Broker (Verbose)
│   ├── 2_test_wildcards.ps1           - สคริปต์ทดสอบ Wildcard (+) และ (#)
│   ├── 3_test_qos.ps1                 - สคริปต์ทดสอบระดับ QoS 0, 1, 2
│   ├── 4_test_retained.ps1            - สคริปต์ทดสอบ Retained Message (-r)
│   └── 5_test_lwt.ps1                 - สคริปต์ทดสอบ Last Will and Testament (LWT)
│
├── Lab11-2_ESP32_MQTT_Client/         (แล็บ 11.2: จำลอง ESP32 ส่ง Telemetry & รับคำสั่ง)
│   ├── simulate_esp32_device.py       - ตัวจำลอง ESP32 โค้ดส่ง JSON & รับคำสั่ง LED
│   ├── 1_listen_telemetry.ps1         - สคริปต์ดักรับ Telemetry JSON ทุกๆ 4 วินาที
│   ├── 2_send_led_on.ps1              - สคริปต์สั่งเปิดไฟ LED ("ON")
│   ├── 3_send_led_off.ps1             - สคริปต์สั่งปิดไฟ LED ("OFF")
│   └── 4_send_led_json.ps1            - สคริปต์สั่งเปิดไฟผ่าน JSON ({"state": 1})
│
├── Lab11-3_MQTTS_TLS_Security/        (แล็บ 11.3: ความปลอดภัย MQTTS พอร์ต 8883 & mTLS)
│   ├── certs/                         - โฟลเดอร์เก็บใบรับรอง X.509 (ca, server, client)
│   ├── mosquitto_ssl.conf             - ไฟล์คอนฟิก Broker โหมด SSL บังคับตรวจ Client Cert
│   ├── 1_generate_certs.ps1           - สคริปต์ OpenSSL สร้าง Root CA, Server & Client Certs
│   ├── 2_start_ssl_broker.ps1         - สคริปต์เปิดรัน Broker พอร์ต 8883
│   ├── 3_test_legit_client.ps1        - จำลอง Client ที่ถูกต้อง (จับมือ TLS สำเร็จ)
│   └── 4_test_hacker_client.ps1       - จำลอง Hacker ที่ไม่มีใบรับรอง (ถูกดีดทิ้ง)
│
└── Lab11-4_RainMaker_Cloud/           (แล็บ 11.4: จำลอง ESP RainMaker Smart Light)
    └── simulate_rainmaker_node.py     - ตัวจำลองระบบ RainMaker, QR Code, Callback, 4G/5G
```

---

## 💻 วิธีการรันใน VS Code:
1. เปิด VS Code ขึ้นมาในโฟลเดอร์นี้
2. เปิด Terminal ใน VS Code (`Ctrl + Shift + \``)
3. เข้าไปยังแต่ละโฟลเดอร์ เช่น:
   ```powershell
   cd simulation_labs\Lab11-1_MQTT_Fundamentals
   .\1_start_broker.ps1
   ```
4. สามารถเปิด Split Terminal (แบ่งครึ่งจอ) เพื่อรัน Broker ฝั่งซ้าย และรัน Script ทดสอบฝั่งขวาได้ตามต้องการ
