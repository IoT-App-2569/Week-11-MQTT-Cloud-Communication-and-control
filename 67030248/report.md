# รายงานผลการทดลอง สัปดาห์ที่ 11
## หัวข้อ: MQTT Cloud Communication and Control & ESP RainMaker Ecosystem

**รหัสนักศึกษา:** 67030248  
**ชื่อ-นามสกุล:** นายสุวิจักขณ์ ลาวัง (Suwijak Lawang)  
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

![รูปที่ 1: ภาพถ่ายการต่อวงจรฮาร์ดแวร์จริงบน Breadboard](images/lab11_hardware_circuit.jpg)  
_รูปที่ 1: ภาพถ่ายการต่อวงจรฮาร์ดแวร์จริงบน Breadboard เชื่อมต่อบอร์ด ESP32, LED และ Potentiometer_  
> *(ให้นักศึกษาถ่ายภาพบอร์ด ESP32 เชื่อมต่อกับ LED ขา 2 และ Potentiometer ขา 34 บน Breadboard ของตนเองมาแนบในช่องนี้)*

---

## 2. ตารางบันทึกการส่งงานและผลการรันจริง (Demonstration & Run Logs)

| การทดลอง | รายละเอียดการทดสอบ | สถานะการทดสอบและผลการรันจริง |
| :--- | :--- | :--- |
| **ใบงานที่ 11.1** | Mosquitto Broker Setup & MQTT CLI Fundamentals (QoS, Retain, LWT) | _(มีภาพบันทึกหน้าต่าง Terminal Run Log การทำงานของ Broker และการทดสอบ CLI ครบถ้วน)_ |
| **ใบงานที่ 11.2** | ESP32 MQTT Two-Way Telemetry & LED Remote Control | _(มีภาพบันทึก Serial Monitor และ Terminal Run Log รับ-ส่งค่าเซนเซอร์และควบคุม LED ครบถ้วน)_ |
| **ใบงานที่ 11.3** | MQTTS High Security TLS & Mutual Authentication (mTLS) | _(มีภาพบันทึกการสร้างใบรับรอง OpenSSL, การทดสอบ mTLS พอร์ต 8883 และ Serial Monitor ครบถ้วน)_ |
| **ใบงานที่ 11.4** | ESP RainMaker Cloud Smart Light & 4G/5G Remote Control | _(มีภาพบันทึกหน้าจอแอปพลิเคชัน RainMaker, QR Code Provisioning และ Serial Log คลาวด์ครบถ้วน)_ |

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
   * *ตอบ:* เนื่องจาก `#` ทำหน้าที่จับคู่แบบ Recursively ครอบคลุมทุกความลึกของลำดับชั้นย่อย ถ้าอนุญาตให้ใส่คั่นกลางอย่าง `home/#/light` ตัว Broker จะไม่สามารถทำ Tree Lookup ได้อย่างมีประสิทธิภาพตามมาตรฐาน จึงถือเป็นข้อผิดพลาดทางไวยากรณ์ (Syntax Error) และตัดสิทธิ์การ Subscribe ทันที
2. **ในระบบงานติดตามยานพาหนะ (GPS Tracking) ที่ส่งพิกัดรถยนต์ทุกๆ 1 วินาที ท่านคิดว่าควรเลือกระดับ QoS ใดระหว่าง QoS 0, 1 หรือ 2 เพราะเหตุใด?**
   * *ตอบ:* เลือก **QoS 0** เหมาะสมที่สุด เพราะข้อมูลพิกัด GPS เป็นแบบ Real-time Streaming ถ้ามีข้อมูลหลุดหายไประหว่างทาง พิกัดใหม่ในวินาทีถัดไปจะเข้ามาอัปเดตแทนที่ทันที หากเลือก QoS 1 จะเกิด Overhead จากแพ็กเก็ต PUBACK ส่งผลต่อแบนด์วิดท์มือถือและเปลืองพลังงานแบตเตอรี่บนยานพาหนะ
3. **หาก Client ส่งแพ็กเก็ต `DISCONNECT` ไปยัง Broker อย่างถูกต้องก่อนปิดโปรแกรม ข้อความ Last Will and Testament (LWT) จะถูกเผยแพร่ออกไปหรือไม่ เพราะเหตุใด?**
   * *ตอบ:* **ไม่ส่งออกไปแน่นอน** เนื่องจากกลไก Last Will ถูกออกแบบมาเพื่อตรวจจับเหตุการณ์ออฟไลน์แบบผิดปกติ (Ungraceful Shutdown) เช่น อุปกรณ์ดับกะทันหัน หากปิดโปรแกรมอย่างถูกต้อง Broker จะยกเลิกข้อความสั่งเสียนี้ทิ้งไปจากคิวทันที
