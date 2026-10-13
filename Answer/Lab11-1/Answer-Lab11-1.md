# Answer Lab 11.1 – Mosquitto Broker และ MQTT Fundamentals

- ชื่อ-สกุล: 
- รหัสนักศึกษา: 67030098
- วันที่ทำการทดลอง: 
- โค้ด: [Code/Lab11-1](../../Code/Lab11-1)
- รูปภาพ: [Answer/Image/Lab11-1](../Image/Lab11-1)

---

## 1. ผลการทดลอง

### 11-1.1 Broker Verbose Log
![Broker verbose log ขณะเชื่อมต่อและ Subscribe](../Image/Lab11-1/11-1-1_broker_log.png)

*รูปที่ Broker verbose log ขณะเชื่อมต่อและ Subscribe*: 

### 11-1.2 Pub/Sub และ Wildcards
| Topic ที่ Publish | Filter `home/+/light` | Filter `home/#` |
|---|---|---|
| home/livingroom/light | | |
| home/kitchen/light | | |
| home/bedroom/balcony/light | | |

![ผลการทดสอบ Wildcard](../Image/Lab11-1/11-1-2_wildcard.png)

*รูปที่ ผลการทดสอบ Wildcard*: 

### 11-1.3 เปรียบเทียบ QoS
| QoS | ลำดับแพ็กเก็ตที่เห็นใน Broker Log | จำนวนแพ็กเก็ต | หมายเหตุ |
|---|---|---|---|
| 0 | | | |
| 1 | | | |
| 2 | | | |

![Broker log QoS 0/1/2](../Image/Lab11-1/11-1-3_qos.png)

*รูปที่ Broker log QoS 0/1/2*: 

### 11-1.4 Retained Message
![Subscriber เปิดใหม่ได้รับ retained message](../Image/Lab11-1/11-1-4_retained.png)

*รูปที่ Subscriber เปิดใหม่ได้รับ retained message*: 

### 11-1.5 LWT
![ข้อความ LWT หลัง client หลุด](../Image/Lab11-1/11-1-5_lwt.png)

*รูปที่ ข้อความ LWT หลัง client หลุด*: 

## 2. คำถามท้ายการทดลอง

**1. เหตุใด `#` ต้องอยู่ท้ายสุด และ `home/#/light` ผิดอย่างไร?**

> คำตอบ: 

**2. ระบบ GPS Tracking ส่งทุก 1 วินาที ควรใช้ QoS ใด เพราะเหตุใด?**

> คำตอบ: 

**3. หาก Client ส่ง `DISCONNECT` ถูกต้อง LWT จะถูกเผยแพร่หรือไม่ เพราะเหตุใด?**

> คำตอบ: 

**4. Retained Message มีประโยชน์อย่างไรกับแดชบอร์ด Smart Home?**

> คำตอบ: 

## 3. สรุปผลการทดลอง

