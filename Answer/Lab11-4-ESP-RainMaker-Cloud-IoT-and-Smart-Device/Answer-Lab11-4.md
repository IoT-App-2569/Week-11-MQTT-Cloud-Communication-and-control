# Answer Lab 11.4 – ESP RainMaker Smart Light

- ชื่อ-สกุล: 
- รหัสนักศึกษา: 67030098
- วันที่ทำการทดลอง: 
- โค้ด: [Code/Lab11-4](../../Code/Lab11-4-ESP-RainMaker-Cloud-IoT-and-Smart-Device)
- รูปภาพ: [Answer/Image/Lab11-4](../Image/Lab11-4)

---

## 1. ผลการทดลอง

### 11-4.1–2 โปรเจกต์และโค้ด
![dependency esp-rainmaker](../Image/Lab11-4/11-4-1_idf_component.png)

*รูปที่ dependency esp-rainmaker*: 

### 11-4.3 Build / Flash และ QR Code
![QR Code ใน serial log](../Image/Lab11-4/11-4-3_qr_code.png)

*รูปที่ QR Code ใน serial log*: 

### 11-4.4 Provisioning และ Claiming
![ขั้นตอน provisioning ในแอป](../Image/Lab11-4/11-4-4_provisioning.png)

*รูปที่ ขั้นตอน provisioning ในแอป*: 

### 4.1 ควบคุมผ่าน Mobile App
| การกระทำในแอป | สถานะ LED | หมายเหตุ |
|---|---|---|
| Power ON | | |
| Power OFF | | |
| Brightness | | |

![ควบคุมผ่านแอป](../Image/Lab11-4/11-4-5_app_control.png)

*รูปที่ ควบคุมผ่านแอป*: 

### 4.2 Remote 4G/5G Test
![ทดสอบผ่านเครือข่ายมือถือ](../Image/Lab11-4/11-4-6_remote_test.png)

*รูปที่ ทดสอบผ่านเครือข่ายมือถือ*: 

### 4.3 RainMaker CLI (ทางเลือก)
![ผลคำสั่ง rainmaker.py](../Image/Lab11-4/11-4-7_cli.png)

*รูปที่ ผลคำสั่ง rainmaker.py*: 

## 2. คำถามท้ายการทดลอง

**1. Assisted Claiming ช่วยแก้ปัญหาการติดตั้งใบรับรองบนอุปกรณ์อย่างไร?**

> คำตอบ: 

**2. เหตุใด `write_cb` ต้องเรียก `esp_rmaker_param_update_and_report()` ทุกครั้ง?**

> คำตอบ: 

**3. เปรียบเทียบ Plain MQTT (Lab 11.2) กับ ESP RainMaker (Lab 11.4) ด้าน Development Effort, Security, Scalability**

| ด้าน | Plain MQTT + Mosquitto | ESP RainMaker |
|---|---|---|
| Development Effort | | |
| Security | | |
| Scalability | | |

**4. ขั้นตอนเชื่อม RainMaker กับ Google Home / Alexa โดยไม่ต้องแก้โค้ดบนบอร์ด**

> คำตอบ: 

## 3. สรุปผลการทดลอง

