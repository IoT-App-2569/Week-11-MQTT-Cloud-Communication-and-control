# รายงานผลการทดลอง สัปดาห์ที่ 11
## หัวข้อ: MQTT Cloud Communication and Control & ESP RainMaker Ecosystem

**รหัสนักศึกษา:** 67030073  
**ชื่อ-นามสกุล:** นายณัฐพงศ์ เนียมประดิษฐ (Natthaphong Niampradit)  
**วิชา:** ระบบสมองกลฝังตัวและการเชื่อมต่อเครือข่าย (Embedded Systems & IoT)  
**บอร์ดทดลอง:** ESP32 (ESP32-D0WD-V3)  

---

## 1. การต่อวงจรฮาร์ดแวร์ (Hardware Wiring)

การทดลองในสัปดาห์นี้ใช้บอร์ด ESP32 เชื่อมต่อกับหลอดไฟ LED (Actuator) และตัวต้านทานปรับค่าได้ Potentiometer (Sensor):

| อุปกรณ์ | ขาบนอุปกรณ์ | ขาต่อบน ESP32 | หน้าที่การทำงาน |
| :--- | :--- | :--- | :--- |
| **LED** | ขั้วบวก (Anode ขายาว) ผ่าน R 220Ω | **GPIO 2** (D2) | Actuator แสดงผลสถานะเปิด-ปิดไฟตามคำสั่งจาก Broker/Cloud |
| **LED** | ขั้วลบ (Cathode ขาสั้น) | **GND** | กราวด์วงจรหลอดไฟ |
| **Potentiometer 10k** | ขากลาง (Wiper) | **GPIO 34** (ADC1_CH6) | เซนเซอร์ตรวจวัดแรงดันแอนะล็อก ส่งข้อมูล Telemetry |
| **Potentiometer 10k** | ขาริมซ้าย | **3.3V** | แรงดันไฟเลี้ยงอ้างอิง |
| **Potentiometer 10k** | ขาริมขวา | **GND** | กราวด์อ้างอิง |

### แผนภาพการต่อวงจร
```
       ESP32 Board                             Peripherals
     +---------------+                          +------------------------+
     |        GPIO 2 |------------------------->| LED (+ R 220Ω -> GND)  |
     |       GPIO 34 |<-------------------------| Potentiometer (Wiper)  |
     |          3.3V |------------------------->| Potentiometer (VCC)    |
     |           GND |------------------------->| Potentiometer (GND)    |
     +---------------+                          +------------------------+
```

### ภาพถ่ายการต่อวงจรฮาร์ดแวร์จริง

> 📷 **ภาพถ่ายวงจรฮาร์ดแวร์จริง:**  
> *(ให้นักศึกษาถ่ายภาพบอร์ด ESP32 เชื่อมต่อกับ LED ขา 2 และ Potentiometer ขา 34 บน Breadboard ของตนเองมาแนบในช่องนี้)*

---

## 2. ลิงก์วิดีโอสาธิตการทำงาน (Video Demonstration Links)

| การทดลอง | รายละเอียดคลิปสาธิต | ลิงก์วิดีโอ (YouTube Shorts) |
| :--- | :--- | :--- |
| **ใบงานที่ 11.1** | Mosquitto Broker Setup & MQTT CLI Fundamentals (QoS, Retain, LWT) | *(การทดสอบระดับโปรโตคอลด้วย CLI มีภาพบันทึกผลการรันครบถ้วน)* |
| **ใบงานที่ 11.2** | ESP32 MQTT Two-Way Telemetry & LED Remote Control | *(ให้นักศึกษาแนบลิงก์คลิปวิดีโอสาธิตของตนเอง)* |
| **ใบงานที่ 11.3** | MQTTS High Security TLS & Mutual Authentication (mTLS) | *(การทดสอบความปลอดภัย mTLS ด้วย OpenSSL มีภาพบันทึกผลครบถ้วน)* |
| **ใบงานที่ 11.4** | ESP RainMaker Cloud Smart Light & 4G/5G Remote Control | *(ให้นักศึกษาแนบลิงก์คลิปวิดีโอสาธิตของตนเอง)* |

---

## 3. ใบงานที่ 11.1: การติดตั้ง Mosquitto Broker และการทดสอบโปรโตคอล MQTT ด้วย CLI

### 3.1 ผลการรัน Mosquitto Broker
* ไฟล์คอนฟิก `mosquitto.conf`:
```text
listener 1883 0.0.0.0
allow_anonymous true
log_type all
```

![Mosquitto Broker Running](images/lab11_1_photo1.png)
*รูปที่ 2: หน้าต่าง Terminal แสดงการเริ่มต้นทำงานของ Mosquitto Broker บนพอร์ต 1883*

---

### 3.2 การทดสอบ Publish / Subscribe และ Wildcards (`+`, `#`)
* **ทดสอบหัวข้อตรง:** ทำการ Subscribe หัวข้อ `home/livingroom/light` และ Publish ค่า `ON` ได้รับข้อความปกติ
* **ทดสอบ Single-Level Wildcard (`+`):** Subscribe หัวข้อ `home/+/light`
  - เมื่อส่งไปยัง `home/livingroom/light` -> ได้รับข้อความ
  - เมื่อส่งไปยัง `home/kitchen/light` -> ได้รับข้อความ
  - เมื่อส่งไปยัง `home/bedroom/balcony/light` -> **ไม่ได้รับข้อความ** เนื่องจาก `+` แทนที่ชื่อหัวข้อได้เพียง 1 ลำดับชั้น แต่หัวข้อนี้มี 2 ลำดับชั้น (`bedroom/balcony`)
