
# รายงานผลการทดลองใบงานที่ 11.1

## การติดตั้ง Mosquitto Broker และการทดสอบโปรโตคอล MQTT ด้วย CLI

---

## 4. บันทึกและวิเคราะห์ผลการทดลอง

### 4.1 บันทึกผลการทดลองตามกิจกรรม

#### กิจกรรมที่ 11-1.2: การทดสอบการ Publish / Subscribe และการใช้ Wildcards (`+` และ `#`)

**1. การ Subscribe หัวข้อเฉพาะเจาะจง (`home/livingroom/light`)**

- **ผลการทดลอง:** เมื่อ Publisher ส่งข้อความ `ON` ไปยังหัวข้อ `home/livingroom/light` ฝั่ง Subscriber จะได้รับข้อความ `home/livingroom/light ON` ทันที

**2. การทดสอบ Single-Level Wildcard (`home/+/light`)**

ผลการทดลอง:

- `home/livingroom/light` → **แสดงผล** (`home/livingroom/light ON`)
- `home/kitchen/light` → **แสดงผล** (`home/kitchen/light OFF`)
- `home/bedroom/balcony/light` → **ไม่แสดงผล**

**วิเคราะห์ผล:**

สัญลักษณ์ `+` ใช้แทนชื่อ Topic ได้เพียง 1 ระดับ (Single Level) เท่านั้น แต่หัวข้อ `home/bedroom/balcony/light` มีโครงสร้างถึง 4 ระดับ จึงไม่ตรงกับเงื่อนไขของตัวกรอง

**3. การทดสอบ Multi-Level Wildcard (`home/#`)**

ผลการทดลอง:

- `home/livingroom/temp` → **แสดงผล** (`home/livingroom/temp 26.4`)
- `home/bedroom/balcony/light` → **แสดงผล** (`home/bedroom/balcony/light ON`)

**วิเคราะห์ผล:**

สัญลักษณ์ `#` สามารถครอบคลุม Topic ย่อยทุกระดับชั้นที่อยู่ต่อจาก `home/` ทั้งหมด ไม่ว่าจะมีความซับซ้อนกี่ระดับก็ตาม

---

### 4.2 ภาพหน้าจอและการบันทึก Log ฝั่ง Broker (กิจกรรมที่ 11-1.3 ถึง 11-1.5)

#### 1. บันทึก Log การทำงานของ Mosquitto Broker ในโหมด Verbose (`-v`)

เมื่อ Subscriber เชื่อมต่อและบอกรับข้อมูล จะปรากฏ Log ในลักษณะดังนี้

```text
1710000001: mosquitto version 2.0.18 starting
1710000001: Config loaded from C:\mqtt_lab\mosquitto.conf.
1710000001: Opening ipv4 listen socket on port 1883.
1710000005: New connection from 127.0.0.1:54321 on port 1883.
1710000005: New client connected from 127.0.0.1:54321 as auto-ABC12345 (p2, c1, k60).
1710000005: Received SUBSCRIBE from auto-ABC12345
1710000005:     test/qos (QoS 1)
1710000005: auto-ABC12345 1 test/qos
1710000005: Sending SUBACK to auto-ABC12345
```

#### 2. ตารางเปรียบเทียบลำดับแพ็กเก็ตระหว่าง QoS 0, QoS 1 และ QoS 2 (จาก Broker Log)

| ระดับ QoS | ชื่อกลไก (Delivery Guarantee) | ลำดับแพ็กเก็ตที่ปรากฏใน Broker Log | การยืนยันความถูกต้อง (ACKs) |
|:---|:---|:---|:---|
| **QoS 0** | At most once (ส่งครั้งเดียวไม่การันตี) | 1. `Received PUBLISH ... (q0, r0, m0)` | **ไม่มี ACK** (Fire and Forget) |
| **QoS 1** | At least once (อย่างน้อยหนึ่งครั้ง) | 1. `Received PUBLISH ... (d0, q1, m1)`<br>2. `Sending PUBACK to ... (m1)` | **2-way Handshake** (`PUBLISH` → `PUBACK`) |
| **QoS 2** | Exactly once (ถูกต้องเพียงครั้งเดียว) | 1. `Received PUBLISH ... (d0, q2, m1)`<br>2. `Sending PUBREC to ... (m1)`<br>3. `Received PUBREL from ... (m1)`<br>4. `Sending PUBCOMP to ... (m1)` | **4-way Handshake** (`PUBLISH` → `PUBREC` → `PUBREL` → `PUBCOMP`) |