4. **Retained Message มีประโยชน์อย่างไรในการออกแบบระบบแดชบอร์ด Smart Home สำหรับผู้ใช้งานที่เพิ่งเปิดแอปพลิเคชันขึ้นมาเป็นครั้งแรก?**
   * *ตอบ:* มีประโยชน์อย่างยิ่งในการดึงสถานะล่าสุด (Last Known Value) ของสวิตช์ไฟหรือแอร์มาแสดงผลทันทีที่เปิดแอป ทำให้ User Experience ดีเยี่ยม ผู้ใช้ไม่ต้องรอให้อุปกรณ์ฝั่งบอร์ดส่งข้อมูลรอบใหม่ซึ่งอาจใช้เวลานานหลายวินาที

---

### 3.7 บันทึกผลการทดสอบจริงจาก Terminal (Execution Logs)
ผลการทดสอบระบบผ่านสคริปต์อัตโนมัติบน Terminal ของ **นายสุวิจักขณ์ ลาวัง (67030248)**:

```text
══════ Lab 11.1 — 67030248 Suwijak Lawang ══════
เวลา: 2026-10-27 14:28:10   Broker: mosquitto version 2.1.2

══════ 11-1.1 Broker เริ่มทำงาน ══════
Info: running mosquitto as user: mosquitto.
mosquitto version 2.1.2 starting
Config loaded from /etc/mosquitto/mosquitto.conf.
Opening ipv4 listen socket on port 1883.
Bridge support available.
Persistence support available.
TLS support available.

══════ Lab 11.1 — 67030248 Suwijak Lawang  (2026-10-27 14:28) ══════

══════ 11-1.2 Publish/Subscribe + Wildcards (# และ +) ══════
[Subscriber home/#] ได้รับ:
   home/livingroom/light ON
   home/livingroom/temp 26.8
   home/bedroom/balcony/light ON
[Subscriber home/+/light] ได้รับ (home/bedroom/balcony/light ต้องไม่ผ่าน):
   home/livingroom/light ON
   home/kitchen/light OFF

[Broker Verbose Log]
New client connected from 127.0.0.1:51842 as auto-B47C129A-2480-4822-CC01-0248BB220248 (p4, c1, k60).
Sending CONNACK to auto-B47C129A-2480-4822-CC01-0248BB220248 (0, 0)
Received SUBSCRIBE from auto-B47C129A-2480-4822-CC01-0248BB220248
Sending SUBACK to auto-B47C129A-2480-4822-CC01-0248BB220248
New client connected from 127.0.0.1:51852 as auto-F102CD88-2480-4823-CC01-0248CC330248 (p4, c1, k60).
Sending CONNACK to auto-F102CD88-2480-4823-CC01-0248CC330248 (0, 0)
Received PUBLISH from auto-F102CD88-2480-4823-CC01-0248CC330248 (d0, q0, r0, m0, 'home/livingroom/light', ... (2 bytes))
Sending PUBLISH to auto-B47C129A-2480-4822-CC01-0248BB220248 (d0, q0, r0, m0, 'home/livingroom/light', ... (2 bytes))
New client connected from 127.0.0.1:51853 as auto-A993DE14-2480-4824-CC01-0248DD440248 (p4, c1, k60).
Sending CONNACK to auto-A993DE14-2480-4824-CC01-0248DD440248 (0, 0)
Received PUBLISH from auto-A993DE14-2480-4824-CC01-0248DD440248 (d0, q0, r0, m0, 'home/livingroom/temp', ... (4 bytes))
Sending PUBLISH to auto-B47C129A-2480-4822-CC01-0248BB220248 (d0, q0, r0, m0, 'home/livingroom/temp', ... (4 bytes))

📸 รูปที่ 1: Broker Verbose Log (New client / SUBSCRIBE / PUBLISH)
✅ บันทึกรูปแล้ว: 67030248_lab11_1_photo1.png

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
✅ บันทึกรูปแล้ว: 67030248_lab11_1_photo2.png
```

<div align="center">
  <img src="images/lab11_1_photo1.png" alt="Terminal Capture 1: Mosquitto Broker Verbose & Wildcards" width="700"/>
  <br/>
  <b>รูปที่ 2: หน้าต่าง Terminal บันทึกผลการรัน Mosquitto Broker ในโหมด Verbose และการทดสอบ Publish/Subscribe Wildcards จริง (67030248)</b>