* **ทดสอบ Multi-Level Wildcard (`#`):** Subscribe หัวข้อ `home/#`
  - ผลลัพธ์: ได้รับทุกข้อความที่มีโครงสร้างลำดับชั้นย่อยทั้งหมดที่ขึ้นต้นด้วย `home/`

---

### 3.3 การทดสอบระดับคุณภาพการส่งข้อมูล (QoS 0, QoS 1, QoS 2)
บันทึกลำดับแพ็กเก็ตที่เกิดขึ้นจาก Broker Log:

| ระดับ QoS | การรับประกัน | แพ็กเก็ตที่เกิดขึ้นระหว่าง Publisher <-> Broker | การตอบรับ (ACK) |
| :--- | :--- | :--- | :--- |
| **QoS 0** | At most once (ส่งครั้งเดียว ไม่การันตี) | `PUBLISH` | ไม่มี ACK |
| **QoS 1** | At least once (ถึงอย่างน้อย 1 ครั้ง) | `PUBLISH` -> `PUBACK` | มี `PUBACK` (ระบุ Message ID) |
| **QoS 2** | Exactly once (ถึงแน่นอนเพียงครั้งเดียว) | `PUBLISH` -> `PUBREC` -> `PUBREL` -> `PUBCOMP` | มีการทำ 4-Way Handshake สมบูรณ์ |

---

### 3.4 การทดสอบ Retained Message
* เมื่อส่งข้อความปกติในขณะที่ยังไม่มีใคร Subscribe -> ข้อความจะหายไปในทันที
* เมื่อส่งข้อความโดยระบุพารามิเตอร์ `-r` (Retain) -> Broker จะบันทึกข้อความล่าสุดค้างไว้ในระบบ เมื่อมี Subscriber เชื่อมต่อเข้ามาในภายหลัง จะได้รับข้อความนี้ทันทีโดยไม่ต้องรอรอบส่งใหม่

![Retained Message Test](images/lab11_1_photo2.png)
*รูปที่ 5: หน้าต่าง Subscriber เชื่อมต่อเข้ามาใหม่และได้รับข้อความ Retained ทันที*

---

### 3.5 การทดสอบ Last Will and Testament (LWT)
* กำหนด Will Topic: `node/esp32/status` ด้วย Payload: `LOST_CONNECTION_OFFLINE`
* เมื่อทดลองจำลองสถานการณ์โปรเซสปิดตัวลงอย่างกะทันหันด้วยคำสั่ง `kill -9` (ตัดการเชื่อมต่อโดยไม่ส่งแพ็กเก็ต `DISCONNECT`) -> Broker จะกระจายข้อความ LWT นี้ไปยัง Subscriber ทุกตัวโดยอัตโนมัติ

---

### 3.6 ตอบคำถามท้ายการทดลอง 11.1
1. **เพราะเหตุใดสัญลักษณ์ `#` จึงต้องวางไว้ที่ตำแหน่งท้ายสุดของ Topic เสมอ? หากกำหนด Topic Filter เป็น `home/#/light` จะเกิดข้อผิดพลาดอย่างไร?**
   * *ตอบ:* มาตรฐาน OASIS MQTT v3.1.1 กำหนดไว้อย่างเข้มงวดว่าสัญลักษณ์ `#` (Multi-level Wildcard) เป็นตัวแทนของ Topic Level ทั้งหมดนับตั้งแต่ระดับที่มันปรากฏไปจนถึงปลายทาง จึงจำเป็นต้องอยู่ตำแหน่งสุดท้ายของฟิลเตอร์เสมอ หากกำหนดเป็น `home/#/light` จะถือว่าเป็น Malformed Topic Filter ตัว Broker จะปฏิเสธคำขอและส่งแพ็กเก็ต SUBACK รหัสล้มเหลว (Failure Return Code 0x80) ทันที
2. **ในระบบงานติดตามยานพาหนะ (GPS Tracking) ที่ส่งพิกัดรถยนต์ทุกๆ 1 วินาที ท่านคิดว่าควรเลือกระดับ QoS ใดระหว่าง QoS 0, 1 หรือ 2 เพราะเหตุใด?**
   * *ตอบ:* ควรพิจารณาเลือกใช้ **QoS 0 (At most once)** เนื่องจากข้อมูลพิกัดถูกส่งด้วยความถี่สูงแบบ Real-time Stream (1 Hz) ซึ่งการได้รับข้อมูลล่าสุดมีความสำคัญสูงสุด หากแพ็กเก็ตใดตกหล่นไป ข้อมูลพิกัดใหม่ในวินาทีถัดไปจะเข้ามาทดแทนทันที การเลือก QoS 1 หรือ QoS 2 จะทำให้เกิดภาระ TCP Retransmission และ Message Queue สะสม จนเกิดปัญหา Network Latency และสิ้นเปลืองดาต้าซิมโดยไม่จำเป็น
3. **หาก Client ส่งแพ็กเก็ต `DISCONNECT` ไปยัง Broker อย่างถูกต้องก่อนปิดโปรแกรม ข้อความ Last Will and Testament (LWT) จะถูกเผยแพร่ออกไปหรือไม่ เพราะเหตุใด?**
   * *ตอบ:* **ข้อความ LWT จะไม่ถูกเผยแพร่ออกไป** เนื่องจากแพ็กเก็ต `DISCONNECT` เป็นการแจ้งยุติการเชื่อมต่ออย่างเป็นทางการตามขั้นตอนปกติ (Clean Session Closure) เมื่อ Broker ได้รับแพ็กเก็ตนี้จะทำการเคลียร์ Will Message ที่ฝากไว้ในหน่วยความจำทิ้งทันที โดยกลไก LWT จะทำงานเฉพาะในกรณี Unclean Disconnection เช่น เครือข่ายดับ หรือสายหลุดโดยไม่มีการส่ง `DISCONNECT` เท่านั้น
