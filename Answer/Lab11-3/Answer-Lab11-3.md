# Answer Lab 11.3 – MQTTS: TLS และ Mutual Authentication

- ชื่อ-สกุล: 
- รหัสนักศึกษา: 67030098
- วันที่ทำการทดลอง: 
- โค้ด: [Code/Lab11-3](../../Code/Lab11-3)
- รูปภาพ: [Answer/Image/Lab11-3](../Image/Lab11-3)

---

## 1. ผลการทดลอง

### 11-3.1 ใบรับรองที่สร้าง
| ไฟล์ | CN | อายุ (วัน) | ผู้ลงนาม |
|---|---|---|---|
| ca.crt | | | |
| server.crt | | | |
| client.crt | | | |

![ไฟล์ cert และ key ที่สร้าง](../Image/Lab11-3/11-3-1_openssl_files.png)

*รูปที่ ไฟล์ cert และ key ที่สร้าง*: 

### 11-3.2 Mosquitto MQTTS (8883)
![Broker เปิดพอร์ต 8883](../Image/Lab11-3/11-3-2_broker_8883.png)

*รูปที่ Broker เปิดพอร์ต 8883*: 

### 11-3.3 ทดสอบ CLI
| กรณีทดสอบ | ผลลัพธ์ | ข้อความ error/log |
|---|---|---|
| มี CA + client cert + key | | |
| ไม่มี client cert | | |
| ใช้พอร์ต 1883 | | |

![ผลทดสอบ CLI ผ่าน TLS](../Image/Lab11-3/11-3-3_cli_tls.png)

*รูปที่ ผลทดสอบ CLI ผ่าน TLS*: 

### 11-3.4–5 ESP32
![Serial monitor: TLS handshake](../Image/Lab11-3/11-3-4_esp_serial.png)

*รูปที่ Serial monitor: TLS handshake*: 

![Broker log: Peer certificate verified](../Image/Lab11-3/11-3-5_broker_peer_cert.png)

*รูปที่ Broker log: Peer certificate verified*: 

## 2. คำถามท้ายการทดลอง

**1. เหตุใด CN ของ Server ต้องตรงกับ IP/Hostname ของ Broker? หากไม่ตรงจะเกิดอะไรขึ้น?**

> คำตอบ: 

**2. mTLS ต่างจาก HTTPS ทั่วไปอย่างไร?**

> คำตอบ: 

**3. หากไฟล์ `.bin` ถูกขโมย จะกู้ `client.key` ได้หรือไม่ และควรป้องกันอย่างไร?**

> คำตอบ: 

## 3. สรุปผลการทดลอง

