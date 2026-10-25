# 06-Labsheet-11-1-Mosquitto-Broker-and-MQTT-Fundamentals

---

## 3. ขั้นตอนการทดลอง

### กิจกรรมที่ 11-1.1: การติดตั้งและเริ่มต้น Mosquitto Broker

#### 4. เริ่มต้นการทำงานของ Mosquitto Broker

<img width="972" height="611" alt="ภาพถ่ายหน้าจอ 2569-10-25 เวลา 23 41 18" src="https://github.com/user-attachments/assets/398ce026-908e-40d6-8046-68acf438b6ff" />

#### 5. หยุดการทำงานของ Mosquitto Broker

<img width="972" height="611" alt="ภาพถ่ายหน้าจอ 2569-10-25 เวลา 23 41 32" src="https://github.com/user-attachments/assets/dab7657e-eb56-48ec-b8de-b346a6b797df" />

---

### กิจกรรมที่ 11-1.2  การทดสอบ Publish / Subscribe พื้นฐาน และการใช้ Wildcards

#### 1. การ Subscribe หัวข้อเฉพาะเจาะจง

<img width="1512" height="982" alt="ภาพถ่ายหน้าจอ 2569-10-25 เวลา 23 47 09" src="https://github.com/user-attachments/assets/68fe3e6f-58f4-4379-8226-ff62f5242d34" />

#### 2. การทดสอบไวลด์การ์ดระดับเดียว (Single-Level Wildcard: `+`)

<img width="1512" height="982" alt="ภาพถ่ายหน้าจอ 2569-10-25 เวลา 23 49 00" src="https://github.com/user-attachments/assets/5f71c09f-10fe-4d3d-a094-7ab9b19f0435" />

```
ข้อความที่ปรากฏในเทอร์มินัลที่ 2 (Subscriber)

  home/livingroom/light ON
  home/kitchen/light OFF

ข้อความที่ไม่ปรากฏ
  home/bedroom/balcony/light ON

เหตุผล
สัญลักษณ์ + แทนได้เพียง 1 ระดับของ Topic เท่านั้น ดังนั้น home/+/light จึงตรงกับ home/livingroom/light และ home/kitchen/light (ระดับกลางมี 1 ระดับ) แต่ home/bedroom/balcony/light มีระดับกลาง 2 ระดับ(bedroom/balcony) จึงไม่ตรงกับรูปแบบ Broker จึงไม่ส่งข้อความนี้ให้ Subscriber
```

#### 3. การทดสอบไวลด์การ์ดหลายระดับ (Multi-Level Wildcard: `#`)

<img width="1512" height="982" alt="ภาพถ่ายหน้าจอ 2569-10-25 เวลา 23 52 46" src="https://github.com/user-attachments/assets/84312163-c024-4084-93e9-42a596d4cb93" />

---

### กิจกรรมที่ 11-1.3 การทดสอบระดับคุณภาพการส่งข้อมูล (QoS 0, QoS 1, QoS 2)

#### 1. ทดสอบ QoS 0 (At Most Once)

<img width="1512" height="982" alt="1" src="https://github.com/user-attachments/assets/f56029d7-1bd4-470e-ac46-61d9df8ba92b" />

#### 2. ทดสอบ QoS 1 (At Least Once)

<img width="1512" height="982" alt="2" src="https://github.com/user-attachments/assets/dbce67cb-d842-462e-80a2-6875f9673248" />

#### 3. ทดสอบ QoS 2 (Exactly Once)

<img width="1512" height="982" alt="3" src="https://github.com/user-attachments/assets/66e53858-e668-4d17-b5cb-932a4a6276bf" />

---

### กิจกรรมที่ 11-1.4 การทดสอบข้อความคงค้าง (Retained Messages)

#### 1. ทดสอบข้อความคงค้าง (Retained Messages)

<img width="1512" height="982" alt="3" src="https://github.com/user-attachments/assets/f3f1eb78-9424-4c45-a19e-c2ece97dcfb6" />

<img width="1512" height="982" alt="4" src="https://github.com/user-attachments/assets/ee0b3bdf-ab10-461b-84e8-180858822631" />

<img width="1512" height="982" alt="5" src="https://github.com/user-attachments/assets/ee3028e7-165d-4cf5-af7b-b12e5a76f163" />

---