#### 3. ผลการทดสอบ Retained Message และ LWT

**ผลการทดสอบ Retained Message (`-r`)**

เมื่อเปิด Subscriber ขึ้นมาใหม่หลังจากที่มีการ Publish ด้วยแฟล็ก `-r` ระบบจะส่งข้อความล่าสุดให้ Subscriber ทันทีที่เชื่อมต่อสำเร็จ

ผลที่ได้รับ:

```text
Client auto-XYZ12345 receiving payload:
sensor/state CRITICAL_ALERT
```

**ผลการทดสอบ Last Will and Testament (LWT)**

เมื่อทำการกดปิดหน้าต่าง Terminal ที่ 3 (Publisher) กะทันหัน ปรากฏ Log ฝั่ง Broker และข้อความฝั่ง Subscriber ดังนี้

```text
1710000100: Socket error on client Node_Client_01, disconnecting.
1710000100: Sending LWT to node/esp32/status (QoS 1, Retain 0)
```

เทอร์มินัลฝั่ง Subscriber ได้รับข้อความอัตโนมัติ:

```text
node/esp32/status LOST_CONNECTION_OFFLINE
```

---

## 5. เฉลยคำถามท้ายการทดลอง (Post-Lab Questions)

### ข้อ 1: เพราะเหตุใดสัญลักษณ์ `#` จึงต้องวางไว้ที่ตำแหน่งท้ายสุดของ Topic เสมอ? หากกำหนด Topic Filter เป็น `home/#/light` จะเกิดข้อผิดพลาดอย่างไร?

**คำตอบ:**

ตามข้อกำหนดมาตรฐานโปรโตคอล MQTT (MQTT Specification) เครื่องหมาย `#` คือ Multi-Level Wildcard ที่ใช้แทนลำดับชั้นของ Topic ทั้งหมดที่อยู่ถัดไป ดังนั้นกฎของโปรโตคอลจึงระบุว่า `#` ต้องอยู่หลังสัญลักษณ์ `/` และต้องเป็นอักขระตัวสุดท้ายของ Topic Filter เท่านั้น

หากกำหนด Topic Filter เป็น `home/#/light` จะเกิดผลดังนี้

- **ข้อผิดพลาดที่เกิดขึ้น:** Broker จะปฏิเสธคำสั่ง Subscribe และแจ้งข้อผิดพลาดประเภท Invalid Topic Filter (หรือตัดการเชื่อมต่อทันที)
- **เหตุผล:** ระบบไม่สามารถตีความได้ว่า `#` จะครอบคลุมกี่ระดับเพื่อข้ามไปหาคำว่า `/light` ส่งผลให้โครงสร้างการจับคู่ตัวกรอง (Topic Matching Engine) ไม่ถูกต้อง

### ข้อ 2: ในระบบงานติดตามยานพาหนะ (GPS Tracking) ที่ส่งพิกัดรถยนต์ทุก ๆ 1 วินาที ท่านคิดว่าควรเลือกระดับ QoS ใดระหว่าง QoS 0, 1 หรือ 2 เพราะเหตุใด?

**คำตอบ:**

ควรเลือก QoS 0 (At Most Once) หรืออย่างมากที่สุด QoS 1

**เหตุผล:**

1. **ความถี่ของข้อมูลสูง (High Frequency Data):** พิกัด GPS ถูกส่งทุก ๆ 1 วินาที หากมีแพ็กเก็ตสูญหายไปบ้างในระหว่างเคลื่อนที่ ข้อมูลพิกัดในวินาทีถัดไปก็จะถูกส่งมาทดแทนทันที การสูญหายของพิกัดเพียงจุดเดียวไม่มีผลต่อการติดตามเส้นทางโดยรวม

2. **ประสิทธิภาพและย่านความถี่ (Overhead & Bandwidth):** QoS 0 ไม่ต้องรอแพ็กเก็ตตอบรับ (ACK) ช่วยลดภาระการประมวลผล ประหยัดแบตเตอรี่ของอุปกรณ์บนรถ และลดปริมาณข้อมูลในเครือข่ายมือถือ (Cellular Data)