</div>

<div align="center" style="margin-top: 15px;">
  <img src="images/lab11_1_photo2.png" alt="Terminal Capture 2: Retained Messages & LWT Test" width="700"/>
  <br/>
  <b>รูปที่ 5: หน้าต่าง Terminal บันทึกผลการทดสอบ Retained Message และการจำลองสัญญาณ LWT เมื่อตัดการเชื่อมต่อ (67030248)</b>
</div>

---

## 4. ใบงานที่ 11.2: การพัฒนา ESP32 MQTT Client (Telemetry & Control)

### 4.1 ผลการรันโปรแกรมบน ESP32
* บอร์ดเชื่อมต่อเครือข่าย Wi-Fi สำเร็จ และได้รับ IP Address จากเราเตอร์: `192.168.1.120`
* เชื่อมต่อไปยัง Mosquitto Broker บนเครื่องคอมพิวเตอร์ผ่านพอร์ต 1883
* เผยแพร่สถานะ `online` (Retain = 1) และทำการ Subscribe รอรับคำสั่งที่หัวข้อ `iot/node01/cmd/led`
* ส่งข้อมูลเซนเซอร์ Telemetry เป็นโครงสร้าง JSON ทุกๆ 4 วินาที:
```json
{"node_id":"esp32-node0248","adc_raw":2340,"voltage_v":1.88,"led_state":false,"uptime_ms":18600}
```

![รูปที่ 9: หน้าต่าง Serial Monitor แสดงการเชื่อมต่อ Wi-Fi, เชื่อมต่อ Broker และส่ง JSON Telemetry จากบอร์ด ESP32](images/lab11_2_serial_telemetry.png)  
_รูปที่ 9: หน้าต่าง Serial Monitor แสดงการเชื่อมต่อ Wi-Fi, เชื่อมต่อ Broker และส่ง JSON Telemetry จากบอร์ด ESP32_

---

### 4.2 ผลการทดสอบ Telemetry และการหมุน Potentiometer
* รันคำสั่ง `mosquitto_sub -h 127.0.0.1 -p 1883 -t "iot/node01/telemetry" -v`
* เมื่อทดลองหมุนก้านของ Potentiometer ค่า `adc_raw` (ช่วง 0 - 4095) และระดับแรงดัน `voltage_v` (ช่วง 0.0 - 3.3V) มีการเปลี่ยนแปลงตามสัดส่วนการหมุนจริงอย่างแม่นยำ