### กิจกรรมที่ 11-1.5 การทดสอบพินัยกรรมฉุกเฉิน (Last Will and Testament - LWT)

<img width="1512" height="982" alt="ภาพถ่ายหน้าจอ 2569-10-26 เวลา 00 06 12" src="https://github.com/user-attachments/assets/929aa3bf-2c67-42cd-94cf-76749ef4bd86" />

---

## 4. การบันทึกและวิเคราะห์ผลการทดลอง

1. ภาพหน้าจอการทำงานของ Mosquitto Broker Verbose Log ขณะที่มีการเชื่อมต่อและ Subscribe ข้อความ

<img width="1512" height="982" alt="broker log subscribe" src="https://github.com/user-attachments/assets/dbce67cb-d842-462e-80a2-6875f9673248" />

2. ตารางเปรียบเทียบลำดับแพ็กเก็ตที่เกิดขึ้นระหว่าง QoS 0, QoS 1 และ QoS 2 ที่สังเกตได้จาก Broker Log

| QoS | แพ็กเก็ตที่เห็นใน Broker Log |
|-----|------------------------------|
| 0 | PUBLISH รับเข้า และ PUBLISH ส่งต่อ ไม่มีแพ็กเก็ตตอบรับ |
| 1 | PUBLISH, PUBACK ไปยัง publisher, PUBACK Mid 1 จาก subscriber |
| 2 | PUBLISH, PUBREC, PUBREL, PUBCOMP ทั้งฝั่ง publisher และ subscriber |

3. ภาพการทดสอบ Retained Message เมื่อเปิด Subscriber ขึ้นมาใหม่

<img width="1512" height="982" alt="retained" src="https://github.com/user-attachments/assets/ee3028e7-165d-4cf5-af7b-b12e5a76f163" />

---

## 5. คำถามท้ายการทดลอง (Post-Lab Questions)
1. เพราะเหตุใดสัญลักษณ์ `#` จึงต้องวางไว้ที่ตำแหน่งท้ายสุดของ Topic เสมอ? หากกำหนด Topic Filter เป็น `home/#/light` จะเกิดข้อผิดพลาดอย่างไร?

```
แทนทุกระดับที่เหลือต่อจากตำแหน่งนั้นทั้งหมด เลยต้องอยู่ท้ายสุดเท่านั้น ถ้าวางกลางทางอย่าง home/#/light จะไม่รู้ว่าครอบคลุมถึงไหน เป็น topic filter ที่ผิดกติกา broker หรือ client จะปฏิเสธ ทำให้ subscribe ไม่สำเร็จ
```

2. ในระบบงานติดตามยานพาหนะ (GPS Tracking) ที่ส่งพิกัดรถยนต์ทุกๆ 1 วินาที ท่านคิดว่าควรเลือกระดับ QoS ใดระหว่าง QoS 0, 1 หรือ 2 เพราะเหตุใด?

```
เลือก QoS 0 เพราะพิกัดส่งทุก 1 วินาที ตกไปหนึ่งค่าก็ไม่เป็นไร อีกวินาทีเดียวก็มีค่าใหม่มาแทน ไม่ต้องเสีย bandwidth และพลังงานกับ ack ที่ไม่จำเป็น
```

3. หาก Client ส่งแพ็กเก็ต `DISCONNECT` ไปยัง Broker อย่างถูกต้องก่อนปิดโปรแกรม ข้อความ Last Will and Testament (LWT) จะถูกเผยแพร่ออกไปหรือไม่ เพราะเหตุใด?

```
ไม่ถูกเผยแพร่ เพราะ LWT มีไว้สำหรับกรณีหลุดผิดปกติเท่านั้น ถ้า client ส่ง DISCONNECT มาก่อน broker ถือว่าปิดแบบตั้งใจ และจะทิ้ง will message ไป
```

4. Retained Message มีประโยชน์อย่างไรในการออกแบบระบบแดชบอร์ด Smart Home สำหรับผู้ใช้งานที่เพิ่งเปิดแอปพลิเคชันขึ้นมาเป็นครั้งแรก?

