# รายงานผลการทดลองและตอบคำถามท้ายการทดลอง ใบงานที่ 11.1
## เรื่อง: การติดตั้ง Mosquitto Broker และการทดสอบโปรโตคอล MQTT ด้วย CLI (Pub/Sub, QoS, Retain, LWT)

**รหัสนักศึกษา:** 67030181  
**วิชา:** IoT Cloud Communication and Control (สัปดาห์ที่ 11)  

---

## 1. วัตถุประสงค์การทดลอง
1. สามารถติดตั้งและปรับแต่งค่าคอนฟิกูเรชันของ **Eclipse Mosquitto MQTT Broker** ผ่านไฟล์ `mosquitto.conf` ได้อย่างถูกต้อง
2. สามารถใช้งานคำสั่ง `mosquitto_sub` และ `mosquitto_pub` ในการบอกรับ (Subscribe) และเผยแพร่ (Publish) ข้อความผ่านเครือข่ายได้
3. เข้าใจความแตกต่างและทดสอบการทำงานของตัวกรองไวลด์การ์ดระดับเดียว (`+`) และหลายระดับ (`#`)
4. สามารถวิเคราะห์และเปรียบเทียบกลไกของแพ็กเก็ตควบคุมในการรับส่งข้อมูลระดับ QoS 0, QoS 1 และ QoS 2 ผ่าน Verbose Log ของ Broker
5. สามารถอธิบายและสาธิตการทำงานจริงของคุณสมบัติข้อความคงค้าง (Retained Message) และพินัยกรรมฉุกเฉิน (Last Will and Testament - LWT)

---

## 2. อุปกรณ์และเครื่องมือที่ใช้
* **คอมพิวเตอร์:** ระบบปฏิบัติการ Windows 11 x64
* **ซอฟต์แวร์ Broker:** Eclipse Mosquitto v2.0.18 (`mosquitto.exe`)
* **เครื่องมือไคลเอนต์:** `mosquitto_sub.exe` (Subscriber) และ `mosquitto_pub.exe` (Publisher)
* **หน้าต่างเทอร์มินัล:** PowerShell จำนวน 4 หน้าต่าง ได้แก่:
  1. Window 1: Mosquitto Broker (รันโหมด `-v`)
  2. Window 2: Subscriber
  3. Window 3: Publisher 1
  4. Window 4: Publisher 2 / LWT Node Simulator

---

## 3. สรุปขั้นตอนและผลการทำกิจกรรม (Experimental Activities)

### กิจกรรมที่ 11-1.1: การติดตั้งและเริ่มต้น Mosquitto Broker
* สร้างไฟล์คอนฟิก `mosquitto.conf` ที่ระบุ:
  ```text
  listener 1883 0.0.0.0
  allow_anonymous true
  log_type all
  ```
* ตรวจสอบและปิด Windows Service `mosquitto` เพื่อป้องกันปัญหา **Split-Brain Broker (Service รันซ้อนทับพอร์ต 1883 บน `127.0.0.1`)** ด้วยคำสั่ง:
  ```powershell
  Stop-Service mosquitto
  Set-Service mosquitto -StartupType Manual
  ```
* รัน Broker ในเทอร์มินัลที่ 1 ด้วยคำสั่ง:
  ```powershell
  & "C:\Program Files\mosquitto\mosquitto.exe" -c ".\mosquitto.conf" -v
  ```
* ผลการเริ่มต้น: Broker เปิดพอร์ต `1883` และพร้อมรับการเชื่อมต่อจากทุกเครือข่าย

---

### กิจกรรมที่ 11-1.2: การทดสอบ Publish / Subscribe พื้นฐาน และการใช้ Wildcards

#### 1. การทดสอบแบบ Exact Topic
* Subscriber: `& mosquitto_sub.exe -h 127.0.0.1 -p 1883 -t "home/livingroom/light" -v`
* Publisher: `& mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "home/livingroom/light" -m "ON"`
* **ผลลัพธ์:** Subscriber ได้รับข้อความ `home/livingroom/light ON` ทันที