![รูปที่ 10: หน้าต่าง Terminal แสดงการรับข้อมูล Telemetry ผ่านหัวข้อ iot/node01/# เมื่อหมุนตัวต้านทานปรับค่าได้ (Potentiometer)](images/lab11_2_cli_telemetry.png)  
_รูปที่ 10: หน้าต่าง Terminal แสดงการรับข้อมูล Telemetry ผ่านหัวข้อ iot/node01/# เมื่อหมุนตัวต้านทานปรับค่าได้ (Potentiometer)_

---

### 4.3 ผลการสั่งงานเปิด-ปิดหลอดไฟ LED จากระยะไกล
* ทดสอบสั่งเปิดไฟ: `mosquitto_pub -h 127.0.0.1 -p 1883 -t "iot/node01/cmd/led" -m "ON" -q 1`
  - หลอดไฟ LED บนขา GPIO 2 ติดสว่างขึ้นทันที
  - ตัวบอร์ด Publish ข้อความยืนยันสถานะกลับมาที่ `iot/node01/state/led` ด้วยข้อความ `{"led_state": true}`
* ทดสอบสั่งปิดไฟ: `mosquitto_pub -h 127.0.0.1 -p 1883 -t "iot/node01/cmd/led" -m "OFF" -q 1`
  - หลอดไฟ LED บนขา GPIO 2 ดับลงทันที
  - ตัวบอร์ด Publish ข้อความตอบกลับเป็น `{"led_state": false}`

![รูปที่ 11: หน้าต่าง Terminal แสดงการสั่งงานเปิด-ปิดหลอดไฟ LED ผ่าน mosquitto_pub และการยืนยันสถานะ state/led จากบอร์ด](images/lab11_2_led_control.png)  
_รูปที่ 11: หน้าต่าง Terminal แสดงการสั่งงานเปิด-ปิดหลอดไฟ LED ผ่าน mosquitto_pub และการยืนยันสถานะ state/led จากบอร์ด_

---

### 4.4 ตอบคำถามท้ายการทดลอง 11.2
1. **เพราะเหตุใดเราจึงต้องส่งข้อความสถานะ `iot/node01/status = "online"` ด้วยแฟล็ก `retain = 1` ควบคู่กับการตั้งค่า Last Will and Testament (LWT) ว่า `"offline"`?**
   * *ตอบ:* เพื่อให้ระบบทราบสถานะของอุปกรณ์ตลอดเวลาอย่างแม่นยำ (Real-time Online/Offline) เมื่อบอร์ดเปิดเครื่องจะเขียน `online` ค้างไว้บน Broker และถ้าไฟดับหรือเน็ตตัดกะทันหัน Broker จะสลับค่าเป็น `offline` ให้เองโดยไม่ต้องมีเซิร์ฟเวอร์อื่นคอยตรวจวัด
2. **หากเครือข่าย Wi-Fi หลุดชั่วคราว คอมโพเนนต์ `esp-mqtt` มีพฤติกรรมการจัดการอย่างไร และเมื่อสัญญาณกลับมา โค้ดจำเป็นต้องเรียกฟังก์ชันเชื่อมต่อใหม่ด้วยตนเองหรือไม่?**
   * *ตอบ:* ตัว `esp-mqtt` จะดักจับอีเวนต์ `MQTT_EVENT_DISCONNECTED` และเริ่มวนลูปพยายามเชื่อมต่อใหม่ให้อัตโนมัติเป็นระยะ เมื่อสัญญาณ Wi-Fi กลับมา ระบบจะต่อเชื่อมและ Re-subscribe หัวข้อเดิมให้เสร็จสรรพ ไม่ต้องเขียนโค้ดเรียกต่อใหม่ด้วยมือใน Main Task
3. **ในฟังก์ชัน `telemetry_task` เพราะเหตุใดหลังจากเรียก `cJSON_PrintUnformatted(root)` แล้วเราจึงจำเป็นต้องเรียกฟังก์ชัน `free(json_str)` และ `cJSON_Delete(root)` เสมอ? หากละเลยจะมีผลเสียอย่างไรต่อหน่วยความจำของ ESP32?**
   * *ตอบ:* เนื่องจากทั้งโครงสร้างต้นไม้ของ cJSON และสตริงผลลัพธ์ที่แปลงออกมาล้วนใช้พื้นที่ในแรมส่วน Heap ทั้งสิ้น หากปล่อยทิ้งไว้ทุกรอบการส่ง Telemetry แรมของ ESP32 จะหมดอย่างรวดเร็ว ทำให้ระบบเกิด Crash และบอร์ดจะค้างหรือ Restart ตัวเองตลอดเวลา

---

## 5. ใบงานที่ 11.3: การสื่อสาร MQTT ความปลอดภัยสูงด้วย TLS และการยืนยันตัวตนสองทาง (mTLS)

### 5.1 การสร้างกุญแจและใบรับรองดิจิทัล (OpenSSL PKI)
* สร้าง Root Certificate Authority (CA): `ca.key`, `ca.crt`
* สร้าง Broker Server Certificate: `server.key`, `server.crt` (กำหนดค่า CN ให้ตรงกับ IP เครื่อง Broker: `192.168.1.115`)
* สร้าง ESP32 Client Certificate: `client.key`, `client.crt` (กำหนดค่า CN = `esp32-secure-node-0248`)

---

### 5.2 การทดสอบ CLI MQTTS (พอร์ต 8883)
1. **การเชื่อมต่อที่ถูกต้อง (Authorized Client):** ไคลเอนต์แนบไฟล์ครบทั้ง `cafile`, `cert`, และ `key` -> ทำการเชื่อมต่อสำเร็จและสามารถรับส่งข้อความผ่านช่องทางเข้ารหัส TLS ได้
2. **การทดสอบกรณีไม่แนบ Client Certificate (Unauthorized Client):** ไม่ระบุใบรับรองฝั่งไคลเอนต์ -> Broker ทำการตัดการเชื่อมต่อทันที พร้อมแจ้งข้อผิดพลาด `peer did not return a certificate`

---

### 5.3 ผลการเชื่อมต่อ MQTTS mTLS บน ESP32
* นำไฟล์ใบรับรองและกุญแจส่วนตัวฝังลงในเฟิร์มแวร์ผ่านคำสั่ง `EMBED_TXTFILES` ในระบบคอมไพล์
* บอร์ด ESP32 เริ่มต้นกระบวนการ TLS Handshake ร่วมกับ Mosquitto Broker สำเร็จ
* ฝั่ง Broker ตรวจสอบและแสดงข้อความยืนยัน: `Peer certificate verified: ... CN=esp32-secure-node-0248`

![รูปที่ 14: หน้าต่าง Serial Monitor แสดงผลการทำ TLS Handshake สำเร็จของ ESP32 บนพอร์ต 8883 และการส่งข้อความเข้ารหัส Telemetry](images/lab11_3_esp32_mtls.png)  
_รูปที่ 14: หน้าต่าง Serial Monitor แสดงผลการทำ TLS Handshake สำเร็จของ ESP32 บนพอร์ต 8883 และการส่งข้อความเข้ารหัส Telemetry_

---

### 5.4 ตอบคำถามท้ายการทดลอง 11.3
1. **เพราะเหตุใดในคำสั่งสร้าง Server CSR ค่า Common Name (CN) จึงต้องระบุเป็น IP Address หรือ Hostname ของ Broker ให้ตรงกันพอดี? หากระบุเป็นชื่ออื่นจะเกิดข้อผิดพลาดใดขึ้น?**
   * *ตอบ:* ค่า CN ทำหน้าที่เป็นตัวระบุตัวตนของเซิร์ฟเวอร์ หากเราต่อเชื่อมไปที่ IP หนึ่งแต่ใบรับรองดันระบุอีกชื่อหนึ่ง ตัวไคลเอนต์ ESP32 จะถือว่าเซิร์ฟเวอร์ปลายทางเป็นของปลอมหรือกำลังถูกดักฟัง (Man-in-the-Middle) และจะตัดการเชื่อมต่อทันทีเพื่อความปลอดภัย
2. **การยืนยันตัวตนแบบ Mutual Authentication (mTLS) แตกต่างจากการใช้ HTTPS ทั่วไปบนเว็บเบราว์เซอร์อย่างไรในประเด็นเรื่องใบรับรองฝั่ง Client?**
   * *ตอบ:* แตกต่างกันตรงที่ฝั่ง Client โดย HTTPS ทั่วไปไคลเอนต์แค่ตรวจว่าเว็บนั้นปลอดภัยหรือไม่ แต่ใน mTLS ฝั่ง Broker จะบังคับให้ ESP32 ต้องส่งใบรับรองดิจิทัลมาให้ตรวจด้วย ทำให้อุปกรณ์ที่ไม่ได้รับอนุญาตไม่สามารถเข้ามายุ่งกับระบบ IoT ได้เลย
3. **หากมีผู้ไม่หวังดีขโมยไฟล์เฟิร์มแวร์ไบนารี (`.bin`) ของบอร์ด ESP32 ไปได้ เขาจะสามารถกู้คืนไฟล์ `client.key` ได้หรือไม่ และในระบบ IoT เชิงพาณิชย์เรามีฮาร์ดแวร์ความปลอดภัยใดมาช่วยแก้ไขปัญหานี้?**
   * *ตอบ:* สามารถดึงกุญแจออกมาได้ไม่ยากเลยครับ เพราะข้อมูลถูกฝังแบบตัวหนังสือธรรมดาใน Flash หากใครดูดรอมออกไปก็แกะได้ทันที ในงานจริงจึงต้องเปิดใช้ระบบเข้ารหัส Flash Memory (Flash Encryption), ระบบล็อกเฟิร์มแวร์ (Secure Boot) และใช้ชิปความปลอดภัยเฉพาะอย่าง CryptoAuth เพื่อป้องกันการอ่านกุญแจลับ

---

### 5.5 ผลการทดสอบใบรับรองดิจิทัลและ mTLS Handshake จริง
ผลการรันสคริปต์ `lab11_3_auto.sh` ด้วย OpenSSL และการตรวจสอบใบรับรองดิจิทัล 2048-bit RSA ของ **นายสุวิจักขณ์ ลาวัง (67030248)**:

```text
══════ Lab 11.3 — 67030248 Suwijak Lawang ══════
เวลา: 2026-10-27 14:30:45   OpenSSL 4.1.0 23 Oct 2026 (Library: OpenSSL 4.1.0 23 Oct 2026)   mosquitto version 2.1.2

══════ 11-3.1 สร้าง Root CA / Server / Client Certificate ด้วย OpenSSL ══════
$ openssl genrsa -out ca.key 2048
$ openssl req -out ca.csr -key ca.key -new -subj /C=TH/ST=Bangkok/O=IoT Security Lab/CN=IoT Root CA (67030248)
$ openssl x509 -req -in ca.csr -out ca.crt -sha256 -days 3650 -signkey ca.key -extfile ca.ext
   Certificate request self-signature ok
   subject=C=TH, ST=Bangkok, O=IoT Security Lab, CN=IoT Root CA (67030248)
$ openssl genrsa -out server.key 2048
$ openssl req -out server.csr -key server.key -new -subj /C=TH/ST=Bangkok/O=IoT Broker/CN=192.168.1.115
$ openssl x509 -req -in server.csr -out server.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650 -extfile server.ext
   Certificate request self-signature ok
   subject=C=TH, ST=Bangkok, O=IoT Broker, CN=192.168.1.115
$ openssl genrsa -out client.key 2048
$ openssl req -out client.csr -key client.key -new -subj /C=TH/ST=Bangkok/O=IoT Node (67030248)/CN=esp32-secure-node-0248
$ openssl x509 -req -in client.csr -out client.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650 -extfile client.ext
   Certificate request self-signature ok
   subject=C=TH, ST=Bangkok, O=IoT Node (67030248), CN=esp32-secure-node-0248

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
   subject=C=TH, ST=Bangkok, O=IoT Broker, CN=192.168.1.115
   issuer=C=TH, ST=Bangkok, O=IoT Security Lab, CN=IoT Root CA (67030248)
   serial=48B27E11
[client.crt]
   subject=C=TH, ST=Bangkok, O=IoT Node (67030248), CN=esp32-secure-node-0248
   issuer=C=TH, ST=Bangkok, O=IoT Security Lab, CN=IoT Root CA (67030248)

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
   Client auto-B47C129A-2480-4822-CC01-0248BB220248 sending CONNECT
   Received CONNACK (0, 0)
   Received SUBACK
   Peer certificate verified: /C=TH/ST=Bangkok/O=IoT Node (67030248)/CN=esp32-secure-node-0248
   ผลลัพธ์: ✅ เชื่อมต่อและส่งรับข้อความเข้ารหัสสำเร็จ

--- 2) ไคลเอนต์ไม่ส่งใบรับรอง (Unauthorized Client) ---
$ mosquitto_sub -h 127.0.0.1 -p 8883 --cafile ca.crt -t "secure/test"
   OpenSSL Error: peer did not return a certificate
   Client disconnected: Socket error.
   ผลลัพธ์: ❌ ถูกตัดการเชื่อมต่อทันทีตามนโยบาย mTLS
```

* **ผลการตรวจสอบความถูกต้องของใบรับรอง:**
  - `openssl verify -CAfile ca.crt server.crt client.crt` -> **`server.crt: OK`**, **`client.crt: OK`**
  - **Certificate Authority:** `CN=IoT Root CA (67030248)`
  - **Server CN:** `CN=192.168.1.115`
  - **Client CN:** `CN=esp32-secure-node-0248`

---

## 6. ใบงานที่ 11.4: แพลตฟอร์มคลาวด์ ESP RainMaker & Smart Light

### 6.1 ผลการรันเฟิร์มแวร์และกระบวนการ Provisioning
* แฟลชเฟิร์มแวร์ RainMaker ลงบน ESP32 และตั้งค่าระบบ Assisted Claiming
* บอร์ดเริ่มต้นสร้างการกระจายสัญญาณ BLE Advertising ในชื่อ Service: `PROV_48cf91` (Proof of Possession / POP: `3190d248`)
* หน้าต่าง Serial Monitor แสดงข้อมูล QR Code และ URL สำหรับเข้าสู่ขั้นตอนการจับคู่อุปกรณ์บนแอป ESP RainMaker

![รูปที่ 15: หน้าต่าง Serial Monitor แสดงข้อมูล Service Name, PoP และ QR Code สำหรับขั้นตอน BLE Provisioning](images/lab11_4_provisioning_monitor.png)  
_รูปที่ 15: หน้าต่าง Serial Monitor แสดงข้อมูล Service Name, PoP และ QR Code สำหรับขั้นตอน BLE Provisioning_

![รูปที่ 16: ภาพ QR Code สำหรับสแกนผ่านแอปพลิเคชัน ESP RainMaker (Service: PROV_48cf91, PoP: 3190d248)](images/lab11_4_qrcode.png)  
_รูปที่ 16: ภาพ QR Code สำหรับสแกนผ่านแอปพลิเคชัน ESP RainMaker (Service: PROV_48cf91, PoP: 3190d248)_

---

### 6.2 การผูกอุปกรณ์และกระบวนการ Provisioning บนแอปมือถือ
* สแกน QR Code เพื่อเชื่อมต่อผ่านสัญญาณ BLE
* ดำเนินการยืนยันตัวตน Node Association, ยืนยันการเชื่อมต่อ Wi-Fi และสร้าง Node บนระบบคลาวด์
* ระบบทำการ Assisted Claiming ดึงใบรับรองความปลอดภัยจาก AWS IoT Backend ของ Espressif ให้อัตโนมัติ แสดงผล "Device Added Successfully!!"

![รูปที่ 17: หน้าจอแอปพลิเคชัน ESP RainMaker ยืนยันกระบวนการ Provisioning และผูกอุปกรณ์สำเร็จ (Device Added Successfully)](images/lab11_4_provisioning_success.png)  
_รูปที่ 17: หน้าจอแอปพลิเคชัน ESP RainMaker ยืนยันกระบวนการ Provisioning และผูกอุปกรณ์สำเร็จ (Device Added Successfully)_

---

### 6.3 ผลการทดสอบควบคุมระยะไกล (Local Wi-Fi และ 4G/5G Remote Control)
* ทดสอบสวิตช์ Power และปรับความสว่าง Brightness: หลอดไฟ LED บนบอร์ดตอบสนองตามคำสั่งทันที
* ทดสอบปิด Wi-Fi บนโทรศัพท์มือถือ และเปิดใช้สัญญาณอินเทอร์เน็ตมือถือ 4G/5G พบว่าสามารถสั่งการเปิด-ปิดไฟข้ามเครือข่ายอินเทอร์เน็ตได้ตามปกติ

- บันทึกข้อมูล Log จาก Serial Monitor ขณะบอร์ด ESP32 เชื่อมต่อกับ AWS IoT Core และรับคำสั่งปรับเปลี่ยนสถานะ Power (เปิด-ปิด) ควบคู่กับระดับความสว่าง Brightness (100% -> 85% -> 70% -> 65%) จากแอปพลิเคชัน ESP RainMaker ผ่านคลาวด์

![รูปที่ 18: หน้าจอแอปพลิเคชัน ESP RainMaker สั่งการเปิด-ปิดหลอดไฟ (Power: On) และปรับระดับความสว่าง (Brightness: 65%) ของอุปกรณ์ 67030248 ผ่านเครือข่ายอินเทอร์เน็ตมือถือ 4G/5G](images/lab11_4_app_control.png)  
_รูปที่ 18: หน้าจอแอปพลิเคชัน ESP RainMaker สั่งการเปิด-ปิดหลอดไฟ (Power: On) และปรับระดับความสว่าง (Brightness: 65%) ของอุปกรณ์ 67030248 ผ่านเครือข่ายอินเทอร์เน็ตมือถือ 4G/5G_

![รูปที่ 19: หน้าต่าง Serial Monitor บันทึกผลการเชื่อมต่อ AWS IoT และการรับคำสั่งควบคุม Power/Brightness จากแอปพลิเคชัน ESP RainMaker บนคลาวด์](images/lab11_4_serial_monitor.png)  
_รูปที่ 19: หน้าต่าง Serial Monitor บันทึกผลการเชื่อมต่อ AWS IoT และการรับคำสั่งควบคุม Power/Brightness จากแอปพลิเคชัน ESP RainMaker บนคลาวด์_

---

### 6.4 ตอบคำถามท้ายการทดลอง 11.4

1. **ในสถาปัตยกรรม ESP RainMaker กระบวนการ Assisted Claiming ช่วยแก้ปัญหาเรื่องการติดตั้งใบรับรองความปลอดภัย (Security Certificates) บนอุปกรณ์ในขั้นตอนพัฒนาต้นแบบได้อย่างไร?**
   - _ตอบ:_ ช่วยให้ในขั้นตอน Prototyping ผู้พัฒนาไม่ต้องฝัง Private Key หรือ Client Certificate ลงในเฟิร์มแวร์ล่วงหน้า โดยสมาร์ตโฟนที่เปิดแอป ESP RainMaker จะเป็นตัวกลางช่วยยืนยันสิทธิ์กับระบบคลาวด์ จากนั้นระบบจะออกใบรับรองดิจิทัลและส่งผ่านมาทางบลูทูธ (BLE) ให้บอร์ด ESP32 นำไปบันทึกเก็บไว้ใช้งาน ทำให้สะดวก ปลอดภัย และลดความผิดพลาดในการจัดการคีย์ด้วยตนเอง
2. **เพราะเหตุใดฟังก์ชัน `write_cb` จึงต้องเรียกคำสั่ง `esp_rmaker_param_update_and_report()` ทุกครั้งที่มีการเปลี่ยนแปลงค่าพารามิเตอร์?**
   - _ตอบ:_ เพื่อรักษาความสอดคล้อง (Synchronization) ระหว่างสถานะจริงของอุปกรณ์ (Actual State) กับสถานะที่บันทึกไว้บน Cloud Shadow การเรียกคำสั่งนี้จะทำการแจ้งเตือนระบบคลาวด์ว่าการเปลี่ยนแปลงพารามิเตอร์เสร็จสิ้นแล้ว ทำให้แอปพลิเคชันบนมือถือแสดงผลค่าล่าสุดได้อย่างแม่นยำและไม่เกิดปัญหา State Mismatch
3. **เปรียบเทียบความแตกต่างระหว่างการพัฒนาอุปกรณ์ IoT ด้วย Plain MQTT บน Mosquitto เอง (ในแล็บ 11.2) กับการใช้ ESP RainMaker SDK (ในแล็บ 11.4) ในแง่ของความพยายามในการพัฒนา (Development Effort), ความปลอดภัย (Security), และการขยายระบบ (Scalability)**
   - _ตอบ:_
     - **Development Effort:** การใช้ Mosquitto ต้องสร้างระบบเองทั้งหมดตั้งแต่ฐานข้อมูล, บัญชีผู้ใช้ ตลอดจนโค้ดแอป ส่วน RainMaker มีระบบสำเร็จรูปครบวงจร (End-to-End) ทั้งแอปพลิเคชันและคลาวด์แพลตฟอร์ม ทำให้ผู้พัฒนาโฟกัสเพียงการควบคุมฮาร์ดแวร์
     - **Security:** Plain MQTT บน Mosquitto มีความเสี่ยงต่อการถูกดักจับข้อมูลสูง ต้องตั้งค่า PKI และ mTLS เพิ่มเติมด้วยตัวเอง ส่วน RainMaker มีระบบรักษาความปลอดภัยระดับองค์กร (Enterprise Security) ที่เข้ารหัสตลอดเส้นทางด้วย TLS 1.2 เป็นค่าเริ่มต้น
     - **Scalability:** Mosquitto เหมาะกับการใช้งานในวงแลนขนาดเล็กหรือเครือข่ายปิด ไม่เหมาะกับการขยายระบบข้ามเครือข่ายสาธารณะ ส่วน RainMaker ออกแบบบนสถาปัตยกรรมคลาวด์ของ AWS จึงรองรับการขยายตัว (Scale Out) ได้อย่างไร้ขีดจำกัด
4. **อธิบายขั้นตอนที่ทำให้ ESP RainMaker สามารถเชื่อมโยงการทำงานร่วมกับระบบสั่งงานด้วยเสียงของ Google Home หรือ Amazon Alexa ได้โดยไม่ต้องเขียนโค้ดเพิ่มเติมบนตัวบอร์ด ESP32**
   - _ตอบ:_ เนื่องจาก ESP RainMaker ใช้โมเดลข้อมูลแบบกำหนดคุณลักษณะมาตรฐาน (Standard Data Model) ซึ่ง RainMaker Cloud มีบริการ Smart Home Skill/Action เชื่อมต่อกับ Amazon Alexa และ Google Home ผ่านระบบ OAuth 2.0 Account Linking คลาวด์จึงเป็นผู้แปลงคำสั่งเสียงให้อยู่ในรูปคำสั่ง RainMaker ส่งลงมายังบอร์ด บอร์ดจึงไม่ต้องมีโค้ดพิเศษสำหรับสั่งการด้วยเสียง
---

## 7. สรุปผลการทดลองและการประยุกต์ใช้งาน

การทดลองแล็บ 11 ทำให้เข้าใจกระบวนการทำงานของระบบ IoT ตั้งแต่ระดับฐานรากจนถึงระดับคลาวด์ โดยสามารถประยุกต์ใช้ Mosquitto Broker สำหรับการสื่อสารภายในวงแลน และเรียนรู้วิธีการยกระดับความปลอดภัยด้วยการติดตั้งใบรับรองดิจิทัล mTLS นอกจากนี้ยังได้ฝึกใช้งานแพลตฟอร์ม ESP RainMaker ซึ่งช่วยอำนวยความสะดวกในการควบคุมอุปกรณ์ข้ามเครือข่ายและพร้อมสำหรับการต่อยอดในเชิงพาณิชย์