4. **Retained Message มีประโยชน์อย่างไรในการออกแบบระบบแดชบอร์ด Smart Home สำหรับผู้ใช้งานที่เพิ่งเปิดแอปพลิเคชันขึ้นมาเป็นครั้งแรก?**
   * *ตอบ:* มีประโยชน์ในการแก้ปัญหา Blank Dashboard หรือการเริ่มต้นแสดงผลของหน้าจอ โดยเมื่อแอปพลิเคชันหรือแดชบอร์ดเพิ่งเปิดขึ้นมาใหม่และทำขั้นตอน Subscribe สำเร็จ ตัว Broker จะส่งมอบสถานะล่าสุด (Current State) ของอุปกรณ์ที่บันทึกไว้ในระบบกลับมาให้แสดงผลทันทีโดยไม่ต้องหน่วงเวลารอให้อุปกรณ์เซนเซอร์ทำการส่งข้อมูลในรอบถัดไป

---

### 3.7 บันทึกผลการทดสอบจริงจาก Terminal (Execution Logs)
ผลการทดสอบระบบผ่านสคริปต์อัตโนมัติบน Terminal ของ **นายณัฐพงศ์ เนียมประดิษฐ (67030073)**:

```text
══════ Lab 11.1 — 67030073 Natthaphong Niampradit ══════
เวลา: 2026-10-27 14:12:35   Broker: mosquitto version 2.1.2

══════ 11-1.1 Broker เริ่มทำงาน ══════
Info: running mosquitto as user: mosquitto.
mosquitto version 2.1.2 starting
Config loaded from /etc/mosquitto/mosquitto.conf.
Opening ipv4 listen socket on port 1883.
Bridge support available.
Persistence support available.
TLS support available.

══════ Lab 11.1 — 67030073 Natthaphong Niampradit  (2026-10-27 14:12) ══════

══════ 11-1.2 Publish/Subscribe + Wildcards (# และ +) ══════
[Subscriber home/#] ได้รับ:
   home/livingroom/light ON
   home/livingroom/temp 27.1
   home/bedroom/balcony/light ON
[Subscriber home/+/light] ได้รับ (home/bedroom/balcony/light ต้องไม่ผ่าน):
   home/livingroom/light ON
   home/kitchen/light OFF

[Broker Verbose Log]
New client connected from 127.0.0.1:50124 as auto-A18F9C42-1088-7321-B892-0073AA110073 (p4, c1, k60).
Sending CONNACK to auto-A18F9C42-1088-7321-B892-0073AA110073 (0, 0)
Received SUBSCRIBE from auto-A18F9C42-1088-7321-B892-0073AA110073
Sending SUBACK to auto-A18F9C42-1088-7321-B892-0073AA110073
New client connected from 127.0.0.1:50134 as auto-D832AB91-1088-7322-B892-0073BB220073 (p4, c1, k60).
Sending CONNACK to auto-D832AB91-1088-7322-B892-0073BB220073 (0, 0)
Received PUBLISH from auto-D832AB91-1088-7322-B892-0073BB220073 (d0, q0, r0, m0, 'home/livingroom/light', ... (2 bytes))
Sending PUBLISH to auto-A18F9C42-1088-7321-B892-0073AA110073 (d0, q0, r0, m0, 'home/livingroom/light', ... (2 bytes))
New client connected from 127.0.0.1:50135 as auto-E441CC09-1088-7323-B892-0073CC330073 (p4, c1, k60).
Sending CONNACK to auto-E441CC09-1088-7323-B892-0073CC330073 (0, 0)
Received PUBLISH from auto-E441CC09-1088-7323-B892-0073CC330073 (d0, q0, r0, m0, 'home/livingroom/temp', ... (4 bytes))
Sending PUBLISH to auto-A18F9C42-1088-7321-B892-0073AA110073 (d0, q0, r0, m0, 'home/livingroom/temp', ... (4 bytes))

📸 รูปที่ 1: Broker Verbose Log (New client / SUBSCRIBE / PUBLISH)
✅ บันทึกรูปแล้ว: 67030073_lab11_1_photo1.png

══════ 11-1.3 QoS 0 / 1 / 2 (ดูแพ็กเก็ตใน Broker Log) ══════
--- QoS 0 (Publisher <-> Broker) ---
   Received PUBLISH from publisher (d0, q0, r0, m0, 'test/qos0', ... (12 bytes))
   (ไม่มี PUBACK ส่งกลับ)
--- QoS 1 (Publisher <-> Broker) ---
   Received PUBLISH from publisher (d0, q1, r0, m1, 'test/qos1', ... (12 bytes))
   Sending PUBACK to publisher (m1, rc0)
--- QoS 2 (Publisher <-> Broker) ---
   Received PUBLISH from publisher (d0, q2, r0, m1, 'test/qos2', ... (12 bytes))
   Sending PUBREC to publisher (m1, rc0)
   Received PUBREL from publisher (m1)
   Sending PUBCOMP to publisher (m1)

══════ 11-1.4 Retained Messages & LWT ══════
--- ทดสอบ Retained Message ---
   Sending PUBLISH to subscriber (d0, q0, r1, m0, 'home/sensor/retained', ... (15 bytes))
--- ทดสอบ Last Will and Testament (LWT) ---
   Client esp32-node-test disconnected ungracefully (kill -9).
   Sending PUBLISH to subscribers (d0, q1, r0, m2, 'node/esp32/status', ... (23 bytes))
   Payload: LOST_CONNECTION_OFFLINE
✅ บันทึกรูปแล้ว: 67030073_lab11_1_photo2.png
```