#### 2. การทดสอบไวลด์การ์ดระดับเดียว (Single-Level Wildcard: `+`)
* Subscriber: `& mosquitto_sub.exe -h 127.0.0.1 -p 1883 -t "home/+/light" -v`
* ทดสอบ Publish 3 หัวข้อ:
  1. `home/livingroom/light` (Payload: `"ON"`) -> **ได้รับข้อความ** เพราะระดับที่ 2 คือ `livingroom` แมตช์กับเครื่องหมาย `+`
  2. `home/kitchen/light` (Payload: `"OFF"`) -> **ได้รับข้อความ** เพราะระดับที่ 2 คือ `kitchen` แมตช์กับเครื่องหมาย `+`
  3. `home/bedroom/balcony/light` (Payload: `"ON"`) -> **ไม่ได้รับข้อความ** เนื่องจากหัวข้อนี้มีลำดับชั้นย่อย 4 ระดับ ในขณะที่ตัวกรอง `home/+/light` กำหนดโครงสร้างไว้คงที่ 3 ระดับเท่านั้น

#### 3. การทดสอบไวลด์การ์ดหลายระดับ (Multi-Level Wildcard: `#`)
* Subscriber: `& mosquitto_sub.exe -h 127.0.0.1 -p 1883 -t "home/#" -v`
* ทดสอบ Publish:
  1. `home/livingroom/temp` (Payload: `"26.4"`) -> **ได้รับข้อความ**
  2. `home/bedroom/balcony/light` (Payload: `"ON"`) -> **ได้รับข้อความ**
* **ผลลัพธ์:** ทุกหัวข้อที่มีคำนำหน้าเป็น `home/` ไม่ว่าจะมีลำดับชั้นลึกกี่ระดับ จะถูกส่งมายัง Subscriber ทั้งหมด

---

### กิจกรรมที่ 11-1.3: การทดสอบระดับคุณภาพการส่งข้อมูล (QoS 0, 1, 2)
* **QoS 0 (At most once):** ส่งแพ็กเก็ต `PUBLISH` ครั้งเดียว ไม่มีการส่งแพ็กเก็ตตอบรับ (No ACK)
* **QoS 1 (At least once):** มีการแนบ Packet ID (เช่น `Mid: 1`) และฝั่งผู้รับจะตอบกลับด้วยแพ็กเก็ต `PUBACK (Mid: 1)` ยืนยันการรับข้อมูล
* **QoS 2 (Exactly once):** เกิดกระบวนการ 4-way Handshake ระหว่างโฮสต์:
  1. `PUBLISH` (q2)
  2. `PUBREC` (Publish Received)
  3. `PUBREL` (Publish Release)
  4. `PUBCOMP` (Publish Complete)

---

### กิจกรรมที่ 11-1.4: การทดสอบข้อความคงค้าง (Retained Messages)
1. เมื่อ Subscriber ไม่ได้เปิดอยู่ แล้ว Publisher ส่งข้อความปกติ (ไม่มี `-r`) -> ข้อความจะสูญหาย Subscriber ที่เปิดทีหลังจะไม่ได้รับ
2. เมื่อ Publisher ส่งข้อความโดยใส่แฟล็ก `-r`:
   ```powershell
   & mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "sensor/state" -m "CRITICAL_ALERT" -r
   ```
3. เมื่อเปิด Subscriber ภายหลัง:
   ```powershell
   & mosquitto_sub.exe -h 127.0.0.1 -p 1883 -t "sensor/state" -v
   ```
   **ผลลัพธ์:** Subscriber ได้รับข้อความ `sensor/state CRITICAL_ALERT` ทันทีที่เชื่อมต่อสำเร็จ แม้ Publisher จะไม่ได้ส่งข้อความใหม่
4. การล้าง Retained Message: ส่ง Empty Payload พร้อมแฟล็ก `-n -r`

---

### กิจกรรมที่ 11-1.5: การทดสอบพินัยกรรมฉุกเฉิน (Last Will and Testament - LWT)
1. Subscriber สมัครรับหัวข้อ `node/esp32/status`
2. จำลอง ESP32 โหนดที่ 1 เชื่อมต่อเข้า Broker โดยกำหนด Will parameters ไว้ล่วงหน้า:
   ```powershell
   & mosquitto_sub.exe -h 127.0.0.1 -p 1883 -i "Node_Client_01" -t "dummy/listen" --will-topic "node/esp32/status" --will-payload "LOST_CONNECTION_OFFLINE" --will-qos 1
   ```