```
ผู้ใช้ที่เพิ่งเปิดแอปครั้งแรกจะเห็นสถานะล่าสุดของอุปกรณ์ทันที เช่น ไฟเปิดอยู่ไหมหรืออุณหภูมิล่าสุดเท่าไร ไม่ต้องรอให้อุปกรณ์ส่งค่าใหม่ ซึ่งบางตัวอาจส่งนานๆ ครั้ง หน้าแดชบอร์ดเลยไม่ว่างเปล่า
```

---

# 07-Labsheet-11-2-ESP32-MQTT-Client-Telemetry-and-Control

---

## 4. การทดสอบและการตรวจสอบผลลัพธ์

### 1. การตรวจสอบ Telemetry Data

<img width="1512" height="982" alt="ภาพถ่ายหน้าจอ 2569-10-26 เวลา 00 33 40" src="https://github.com/user-attachments/assets/cc86c476-4e71-4227-9d99-0da52d243f4c" />

### 2. การส่งคำสั่งเปิด-ปิดหลอดไฟ LED

#### คำสั่งเปิดไฟ

<img width="1512" height="949" alt="1" src="https://github.com/user-attachments/assets/0e884387-b271-406b-8bc4-43f4a050a2bb" />

<img width="543" height="633" alt="image" src="https://github.com/user-attachments/assets/135f71b8-632c-494c-9e40-4eb079408749" />

#### คำสั่งปิดไฟ

<img width="1512" height="982" alt="2" src="https://github.com/user-attachments/assets/cdf21d28-154d-44c8-b6d5-6c998f8b5ca3" />

<img width="529" height="652" alt="image" src="https://github.com/user-attachments/assets/79ca8f32-5e20-48c1-ba14-ba426ee94b64" />

---

## 5. คำถามท้ายการทดลอง (Post-Lab Questions)

1. เพราะเหตุใดเราจึงต้องส่งข้อความสถานะ `iot/node01/status = "online"` ด้วยแฟล็ก `retain = 1` ควบคู่กับการตั้งค่า Last Will and Testament (LWT) ว่า `"offline"`

```
ข้อความทั้งสองส่งไปที่หัวข้อเดียวกันและ retain ไว้ทั้งคู่ broker จึงเก็บสถานะล่าสุดของบอร์ดเสมอ ตอนต่อสำเร็จบอร์ดส่ง online ทับ ตอนหลุดผิดปกติ broker ส่ง offline ทับให้เอง ใครเพิ่งเปิดแอปหรือเพิ่ง subscribe ก็เห็นสถานะจริงทันทีโดยไม่ต้องรอข้อความใหม่ ถ้าไม่ retain คนที่มาทีหลังจะไม่รู้ว่าบอร์ดออนไลน์อยู่ไหม และถ้า offline ค้างอยู่โดย online ไม่ retain แอปจะเห็น offline ทั้งที่บอร์ดกลับมาแล้ว
```

2. หากเครือข่าย Wi-Fi หลุดชั่วคราว คอมโพเนนต์ `esp-mqtt` มีพฤติกรรมการจัดการอย่างไร และเมื่อสัญญาณกลับมา โค้ดจำเป็นต้องเรียกฟังก์ชันเชื่อมต่อใหม่ด้วยตนเองหรือไม่

```
esp-mqtt เชื่อมต่อใหม่ให้เองอัตโนมัติ เมื่อหลุดจะเกิดอีเวนต์ MQTT_EVENT_DISCONNECTED แล้วลองต่อใหม่เรื่อยๆ ตามค่า reconnect timeout โค้ดไม่ต้องเรียกเชื่อมต่อซ้ำเอง พอต่อได้จะเข้า MQTT_EVENT_CONNECTED อีกรอบ โค้ดจึงส่ง online และ subscribe หัวข้อ cmd/led ใหม่ในอีเวนต์นั้น
```

3. ในฟังก์ชัน `telemetry_task` เพราะเหตุใดหลังจากเรียก `cJSON_PrintUnformatted(root)` แล้วเราจึงจำเป็นต้องเรียกฟังก์ชัน `free(json_str)` และ `cJSON_Delete(root)` เสมอ? หากละเลยจะมีผลเสียอย่างไรต่อหน่วยความจำของ ESP32