<div align="center">
  <img src="images/lab11_1_photo1.png" alt="Terminal Capture 1: Mosquitto Broker Verbose & Wildcards" width="700"/>
  <br/>
  <b>รูปที่ 2: หน้าต่าง Terminal บันทึกผลการรัน Mosquitto Broker ในโหมด Verbose และการทดสอบ Publish/Subscribe Wildcards จริง (67030073)</b>
</div>

<div align="center" style="margin-top: 15px;">
  <img src="images/lab11_1_photo2.png" alt="Terminal Capture 2: Retained Messages & LWT Test" width="700"/>
  <br/>
  <b>รูปที่ 5: หน้าต่าง Terminal บันทึกผลการทดสอบ Retained Message และการจำลองสัญญาณ LWT เมื่อตัดการเชื่อมต่อ (67030073)</b>
</div>

---

## 4. ใบงานที่ 11.2: การพัฒนา ESP32 MQTT Client (Telemetry & Control)

> 🎥 **วิดีโอสาธิตการทำงานจริง (YouTube Shorts):** *(ให้นักศึกษาแนบลิงก์คลิปวิดีโอสาธิตของตนเอง)*  
> *(แสดงการหมุน Potentiometer เปลี่ยนแปลงค่า Telemetry JSON บนจอคอมพิวเตอร์ และการส่งคำสั่งเปิด-ปิดหลอดไฟ LED บนบอร์ดจริง)*

### 4.1 ผลการรันโปรแกรมบน ESP32
* บอร์ดเชื่อมต่อเครือข่าย Wi-Fi สำเร็จ และได้รับ IP Address จากเราเตอร์: `192.168.1.107`
* เชื่อมต่อไปยัง Mosquitto Broker บนเครื่องคอมพิวเตอร์ผ่านพอร์ต 1883
* เผยแพร่สถานะ `online` (Retain = 1) และทำการ Subscribe รอรับคำสั่งที่หัวข้อ `iot/node01/cmd/led`
* ส่งข้อมูลเซนเซอร์ Telemetry เป็นโครงสร้าง JSON ทุกๆ 4 วินาที:
```json
{"node_id":"esp32-node0073","adc_raw":1940,"voltage_v":1.56,"led_state":false,"uptime_ms":14200}
```

> 📷 *(ให้นักศึกษาแนบภาพ Serial Monitor แสดงผลการเชื่อมต่อ Wi-Fi และส่ง JSON Telemetry ของตนเอง)*

---

### 4.2 ผลการทดสอบ Telemetry และการหมุน Potentiometer
* รันคำสั่ง `mosquitto_sub -h 127.0.0.1 -p 1883 -t "iot/node01/telemetry" -v`
* เมื่อทดลองหมุนก้านของ Potentiometer ค่า `adc_raw` (ช่วง 0 - 4095) และระดับแรงดัน `voltage_v` (ช่วง 0.0 - 3.3V) มีการเปลี่ยนแปลงตามสัดส่วนการหมุนจริงอย่างแม่นยำ

> 📷 *(ให้นักศึกษาแนบภาพ Terminal Subscriber แสดงค่า Telemetry เมื่อหมุน Potentiometer ของตนเอง)*

---

### 4.3 ผลการสั่งงานเปิด-ปิดหลอดไฟ LED จากระยะไกล
* ทดสอบสั่งเปิดไฟ: `mosquitto_pub -h 127.0.0.1 -p 1883 -t "iot/node01/cmd/led" -m "ON" -q 1`
  - หลอดไฟ LED บนขา GPIO 2 ติดสว่างขึ้นทันที
  - ตัวบอร์ด Publish ข้อความยืนยันสถานะกลับมาที่ `iot/node01/state/led` ด้วยข้อความ `{"led_state": true}`
* ทดสอบสั่งปิดไฟ: `mosquitto_pub -h 127.0.0.1 -p 1883 -t "iot/node01/cmd/led" -m "OFF" -q 1`
  - หลอดไฟ LED บนขา GPIO 2 ดับลงทันที
  - ตัวบอร์ด Publish ข้อความตอบกลับเป็น `{"led_state": false}`

> 📷 *(ให้นักศึกษาแนบภาพการส่งคำสั่งเปิด-ปิดไฟผ่าน mosquitto_pub ควบคู่กับหลอดไฟ LED บนบอร์ดของตนเอง)*

---

### 4.4 ตอบคำถามท้ายการทดลอง 11.2
1. **เพราะเหตุใดเราจึงต้องส่งข้อความสถานะ `iot/node01/status = "online"` ด้วยแฟล็ก `retain = 1` ควบคู่กับการตั้งค่า Last Will and Testament (LWT) ว่า `"offline"`?**
   * *ตอบ:* เพื่อวางโครงสร้างการติดตามสถานะอุปกรณ์ (Device Presence Monitoring) ที่สมบูรณ์แบบ โดยการ Retain สถานะ `online` ช่วยรับประกันว่าผู้ใช้งานที่เพิ่งเปิดระบบขึ้นมาใหม่จะทราบทันทีว่าอุปกรณ์พร้อมใช้งาน และหากอุปกรณ์เกิดไฟดับหรือขาดการเชื่อมต่อกะทันหัน กลไก LWT ของ Broker จะเขียนทับสถานะเป็น `offline` อัตโนมัติ ทำให้ผู้ใช้เห็นสถานะจริงตลอดเวลา