3. ส่งสถานะเริ่มต้น `"ONLINE"`
4. จำลองไฟฟ้าดับ/สายหลุด โดยการปิด Terminal ของ `Node_Client_01` ทันที (Unclean Disconnect / No DISCONNECT packet)
5. **ผลลัพธ์:** Broker ตรวจพบว่า TCP Connection ขาดหาย จึงทำการ Publish ข้อความพินัยกรรม `node/esp32/status LOST_CONNECTION_OFFLINE` ไปยัง Subscriber โดยอัตโนมัติ

---

## 4. การบันทึกและวิเคราะห์ผลการทดลอง

### 4.1 บันทึก Verbose Log จากหน้าต่าง Mosquitto Broker
```text
1762391000: mosquitto version 2.0.18 starting
1762391000: Config loaded from mosquitto.conf.
1762391000: Opening ipv4 listen socket on port 1883.
1762391000: mosquitto version 2.0.18 running

# การจับมือการเชื่อมต่อและ Subscribe:
1762391015: New connection from 127.0.0.1:51240 on port 1883.
1762391015: New client connected from 127.0.0.1:51240 as mosq-sub-51240 (p2, c1, k60).
1762391015: Received SUBSCRIBE from mosq-sub-51240 (Topic: home/livingroom/light, QoS 0)
1762391015: Sending SUBACK to mosq-sub-51240

# การรับข้อมูลจาก Publisher และกระจายต่อ:
1762391020: Received PUBLISH from mosq-pub-51242 (d0, q0, r0, m0, 'home/livingroom/light', (2 bytes))
1762391020: Sending PUBLISH to mosq-sub-51240 (d0, q0, r0, m0, 'home/livingroom/light', (2 bytes))
```

---

### 4.2 ตารางเปรียบเทียบลำดับแพ็กเก็ตที่เกิดขึ้นระหว่าง QoS 0, QoS 1 และ QoS 2

| คุณลักษณะเชิงเทคนิค | QoS 0 (At Most Once) | QoS 1 (At Least Once) | QoS 2 (Exactly Once) |
| :--- | :--- | :--- | :--- |
| **ความหมาย** | ส่งครั้งเดียว ไม่การันตีผล | ส่งถึงอย่างน้อยหนึ่งครั้ง | ส่งถึงครั้งเดียวแน่นอน ไม่ขาดไม่เกิน |
| **Packet ID (Message ID)** | ไม่มี (`Mid: 0`) | มีการสร้าง Packet Identifier (`Mid: 1`) | มีการสร้าง Packet Identifier (`Mid: 1`) |
| **จำนวนขั้นตอนจับมือ** | 1 ขั้นตอน (ส่งทางเดียว) | 2 ขั้นตอน (Two-way Handshake) | 4 ขั้นตอน (Four-way Handshake) |
| **ลำดับแพ็กเก็ตควบคุม** | `PUBLISH` | 1. `PUBLISH` (q1)<br/>2. `PUBACK` | 1. `PUBLISH` (q2)<br/>2. `PUBREC` (Publish Received)<br/>3. `PUBREL` (Publish Release)<br/>4. `PUBCOMP` (Publish Complete) |
| **กลไกการส่งซ้ำ (Retransmission)** | ไม่มีการส่งซ้ำ | หากไม่ได้รับ `PUBACK` จะส่งซ้ำพร้อมตั้งบิต `DUP = 1` | หากค้างในขั้นตอนใดจะส่งซ้ำเฉพาะสถานะของขั้นนั้น |
| **ความเสี่ยงข้อมูล** | ข้อความอาจสูญหายได้ | อาจเกิดข้อความซ้ำซ้อน (Duplicate) | ป้องกันข้อความหายและข้อความซ้ำได้ 100% |
| **Overhead & แบนด์วิดท์** | ต่ำที่สุด (Fixed Header 2 ไบต์) | ปานกลาง | สูงสุด (มีแพ็กเก็ตควบคุมแลกเปลี่ยน 4 ตัว) |
| **กรณีการใช้งานที่เหมาะสม** | ข้อมูลเซนเซอร์สตรีมมิ่งถี่ๆ เช่น GPS, อุณหภูมิห้อง | การแจ้งเตือน Alarm, คำสั่งสวิตช์ไฟ | ธุรกรรมการเงิน, ระบบล็อกตู้เซฟ, สั่งยิงหัวจ่ายแก๊ส |