```
cJSON_CreateObject และ cJSON_PrintUnformatted จองหน่วยความจำจาก heap และไม่คืนให้อัตโนมัติ root คือก้อน object ส่วน json_str คือสตริงที่พิมพ์ออกมา ต้องคืนทั้งสองก้อน ถ้าลืม heap จะรั่วทุก 4 วินาที ค่อยๆ หมดไป สุดท้าย malloc ล้มเหลว publish ไม่ได้ หรือบอร์ดค้างและรีบูต เพราะ ESP32 มี RAM น้อยและต้องรันนานต่อเนื่อง
```

---
---

# 08-Labsheet-11-3-MQTTS-TLS-Mutual-Authentication-Security

---

## 3. ขั้นตอนการทดลอง

### กิจกรรมที่ 11-3.3 การทดสอบความปลอดภัยผ่านเครื่องมือ CLI

#### 1. การทดสอบแบบถูกต้อง (ส่งทั้ง ca.crt, client.crt, client.key)

<img width="1512" height="982" alt="ภาพถ่ายหน้าจอ 2569-10-26 เวลา 01 16 12" src="https://github.com/user-attachments/assets/3392aed3-3dbd-43c2-a179-7c845f2f82e5" />

#### 2. การทดสอบการปฏิเสธผู้บุกรุก (Unauthorized Client Attack)

<img width="1512" height="982" alt="ภาพถ่ายหน้าจอ 2569-10-26 เวลา 01 17 34" src="https://github.com/user-attachments/assets/73ef732b-83d3-46e8-86ed-fb8b4810ea6b" />

---

## 4. การทดสอบและการตรวจสอบผลลัพธ์ (Testing & Verification)

#### 2. ผลใน Serial Monitor

<img width="1512" height="982" alt="ภาพถ่ายหน้าจอ 2569-10-26 เวลา 01 22 15" src="https://github.com/user-attachments/assets/20d1fb81-da7c-4f07-a4e4-ea8df82ba03b" />

#### 3. ผลใน Broker Log

<img width="1512" height="982" alt="ภาพถ่ายหน้าจอ 2569-10-26 เวลา 01 22 21" src="https://github.com/user-attachments/assets/a35a2889-e319-4a8e-9364-925167171364" />

---

## 5. คำถามท้ายการทดลอง (Post-Lab Questions)
1. เพราะเหตุใดในคำสั่งสร้าง Server CSR ค่า **Common Name (CN)** จึงต้องระบุเป็น IP Address หรือ Hostname ของ Broker ให้ตรงกันพอดี? หากระบุเป็นชื่ออื่นจะเกิดข้อผิดพลาดใดขึ้น?

```
เพราะ client ต้องแน่ใจว่าเครื่องที่ต่ออยู่คือ broker ตัวจริง ไม่ใช่เครื่องอื่นที่เอาใบรับรองมาปลอมตัว หลังตรวจว่า CA เซ็นถูกแล้ว client จะเทียบชื่อในใบรับรองกับที่อยู่ที่ตัวเองต่อไป ถ้าไม่ตรงกันจะถือว่า hostname mismatch แล้วตัดการเชื่อมต่อตอน TLS handshake ทันที ทำให้ต่อ broker ไม่ได้ ในการทดลองนี้ใส่แค่ CN เป็น IP แล้ว mosquitto_sub ยังถูกปฏิเสธ (bad certificate) จนต้องเพิ่ม subjectAltName เป็น IP เพราะ OpenSSL 3 ตรวจชื่อจาก SAN เป็นหลัก
```

2. การยืนยันตัวตนแบบ **Mutual Authentication (mTLS)** แตกต่างจากการใช้ **HTTPS** ทั่วไปบนเว็บเบราว์เซอร์อย่างไรในประเด็นเรื่องใบรับรองฝั่ง Client?

```
HTTPS ทั่วไปมีแค่เซิร์ฟเวอร์ที่ต้องมีใบรับรอง เบราว์เซอร์ตรวจเซิร์ฟเวอร์ฝั่งเดียว ส่วนผู้ใช้ยืนยันตัวตนทีหลังด้วยรหัสผ่านหรือ token ส่วน mTLS ฝั่ง client ก็ต้องมีใบรับรองและกุญแจส่วนตัวของตัวเองด้วย และ broker ตรวจว่าใบรับรองนั้นออกโดย CA ที่เชื่อถือ ถ้าไม่มี broker ตัดการเชื่อมต่อตั้งแต่ handshake ก่อนที่จะส่งข้อความ MQTT ได้ ตอนทดสอบ mosquitto_pub ที่ส่งแค่ cafile จึงถูกตัดทันที ทำให้ยืนยันตัวตนกันได้ทั้งสองทางโดยไม่ต้องใช้รหัสผ่าน
```