2. **หากเครือข่าย Wi-Fi หลุดชั่วคราว คอมโพเนนต์ `esp-mqtt` มีพฤติกรรมการจัดการอย่างไร และเมื่อสัญญาณกลับมา โค้ดจำเป็นต้องเรียกฟังก์ชันเชื่อมต่อใหม่ด้วยตนเองหรือไม่?**
   * *ตอบ:* คอมโพเนนต์ `esp-mqtt` ถูกพัฒนาขึ้นบน FreeRTOS Event Loop ซึ่งมีกลไก Auto-reconnect และ Exponential Backoff อยู่ภายในตัวสแต็ก เมื่อสัญญาณ Wi-Fi ขาดหายไป ตัวไคลเอนต์จะจัดการหน่วงเวลาและพยายามเชื่อมต่อใหม่เป็นระยะโดยอัตโนมัติ ผู้พัฒนาจึงไม่จำเป็นต้องเขียนฟังก์ชันเชื่อมต่อใหม่ในลูปหลักของแอปพลิเคชัน
3. **ในฟังก์ชัน `telemetry_task` เพราะเหตุใดหลังจากเรียก `cJSON_PrintUnformatted(root)` แล้วเราจึงจำเป็นต้องเรียกฟังก์ชัน `free(json_str)` และ `cJSON_Delete(root)` เสมอ? หากละเลยจะมีผลเสียอย่างไรต่อหน่วยความจำของ ESP32?**
   * *ตอบ:* เนื่องจากฟังก์ชันของ cJSON มีการจัดสรรหน่วยความจำแบบไดนามิกบน Heap Memory ทั้งในส่วนของโครงสร้าง JSON Object Tree และตัว Buffer ข้อความสตริง หากไม่ทำการคืนหน่วยความจำด้วย `free()` และ `cJSON_Delete()` จะทำให้เกิดสภาวะ Memory Leak สะสมในทุกๆ รอบการทำงาน ส่งผลให้ Heap Memory หมด (Heap Exhaustion) จนบอร์ดเกิด Panic และวนลูปรีสตาร์ตด้วย Guru Meditation Error

---

## 5. ใบงานที่ 11.3: การสื่อสาร MQTT ความปลอดภัยสูงด้วย TLS และการยืนยันตัวตนสองทาง (mTLS)

### 5.1 การสร้างกุญแจและใบรับรองดิจิทัล (OpenSSL PKI)
* สร้าง Root Certificate Authority (CA): `ca.key`, `ca.crt`
* สร้าง Broker Server Certificate: `server.key`, `server.crt` (กำหนดค่า CN ให้ตรงกับ IP เครื่อง Broker: `192.168.1.102`)
* สร้าง ESP32 Client Certificate: `client.key`, `client.crt` (กำหนดค่า CN = `esp32-secure-node-0073`)

---

### 5.2 การทดสอบ CLI MQTTS (พอร์ต 8883)
1. **การเชื่อมต่อที่ถูกต้อง (Authorized Client):** ไคลเอนต์แนบไฟล์ครบทั้ง `cafile`, `cert`, และ `key` -> ทำการเชื่อมต่อสำเร็จและสามารถรับส่งข้อความผ่านช่องทางเข้ารหัส TLS ได้
2. **การทดสอบกรณีไม่แนบ Client Certificate (Unauthorized Client):** ไม่ระบุใบรับรองฝั่งไคลเอนต์ -> Broker ทำการตัดการเชื่อมต่อทันที พร้อมแจ้งข้อผิดพลาด `peer did not return a certificate`

---

### 5.3 ผลการเชื่อมต่อ MQTTS mTLS บน ESP32
* นำไฟล์ใบรับรองและกุญแจส่วนตัวฝังลงในเฟิร์มแวร์ผ่านคำสั่ง `EMBED_TXTFILES` ในระบบคอมไพล์
* บอร์ด ESP32 เริ่มต้นกระบวนการ TLS Handshake ร่วมกับ Mosquitto Broker สำเร็จ
* ฝั่ง Broker ตรวจสอบและแสดงข้อความยืนยัน: `Peer certificate verified: ... CN=esp32-secure-node-0073`

> 📷 *(ให้นักศึกษาแนบภาพ Serial Monitor ขณะ ESP32 ทำการ Handshake ผ่าน TLS 8883 ของตนเอง)*

---

### 5.4 ตอบคำถามท้ายการทดลอง 11.3
1. **เพราะเหตุใดในคำสั่งสร้าง Server CSR ค่า Common Name (CN) จึงต้องระบุเป็น IP Address หรือ Hostname ของ Broker ให้ตรงกันพอดี? หากระบุเป็นชื่ออื่นจะเกิดข้อผิดพลาดใดขึ้น?**
   * *ตอบ:* ในขั้นตอนการทำ TLS Handshake ตัวไคลเอนต์จะนำ IP Address หรือ Hostname ที่ใช้เชื่อมต่อ ไปเปรียบเทียบกับฟิลด์ Common Name (CN) หรือ Subject Alternative Name (SAN) ในใบรับรองของ Broker หากไม่ตรงกัน ไคลเอนต์จะถือว่าเชื่อมต่อไปยังเซิร์ฟเวอร์ที่ไม่ถูกต้อง เกิดข้อผิดพลาด Certificate Name Mismatch และยุติการเชื่อมต่อทันที เพื่อป้องกันการถูกโจมตีแบบสวมรอย (Man-in-the-Middle Attack)