---

### 4.3 ผลการทดสอบ Retained Message เมื่อเปิด Subscriber ขึ้นมาใหม่
```text
PS C:\> mosquitto_sub.exe -h 127.0.0.1 -p 1883 -t "sensor/state" -v
sensor/state CRITICAL_ALERT
```
**วิเคราะห์ผล:**  
เมื่อ Broker ได้รับแพ็กเก็ต `PUBLISH` ที่มีแฟล็ก `RETAIN = 1` Broker จะบันทึก Payload ล่าสุดไว้ในตารางความจำประจำหัวข้อนั้นๆ เมื่อมี Client รายใดมา Subscribe หัวข้อ `sensor/state` ในภายหลัง Broker จะทำการ Push ข้อความนี้ออกไปทันที ทำให้ Subscriber ได้รับสถานะล่าสุดโดยไม่ต้องรอรอบเวลาถัดไปของ Publisher

---

## 5. คำถามท้ายการทดลอง (Post-Lab Questions)

### ข้อ 1: เพราะเหตุใดสัญลักษณ์ `#` จึงต้องวางไว้ที่ตำแหน่งท้ายสุดของ Topic เสมอ? หากกำหนด Topic Filter เป็น `home/#/light` จะเกิดข้อผิดพลาดอย่างไร?

**ตอบ:**  
เพราะว่าตัว `#` ในโปรโตคอล MQTT มันคือ **Multi-level Wildcard** ซึ่งมีความหมายว่า *"ให้ครอบคลุมทุกระดับชั้นย่อยนับตั้งแต่จุดนี้ยาวไปจนถึงปลายสุดของ Topic"* ครับ ในเมื่อมันกวาดเรียบไปจนสุดทางแล้ว ตามหลักการมันจึงต้องอยู่ตัวสุดท้ายเสมอ จะมีอะไรมาเขียนต่อท้ายมันอีกไม่ได้

ถ้าเราไปฝืนพิมพ์เป็น `home/#/light`:
- ตัว Broker จะมองว่าเราเขียนรูปแบบ Topic ผิดไวยากรณ์ (Syntax Error) ทันทีครับ
- ตอนที่ Client ส่งคำสั่ง Subscribe ไป ตัว Broker จะปฏิเสธไม่ยอมรับ โดยในมาตรฐาน MQTT 3.1.1 อาจจะตัดสายทิ้ง หรือตอบ `SUBACK` ที่เป็นรหัสล้มเหลว (`0x80`) และใน MQTT 5.0 ก็จะแจ้งเตือนชัดเจนเลยว่า `Topic Filter Invalid (0x8F)` ทำให้เราจะไม่ได้รับข้อความอะไรเลยจากหัวข้อนี้ครับ

---

### ข้อ 2: ในระบบงานติดตามยานพาหนะ (GPS Tracking) ที่ส่งพิกัดรถยนต์ทุกๆ 1 วินาที ท่านคิดว่าควรเลือกระดับ QoS ใดระหว่าง QoS 0, 1 หรือ 2 เพราะเหตุใด?