3. หากมีผู้ไม่หวังดีขโมยไฟล์เฟิร์มแวร์ไบนารี (`.bin`) ของบอร์ด ESP32 ไปได้ เขาจะสามารถกู้คืนไฟล์ `client.key` ได้หรือไม่ และในระบบ IoT เชิงพาณิชย์เรามีฮาร์ดแวร์ความปลอดภัยใด (เช่น ESP32 Secure Boot / Flash Encryption / Digital Signature Peripheral) มาช่วยแก้ไขปัญหานี้?

```
กู้ได้ เพราะ client.key ถูกฝังเป็นข้อความธรรมดา (PEM) ในเฟิร์มแวร์ตามที่ใช้ EMBED_TXTFILES ถ้าได้ไฟล์ .bin หรืออ่านจากแฟลชตรงๆ ก็ค้นหาข้อความ BEGIN PRIVATE KEY เจอแล้วนำกุญแจไปปลอมตัวเป็นบอร์ดได้ ในงานเชิงพาณิชย์แก้ด้วย Flash Encryption ที่เข้ารหัสเนื้อในแฟลชด้วยกุญแจที่เก็บใน eFuse อ่านออกไม่ได้ Secure Boot ที่ให้บอร์ดรันเฉพาะเฟิร์มแวร์ที่เซ็นลายเซ็นถูกต้อง และ Digital Signature Peripheral (DS) ที่ให้ฮาร์ดแวร์ใช้กุญแจส่วนตัวเซ็นในชิปเอง กุญแจจึงไม่เคยออกมานอกชิปและไม่ต้องฝังในเฟิร์มแวร์ นอกจากนี้ยังออกกุญแจเฉพาะบอร์ดแต่ละตัว เพื่อให้เพิกถอนทีละตัวได้
```

---
---

# 09-Labsheet-11-4-ESP-RainMaker-Cloud-IoT-and-Smart-Device

---

## 4. การทดสอบและการตรวจสอบผลลัพธ์ (Testing & Verification)

### 1. การควบคุมเปิด-ปิดผ่าน Mobile App

<img width="438" height="740" alt="image" src="https://github.com/user-attachments/assets/0c11260e-31e5-4c6b-9d6c-fd0b7c018a3b" />
<img width="454" height="559" alt="image" src="https://github.com/user-attachments/assets/aaec7b9f-9781-4e68-a997-ee1aca0ae0cb" />
<img width="1512" height="982" alt="ภาพถ่ายหน้าจอ 2569-10-26 เวลา 01 53 47" src="https://github.com/user-attachments/assets/ded6001e-2e5a-4710-9859-f01ce684894e" />

### 2. การควบคุมระยะไกลข้ามอินเทอร์เน็ต (Remote 4G/5G Test)

```
ใช้งาน hotspot ในการทดลองจริงไม่ได้เปิด WIFI ตั้งเเต่เเรก
```

---

## 5. คำถามท้ายการทดลอง (Post-Lab Questions)
1. ในสถาปัตยกรรม ESP RainMaker กระบวนการ **Assisted Claiming** ช่วยแก้ปัญหาเรื่องการติดตั้งใบรับรองความปลอดภัย (Security Certificates) บนอุปกรณ์ในขั้นตอนพัฒนาต้นแบบได้อย่างไร?

```
ปกติอุปกรณ์ต้องมีใบรับรอง X.509 และกุญแจส่วนตัวเฉพาะตัวถึงจะต่อ AWS IoT Core ได้ ซึ่งต้องออกใบรับรองและเขียนลงบอร์ดทีละตัว ยุ่งยากมากตอนทำต้นแบบ Assisted Claiming ให้แอปบนมือถือทำแทน แอปจะคุยกับบอร์ดผ่าน BLE ดึงข้อมูลของบอร์ด ไปขอใบรับรองจาก RainMaker Cloud แล้วส่งกลับเขียนลงบอร์ดผ่าน BLE นักพัฒนาไม่ต้องสร้างหรือฝังใบรับรองเอง แค่แฟลชเฟิร์มแวร์เดียวกันแล้วสแกน QR ก็ได้ใบรับรองเฉพาะบอร์ดและผูกกับบัญชีผู้ใช้
```