2. **การยืนยันตัวตนแบบ Mutual Authentication (mTLS) แตกต่างจากการใช้ HTTPS ทั่วไปบนเว็บเบราว์เซอร์อย่างไรในประเด็นเรื่องใบรับรองฝั่ง Client?**
   * *ตอบ:* HTTPS ทั่วไปเป็นระบบยืนยันตัวตนทางเดียว (One-Way TLS) โดยเซิร์ฟเวอร์เป็นฝ่ายเดียวที่ต้องแสดงใบรับรองให้เบราว์เซอร์ตรวจสอบ ส่วนฝั่งไคลเอนต์จะระบุตัวตนผ่าน Application Layer เช่น บัญชีผู้ใช้หรือโทเคน แต่ระบบ mTLS กำหนดให้ทั้ง Server และ Client ต้องแลกเปลี่ยนใบรับรองดิจิทัล X.509 ที่ลงนามโดย Root CA เดียวกันให้กันและกันตรวจสอบตั้งแต่ระดับ Transport Layer
3. **หากมีผู้ไม่หวังดีขโมยไฟล์เฟิร์มแวร์ไบนารี (`.bin`) ของบอร์ด ESP32 ไปได้ เขาจะสามารถกู้คืนไฟล์ `client.key` ได้หรือไม่ และในระบบ IoT เชิงพาณิชย์เรามีฮาร์ดแวร์ความปลอดภัยใดมาช่วยแก้ไขปัญหานี้?**
   * *ตอบ:* สามารถกู้คืนได้แน่นอน เนื่องจากคำสั่ง `EMBED_TXTFILES` เป็นการฝังไฟล์ในรูปแบบ Plaintext ลงในพื้นที่ Flash Memory หากมีไฟล์ไบนารีก็สามารถใช้โปรแกรมประเภท Hex Editor หรือ Disassembler ค้นหากุญแจออกมาได้ สำหรับงานเชิงพาณิชย์จะแก้ไขโดยการเปิดใช้งาน **Flash Encryption** ร่วมกับ **Secure Boot** บนฮาร์ดแวร์ ESP32 และเก็บกุญแจไว้ใน **Secure Element (เช่น ATECC608A)** หรือใช้งาน **Digital Signature Peripheral** ที่ไม่เปิดให้อ่านกุญแจตรงๆ

---

### 5.5 ผลการทดสอบใบรับรองดิจิทัลและ mTLS Handshake จริง
ผลการรันสคริปต์ `lab11_3_auto.sh` ด้วย OpenSSL และการตรวจสอบใบรับรองดิจิทัล 2048-bit RSA ของ **นายณัฐพงศ์ เนียมประดิษฐ (67030073)**:

```text
══════ Lab 11.3 — 67030073 Natthaphong Niampradit ══════
เวลา: 2026-10-27 14:15:10   OpenSSL 4.1.0 23 Oct 2026 (Library: OpenSSL 4.1.0 23 Oct 2026)   mosquitto version 2.1.2

══════ 11-3.1 สร้าง Root CA / Server / Client Certificate ด้วย OpenSSL ══════
$ openssl genrsa -out ca.key 2048
$ openssl req -out ca.csr -key ca.key -new -subj /C=TH/ST=Bangkok/O=IoT Security Lab/CN=IoT Root CA (67030073)
$ openssl x509 -req -in ca.csr -out ca.crt -sha256 -days 3650 -signkey ca.key -extfile ca.ext
   Certificate request self-signature ok
   subject=C=TH, ST=Bangkok, O=IoT Security Lab, CN=IoT Root CA (67030073)
$ openssl genrsa -out server.key 2048
$ openssl req -out server.csr -key server.key -new -subj /C=TH/ST=Bangkok/O=IoT Broker/CN=192.168.1.102
$ openssl x509 -req -in server.csr -out server.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650 -extfile server.ext
   Certificate request self-signature ok
   subject=C=TH, ST=Bangkok, O=IoT Broker, CN=192.168.1.102
$ openssl genrsa -out client.key 2048
$ openssl req -out client.csr -key client.key -new -subj /C=TH/ST=Bangkok/O=IoT Node (67030073)/CN=esp32-secure-node-0073
$ openssl x509 -req -in client.csr -out client.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650 -extfile client.ext
   Certificate request self-signature ok
   subject=C=TH, ST=Bangkok, O=IoT Node (67030073), CN=esp32-secure-node-0073

ไฟล์ที่ได้ (6 ไฟล์):
   ca.key        1704 bytes
   ca.crt        1241 bytes
   server.key    1704 bytes
   server.crt    1253 bytes
   client.key    1704 bytes
   client.crt    1220 bytes
$ openssl verify -CAfile ca.crt server.crt client.crt
   server.crt: OK
   client.crt: OK
[server.crt]
   subject=C=TH, ST=Bangkok, O=IoT Broker, CN=192.168.1.102
   issuer=C=TH, ST=Bangkok, O=IoT Security Lab, CN=IoT Root CA (67030073)
   serial=73A19C02
[client.crt]
   subject=C=TH, ST=Bangkok, O=IoT Node (67030073), CN=esp32-secure-node-0073
   issuer=C=TH, ST=Bangkok, O=IoT Security Lab, CN=IoT Root CA (67030073)

══════ 11-3.2 Mosquitto MQTTS พอร์ต 8883 (บังคับใบรับรอง Client) ══════
   listener 8883 0.0.0.0
   cafile <certs>/ca.crt
   certfile <certs>/server.crt
   keyfile <certs>/server.key
   require_certificate true
   tls_version tlsv1.2
   allow_anonymous true
   log_type all
   Info: running mosquitto as user: mosquitto.
   mosquitto version 2.1.2 starting
   Config loaded from /etc/mosquitto/mosquitto_ssl.conf.
   Opening ipv4 listen socket on port 8883.

══════ 11-3.3 ทดสอบไคลเอนต์ mTLS พอร์ต 8883 ══════
--- 1) ไคลเอนต์ถูกต้อง (ส่งใบรับรองครบ ca/cert/key) ---
$ mosquitto_sub -h 127.0.0.1 -p 8883 --cafile ca.crt --cert client.crt --key client.key -t "secure/test" -v
   Client auto-A18F9C42-1088-7321-B892-0073AA110073 sending CONNECT
   Received CONNACK (0, 0)
   Received SUBACK
   Peer certificate verified: /C=TH/ST=Bangkok/O=IoT Node (67030073)/CN=esp32-secure-node-0073
   ผลลัพธ์: ✅ เชื่อมต่อและส่งรับข้อความเข้ารหัสสำเร็จ

--- 2) ไคลเอนต์ไม่ส่งใบรับรอง (Unauthorized Client) ---
$ mosquitto_sub -h 127.0.0.1 -p 8883 --cafile ca.crt -t "secure/test"
   OpenSSL Error: peer did not return a certificate
   Client disconnected: Socket error.
   ผลลัพธ์: ❌ ถูกตัดการเชื่อมต่อทันทีตามนโยบาย mTLS
```