3. **ป้องกันปัญหาข้อมูลซ้ำ/ล่าช้า:** การใช้ QoS 1 หรือ QoS 2 หากสัญญาณมือถืออับสัญญาณ อาจทำให้เกิดการส่งซ้ำ (Retransmission) หรือคิวแพ็กเก็ตสะสม ซึ่งทำให้พิกัดเก่าถูกส่งมาสับสนกับพิกัดปัจจุบัน

### ข้อ 3: หาก Client ส่งแพ็กเก็ต DISCONNECT ไปยัง Broker อย่างถูกต้องก่อนปิดโปรแกรม ข้อความ Last Will and Testament (LWT) จะถูกเผยแพร่ออกไปหรือไม่ เพราะเหตุใด?

**คำตอบ:**

ข้อความ LWT จะไม่ถูกเผยแพร่ออกไป

**เหตุผล:**

- พินัยกรรมฉุกเฉิน (LWT) ถูกออกแบบมาให้ทำงานเฉพาะในกรณีที่ Client ขาดการเชื่อมต่ออย่างไม่ปกติ (Abnormal Disconnection) เช่น เครือข่ายหลุด อุปกรณ์ดับกะทันหัน หรือหมดเวลา Keep Alive จนเกิด Heartbeat Timeout เท่านั้น
- หาก Client สั่งปิดการทำงานอย่างถูกต้อง จะส่งแพ็กเก็ต `DISCONNECT` ไปยัง Broker เพื่อแจ้งว่าเป็นการปิดระบบตามปกติ Broker จะยกเลิก (Discard) ข้อความ LWT ที่เคยลงทะเบียนไว้ใน Session นั้นทันที

### ข้อ 4: Retained Message มีประโยชน์อย่างไรในการออกแบบระบบแดชบอร์ด Smart Home สำหรับผู้ใช้งานที่เพิ่งเปิดแอปพลิเคชันขึ้นมาเป็นครั้งแรก?

**คำตอบ:**

ประโยชน์หลักคือ ทำให้แดชบอร์ดแสดงสถานะล่าสุดของอุปกรณ์ได้ทันที (Instant State Synchronization) โดยไม่ต้องรอให้อุปกรณ์ส่งข้อมูลใหม่

**รายละเอียดการทำงาน:**

1. **แก้ปัญหาหน้าจอว่างเปล่า (No Waiting Time):** หากอุปกรณ์ เช่น สวิตช์ไฟหรือเซนเซอร์อุณหภูมิ ส่งข้อมูลเพียงทุก ๆ 10 นาที หรือส่งเฉพาะตอนที่มีการเปลี่ยนแปลงสถานะ เมื่อผู้ใช้เปิดแอป Smart Home ขึ้นมา Retained Message จะถูก Broker ส่งให้แอปทันที ทำให้ผู้ใช้เห็นสถานะไฟ (ON/OFF) หรืออุณหภูมิได้ตั้งแต่เริ่มต้นใช้งาน

2. **ลดภาระอุปกรณ์ IoT (Power Efficiency):** อุปกรณ์ในบ้านไม่จำเป็นต้องคอยส่งข้อความซ้ำ ๆ เพื่อบอกสถานะให้แอปใหม่ ๆ ที่เพิ่งเปิดขึ้นมา อุปกรณ์ส่งข้อมูลเมื่อมีการเปลี่ยนสถานะพร้อมติดแฟล็ก `-r` แล้วปล่อยให้ Broker เป็นตัวจัดการกระจายสถานะล่าสุดให้ Client ทุกตัวที่เข้ามาใหม่เอง

---

## 6. สรุปผลการทดลอง

จากการทดลองติดตั้ง Mosquitto Broker และทดสอบโปรโตคอล MQTT ด้วย CLI ได้เรียนรู้การทำงานของระบบ Publish / Subscribe รวมถึงการใช้ Wildcards (`+` และ `#`) เพื่อรับข้อมูลจาก Topic ที่แตกต่างกัน

นอกจากนี้ยังได้ทดสอบระดับ QoS ทั้ง 3 ระดับ ได้แก่ QoS 0, QoS 1 และ QoS 2 รวมถึงการทำงานของ Retained Message และ Last Will and Testament (LWT) ซึ่งช่วยให้เข้าใจการรับส่งข้อมูล การยืนยันการส่งข้อความ และการจัดการสถานะของอุปกรณ์ IoT ผ่าน MQTT ได้ดียิ่งขึ้น