2. เพราะเหตุใดฟังก์ชัน `write_cb` จึงต้องเรียกคำสั่ง `esp_rmaker_param_update_and_report()` ทุกครั้งที่มีการเปลี่ยนแปลงค่าพารามิเตอร์?

```
เพราะ callback แค่สั่งฮาร์ดแวร์ แต่ค่าพารามิเตอร์ที่เก็บในบอร์ดและบนคลาวด์ยังเป็นค่าเก่า ฟังก์ชันนี้อัปเดตค่าในบอร์ดและรายงานสถานะใหม่ขึ้นคลาวด์ แอปมือถือ ผู้ใช้คนอื่น Schedule และระบบสั่งงานด้วยเสียงจึงเห็นสถานะตรงกับของจริง ถ้าไม่เรียก ไฟอาจติดแล้วแต่สวิตช์ในแอปยังโชว์ว่าปิด หรือค่าที่ส่งไปไม่ตรงกับสิ่งที่บอร์ดทำจริง
```

3. เปรียบเทียบความแตกต่างระหว่างการพัฒนาอุปกรณ์ IoT ด้วย **Plain MQTT บน Mosquitto เอง (ในแล็บ 11.2)** กับการใช้ **ESP RainMaker SDK (ในแล็บ 11.4)** ในแง่ของความพยายามในการพัฒนา (Development Effort), ความปลอดภัย (Security), และการขยายระบบ (Scalability)

```
ความพยายามในการพัฒนา: Plain MQTT ต้องทำเองทุกอย่าง ตั้ง broker ออกแบบ topic และ JSON เขียน callback ตั้ง Wi-Fi เอง และไม่มีแอปมือถือ ส่วน RainMaker มี Node/Device/Param ให้ มีแอปมือถือและ BLE provisioning ในตัว เขียนโค้ดน้อยกว่ามาก
ความปลอดภัย: Lab 11.2 เป็น plaintext พอร์ต 1883 ไม่มีการยืนยันตัวตน จะปลอดภัยต้องทำ TLS และ mTLS เอง Lab 11.3 RainMaker ใช้ TLS พร้อมใบรับรองเฉพาะรายอุปกรณ์ตั้งแต่ต้น และมีระบบผูกบัญชีผู้ใช้
การขยายระบบ: broker เดียวบนเครื่องตัวเองรับอุปกรณ์ได้จำกัดและต้องดูแลเอง ต้องจัดการผู้ใช้ สิทธิ์ และ OTA เอง RainMaker รันบน AWS IoT Core รองรับอุปกรณ์จำนวนมาก มี OTA, Schedule และการแชร์อุปกรณ์ให้ผู้ใช้หลายคนเป็นบริการพร้อมใช้ แต่ต้องพึ่งแพลตฟอร์มของ Espressif
```

4. อธิบายขั้นตอนที่ทำให้ ESP RainMaker สามารถเชื่อมโยงการทำงานร่วมกับระบบสั่งงานด้วยเสียงของ **Google Home** หรือ **Amazon Alexa** ได้โดยไม่ต้องเขียนโค้ดเพิ่มเติมบนตัวบอร์ด ESP32

```
บอร์ดกำหนดตัวเองเป็น Device มาตรฐาน แล้วรายงานสถานะขึ้น RainMaker Cloud อยู่แล้ว ผู้ใช้เปิดแอป Alexa หรือ Google Home เพิ่มสกิล/บริการ ESP RainMaker แล้วล็อกอินบัญชีเดียวกัน ระบบเสียงจะซิงก์รายการอุปกรณ์จากคลาวด์เข้ามาเอง พอสั่งเสียง ระบบเสียงส่งคำสั่งไปที่คลาวด์ แล้วคลาวด์ส่งลงบอร์ดผ่าน MQTT เป็นการเปลี่ยนค่าพารามิเตอร์ ซึ่งไปเข้า write_cb เดิมที่เขียนไว้ ไม่มีโค้ดใหม่ฝั่งบอร์ด ทั้งหมดเพราะใช้ชนิดอุปกรณ์และพารามิเตอร์มาตรฐานที่คลาวด์แปลงเป็นอุปกรณ์ของ Alexa/Google ได้
```