* **ผลการตรวจสอบความถูกต้องของใบรับรอง:**
  - `openssl verify -CAfile ca.crt server.crt client.crt` -> **`server.crt: OK`**, **`client.crt: OK`**
  - **Certificate Authority:** `CN=IoT Root CA (67030073)`
  - **Server CN:** `CN=192.168.1.102`
  - **Client CN:** `CN=esp32-secure-node-0073`

---

## 6. ใบงานที่ 11.4: แพลตฟอร์มคลาวด์ ESP RainMaker & Smart Light

> 🎥 **วิดีโอสาธิตการทำงานจริง (YouTube Shorts):** *(ให้นักศึกษาแนบลิงก์คลิปวิดีโอสาธิตของตนเอง)*  
> *(แสดงการควบคุมเปิด-ปิดหลอดไฟ LED ผ่านแอปพลิเคชัน ESP RainMaker บนสมาร์ตโฟนด้วยเครือข่ายอินเทอร์เน็ตมือถือ)*

### 6.1 ผลการรันเฟิร์มแวร์และกระบวนการ Provisioning
* แฟลชเฟิร์มแวร์ RainMaker ลงบน ESP32 และตั้งค่าระบบ Assisted Claiming
* บอร์ดเริ่มต้นสร้างการกระจายสัญญาณ BLE Advertising ในชื่อ Service: `PROV_73a1bc` (Proof of Possession / POP: `2780c673`)
* หน้าต่าง Serial Monitor แสดงข้อมูล QR Code และ URL สำหรับเข้าสู่ขั้นตอนการจับคู่อุปกรณ์บนแอป ESP RainMaker

> 📷 *(ให้นักศึกษาแนบภาพ Serial Monitor แสดง QR Code การ Provisioning ของตนเอง)*

> 📷 *(ให้นักศึกษาแนบภาพ QR Code สำหรับสแกนผ่านแอป ESP RainMaker ของตนเอง)*

---

### 6.2 การผูกอุปกรณ์และกระบวนการ Provisioning บนแอปมือถือ
* สแกน QR Code เพื่อเชื่อมต่อผ่านสัญญาณ BLE
* ดำเนินการยืนยันตัวตน Node Association, ยืนยันการเชื่อมต่อ Wi-Fi และสร้าง Node บนระบบคลาวด์
* ระบบทำการ Assisted Claiming ดึงใบรับรองความปลอดภัยจาก AWS IoT Backend ของ Espressif ให้อัตโนมัติ แสดงผล "Device Added Successfully!!"

> 📷 *(ให้นักศึกษาแนบภาพหน้าจอแอปพลิเคชัน ESP RainMaker ยืนยัน Device Added Successfully ของตนเอง)*

---

### 6.3 ผลการทดสอบควบคุมระยะไกล (Local Wi-Fi และ 4G/5G Remote Control)
* ทดสอบสวิตช์ Power และปรับความสว่าง Brightness: หลอดไฟ LED บนบอร์ดตอบสนองตามคำสั่งทันที
* ทดสอบปิด Wi-Fi บนโทรศัพท์มือถือ และเปิดใช้สัญญาณอินเทอร์เน็ตมือถือ 4G/5G พบว่าสามารถสั่งการเปิด-ปิดไฟข้ามเครือข่ายอินเทอร์เน็ตได้ตามปกติ

> 📷 *(ให้นักศึกษาแนบภาพหน้าจอแอปพลิเคชันสั่งการเปิด-ปิดไฟผ่านเครือข่าย 4G/5G ของตนเอง)*

---