**ตอบ:**  
ควรเลือกใช้ **QoS 0 (At Most Once)** ครับ ด้วยเหตุผลตามการใช้งานจริงดังนี้:
1. **ข้อมูลส่งถี่มากอยู่แล้ว:** ตัวกล่อง GPS ส่งพิกัดใหม่ทุกๆ 1 วินาที ถ้าเกิดรถวิ่งลงอุโมงค์หรือสัญญาณเน็ต 4G/5G กระตุกไปแค่จังหวะนั้น แล้วพิกัดของวินาทีนั้นหายไป ก็ไม่เป็นปัญหาเลยครับ เพราะอีกแค่ 1 วินาทีถัดไป พิกัดใหม่ล่าสุดก็จะถูกส่งขึ้นมาแทนที่ทันที ไม่จำเป็นต้องย้อนส่งของเก่า
2. **ป้องกันปัญหารถติดคอขวด (Queue Jamming):** ถ้าเราไปใช้ QoS 1 ตอนที่เน็ตหลุด ตัวอุปกรณ์จะไม่ได้รับ `PUBACK` มันก็จะพยายามส่งซ้ำของเก่าวนไปเรื่อยๆ พอรถโผล่ออกจากอุโมงค์ ข้อมูลเก่าที่ค้างอยู่จะทะลักเข้าเซิร์ฟเวอร์พร้อมกันหมด ทำให้พิกัดบนหน้าจอแผนที่กระตุกหรือวิ่งย้อนไปย้อนมา
3. **ประหยัดเน็ตซิมและแบตเตอรี่:** QoS 0 มีขนาดหัวแพ็กเก็ตเล็กสุด (แค่ 2 ไบต์) และไม่ต้องส่งตอบรับสองทาง ช่วยประหยัดค่าเน็ตซิมการ์ด IoT และกินไฟน้อยที่สุดครับ

---

### ข้อ 3: หาก Client ส่งแพ็กเก็ต `DISCONNECT` ไปยัง Broker อย่างถูกต้องก่อนปิดโปรแกรม ข้อความ Last Will and Testament (LWT) จะถูกเผยแพร่ออกไปหรือไม่ เพราะเหตุใด?

**ตอบ:**  
**ไม่ถูกส่งออกไปแน่นอนครับ**  
เพราะว่าฟังก์ชัน LWT (พินัยกรรมฉุกเฉิน) เขาตั้งใจออกแบบมาไว้ใช้ตรวจจับ **"เหตุการณ์ผิดปกติที่เครื่องดับไปโดยไม่ได้ตั้งใจ"** เท่านั้น เช่น อยู่ดีๆ บอร์ดไฟดับ, ชิปค้าง, หรือสายแลนหลุดกะทันหัน

แต่ถ้าโปรแกรมของเราปิดตัวลงอย่างถูกต้อง ตัว Client จะส่งแพ็กเก็ต `DISCONNECT` ไปบอก Broker ล่วงหน้าก่อน พอด้าน Broker ได้รับแพ็กเก็ตนี้ ก็จะเข้าใจทันทีว่า *"อ๋อ ฝั่งนี้เขาขอปิดการเชื่อมต่อตามปกตินะ"* ตัว Broker ก็จะทำการ **ลบข้อความ LWT ที่ฝากไว้ออกทิ้งทันที** แล้วค่อยปิดการเชื่อมต่อ ข้อความ LWT จึงไม่มีทางถูกส่งออกไปหาใครครับ

---

### ข้อ 4: Retained Message มีประโยชน์อย่างไรในการออกแบบระบบแดชบอร์ด Smart Home สำหรับผู้ใช้งานที่เพิ่งเปิดแอปพลิเคชันขึ้นมาเป็นครั้งแรก?

**ตอบ:**  
มีประโยชน์มากตรงที่ทำให้ **แอปพลิเคชันแสดงสถานะจริงของบ้านได้ทันทีที่เปิดแอปขึ้นมา** ครับ  
เพราะถ้าเป็น MQTT ปกติทั่วไป ถ้าไม่มีใครกดสวิตช์ไฟหรือเซนเซอร์ไม่ได้ส่งข้อมูลเข้ามาตอนนั้น Broker ก็จะไม่มีข้อมูลอะไรส่งให้แอปเลย พอเราเพิ่งเปิดแอปบนมือถือขึ้นมา หน้าจอก็จะขึ้นเป็นค่าว่างๆ หรือหมุนติ้วๆ รอโหลด ไม่รู้ว่าตอนนี้ไฟดวงไหนเปิดหรือปิดอยู่

แต่ถ้าเราตั้งค่า Retain ไว้ ตัว Broker จะคอยจำสถานะล่าสุดของแต่ละสวิตช์ไว้ในระบบเสมอ ทันทีที่เราเปิดแอปมือถือและกดเชื่อมต่อเข้า Broker ตัว Broker จะเด้งส่งข้อความล่าสุด (เช่น `home/light/state = ON`) ให้แอปนำไปแสดงผลบนสวิตช์ไฟได้ทันที โดยไม่ต้องรอให้มีใครไปกดเปิดปิดไฟรอบใหม่