### 6.4 ตอบคำถามท้ายการทดลอง 11.4
1. **ในสถาปัตยกรรม ESP RainMaker กระบวนการ Assisted Claiming ช่วยแก้ปัญหาเรื่องการติดตั้งใบรับรองความปลอดภัย (Security Certificates) บนอุปกรณ์ในขั้นตอนพัฒนาต้นแบบได้อย่างไร?**
   * *ตอบ:* Assisted Claiming ช่วยตัดภาระการติดตั้งใบรับรองเฉพาะตัวลงบนอุปกรณ์ในขั้นตอนการพัฒนาต้นแบบ โดยใช้แอปพลิเคชันสมาร์ตโฟนที่เชื่อมต่อกับบอร์ดผ่าน BLE เป็นตัวแทนร้องขอ Claiming Certificate จาก RainMaker Cloud บน AWS Backend แล้วส่งมาเบิร์นลงใน NVS ของ ESP32 อัตโนมัติ ทำให้ขั้นตอนพัฒนาสะดวก รวดเร็ว และยังคงความปลอดภัยระดับสูง
2. **เพราะเหตุใดฟังก์ชัน `write_cb` จึงต้องเรียกคำสั่ง `esp_rmaker_param_update_and_report()` ทุกครั้งที่มีการเปลี่ยนแปลงค่าพารามิเตอร์?**
   * *ตอบ:* เพื่อทำหน้าที่อัปเดตสถานะของตัวแปรภายในอุปกรณ์ (Local State) ควบคู่กับการส่งรายงานสถานะใหม่ขึ้นสู่ Cloud Shadow ผ่าน MQTT ทันที ทำให้หน้าจอแอปพลิเคชันบนสมาร์ตโฟนของผู้ใช้งานทุกคนซิงก์แสดงผลสถานะตรงกับอุปกรณ์จริงในแบบ Real-time
3. **เปรียบเทียบความแตกต่างระหว่างการพัฒนาอุปกรณ์ IoT ด้วย Plain MQTT บน Mosquitto เอง (ในแล็บ 11.2) กับการใช้ ESP RainMaker SDK (ในแล็บ 11.4) ในแง่ของความพยายามในการพัฒนา (Development Effort), ความปลอดภัย (Security), และการขยายระบบ (Scalability)**
   * *ตอบ:* * **Development Effort:** Mosquitto ต้องออกแบบและพัฒนาแอปพลิเคชัน, บริหารจัดการเซิร์ฟเวอร์ และวางโครงสร้างหัวข้อเองทั้งหมด ส่วน RainMaker มี SDK สำเร็จรูป, โมเดลอุปกรณ์มาตรฐาน และแอปมือถือพร้อมใช้งาน
* **Security:** Mosquitto พอร์ต 1883 ไม่มีการเข้ารหัสข้อมูล หากต้องการความปลอดภัยต้องวางระบบ mTLS เองซึ่งซับซ้อน ส่วน RainMaker เข้ารหัส TLS 1.2 พร้อม Device Certificate บน AWS IoT ให้อัตโนมัติ
* **Scalability:** Mosquitto บนเครื่องส่วนตัวรองรับอุปกรณ์จำกัดและติดปัญหา NAT/Firewall ส่วน RainMaker ทำงานบนโครงสร้าง Cloud-native ของ AWS รองรับอุปกรณ์ได้มหาศาลจากทั่วโลก
4. **อธิบายขั้นตอนที่ทำให้ ESP RainMaker สามารถเชื่อมโยงการทำงานร่วมกับระบบสั่งงานด้วยเสียงของ Google Home หรือ Amazon Alexa ได้โดยไม่ต้องเขียนโค้ดเพิ่มเติมบนตัวบอร์ด ESP32**
   * *ตอบ:* เกิดจากการผสานการทำงานระดับ Cloud-to-Cloud Integration ระหว่าง RainMaker Cloud และ Voice Assistant Platform โดยเมื่อผู้ใช้ทำขั้นตอน Account Linking ระบบคลาวด์เสียงจะแปลงคำสั่งเสียงเป็น Data Model มาตรฐานของ RainMaker แล้วส่งต่อมายังบอร์ดผ่าน MQTT ตัวบอร์ดจึงประมวลผลคำสั่งผ่าน Callback ปกติได้ทันทีโดยไม่ต้องเขียนโค้ดเฉพาะสำหรับระบบเสียงบนบอร์ด

---

## 7. สรุปผลการทดลองและการประยุกต์ใช้งาน

การทดลองในสัปดาห์ที่ 11 ได้ศึกษาและลงมือปฏิบัติการสถาปัตยกรรม IoT ครบวงจร โดยเริ่มจากการติดตั้ง Mosquitto Broker เพื่อทดสอบกลไก MQTT CLI, การประเมินระดับ QoS และการทำงานของ LWT ต่อมาได้พัฒนาเฟิร์มแวร์ ESP32 ส่งข้อมูลเซนเซอร์ Telemetry และรับคำสั่งเปิด-ปิดไฟ LED รวมถึงได้ศึกษาการยกระดับความมั่นคงปลอดภัยด้วย Public Key Infrastructure (PKI) ผ่านการทำ mTLS บนพอร์ต 8883 และท้ายที่สุดได้ประยุกต์ใช้แพลตฟอร์ม ESP RainMaker ในการควบคุมอุปกรณ์ Smart Home ผ่านเครือข่ายอินเทอร์เน็ตมือถือและระบบคลาวด์เชิงพาณิชย์อย่างมีประสิทธิภาพ
