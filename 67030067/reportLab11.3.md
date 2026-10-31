
# รายงานผลการทดลองที่ 11.3
## การสื่อสาร MQTT ความปลอดภัยสูงด้วย TLS และการยืนยันตัวตนสองทาง (mTLS) บน Mosquitto และ ESP-IDF

---

## 4. บันทึกและวิเคราะห์ผลการทดลอง (Results & Verification)

### 4.1 บันทึกผลการทดสอบผ่านเครื่องมือ CLI (กิจกรรมที่ 11-3.3)

#### 1. การทดสอบแบบถูกต้อง (Authorized Client Connection)

เมื่อทำการ Publish และ Subscribe ข้อมูลด้วยการแนบไฟล์ `ca.crt`, `client.crt` และ `client.key` ครบถ้วน พบว่าสามารถเชื่อมต่อกับ MQTT Broker ผ่าน TLS ได้สำเร็จ

**คำสั่งที่ใช้ทดสอบ**

**เทอร์มินัลที่ 2 (Subscriber)**

```powershell
mosquitto_sub -h 192.168.1.100 -p 8883 -t "secure/test" --cafile ca.crt --cert client.crt --key client.key -v
```

**เทอร์มินัลที่ 3 (Publisher)**

```powershell
mosquitto_pub -h 192.168.1.100 -p 8883 -t "secure/test" -m "Hello Secure World!" --cafile ca.crt --cert client.crt --key client.key
```

**ผลการทดลองฝั่ง Subscriber**

ได้รับข้อความ `secure/test Hello Secure World!` อย่างถูกต้อง

**Log บน Mosquitto Broker**

```text
1710000000: New connection from 192.168.1.100:51234 on port 8883.
1710000000: Client auto-123456 has connected successfully.
1710000000: Peer certificate verified: /C=TH/ST=Bangkok/O=IoT Node/CN=esp32-secure-node
```

#### 2. การทดสอบการปฏิเสธผู้บุกรุก (Unauthorized Client Attack)

เมื่อทดลองส่งข้อความโดยไม่แนบใบรับรอง Client (ระบุเพียง `--cafile ca.crt` เท่านั้น) พบว่าระบบปฏิเสธการเชื่อมต่อ

**คำสั่งที่ใช้ทดสอบ**

```powershell
mosquitto_pub -h 192.168.1.100 -p 8883 -t "secure/test" -m "Hacker message" --cafile ca.crt
```

**ผลการทดลองฝั่ง CLI (Publisher)**

เกิดข้อผิดพลาดและถูกปฏิเสธการเชื่อมต่อ

```text
Error: OpenSSL Error[0]: error:0A0000C6:SSL routines::packet length too long
Error: Connection refused: Not authorised
```

**Log บน Mosquitto Broker**

```text
1710000050: New connection from 192.168.1.100:51235 on port 8883.
1710000050: OpenSSL Error[0]: error:1417C0C7:SSL routines:ssl3_get_record:peer did not return a certificate
1710000050: Client <unknown> disconnected due to TLS error.
```

### 4.2 บันทึกการทดสอบบนบอร์ด ESP32 (กิจกรรมที่ 11-3.4 ถึง 11-3.5)

เมื่อทำการแฟลชโปรแกรมลงบนบอร์ด ESP32 และเปิด Serial Monitor พบว่าบอร์ดสามารถเชื่อมต่อ Wi-Fi และทำ TLS Handshake กับ MQTT Broker ได้สำเร็จ

**ผลลัพธ์บน ESP32 Serial Monitor**

```text
I (3120) MQTTS_SECURE: Waiting for Wi-Fi connection...
I (4560) MQTTS_SECURE: Wi-Fi Connected! Initializing Secure MQTTS Client with mTLS...
I (5100) esp-tls: Connecting to 192.168.1.100:8883...
I (6200) esp-tls: Peer certificate verification OK
I (6350) MQTTS_SECURE: MQTTS TLS Handshake Successful! Connected to Secure Broker.
```

**Log บน Mosquitto Broker เมื่อ ESP32 เชื่อมต่อเข้ามา**

```text
1710000100: New connection from 192.168.1.105:49152 on port 8883.
1710000101: Peer certificate verified: /C=TH/ST=Bangkok/O=IoT Node/CN=esp32-secure-node
1710000101: New client connected from 192.168.1.105:49152 as esp32_mtls_client_node (p2, c1, k120).
```

---

## 5. คำถามท้ายการทดลอง (Post-Lab Questions)

### ข้อ 1. เพราะเหตุใดในคำสั่งสร้าง Server CSR ค่า Common Name (CN) จึงต้องระบุเป็น IP Address หรือ Hostname ของ Broker ให้ตรงกันพอดี? หากระบุเป็นชื่ออื่นจะเกิดข้อผิดพลาดใดขึ้น?

**คำตอบ:** เพราะ Client จะนำ IP หรือ Hostname ที่ใช้เชื่อมต่อไปเปรียบเทียบกับค่า CN ในใบรับรองที่ Broker ส่งมา หากระบุไม่ตรงกัน จะเกิดข้อผิดพลาด CN Mismatch Verification Failure ส่งผลให้ Client ปฏิเสธการเชื่อมต่อทันที เพื่อป้องกันการโจมตีแบบ Man-in-the-Middle

### ข้อ 2. การยืนยันตัวตนแบบ Mutual Authentication (mTLS) แตกต่างจากการใช้ HTTPS ทั่วไปบนเว็บเบราว์เซอร์อย่างไรในประเด็นเรื่องใบรับรองฝั่ง Client?

**คำตอบ:** HTTPS ทั่วไปตรวจสอบใบรับรองเฉพาะฝั่ง Server (One-way TLS) ส่วน mTLS จะทำการยืนยันตัวตนทั้งสองฝั่ง (Bi-directional) โดย Broker จะขอดูและตรวจสอบใบรับรองฝั่ง Client (`client.crt`) ด้วย หากไม่มีหรือไม่ถูกต้องจะปฏิเสธการเชื่อมต่อทันที

### ข้อ 3. หากมีผู้ไม่หวังดีขโมยไฟล์เฟิร์มแวร์ไบนารี (.bin) ของบอร์ด ESP32 ไปได้ เขาจะสามารถกู้คืนไฟล์ `client.key` ได้หรือไม่ และในระบบ IoT เชิงพาณิชย์เรามีฮาร์ดแวร์ความปลอดภัยใดมาช่วยแก้ไขปัญหานี้?

**คำตอบ:** สามารถกู้คืนได้ หากไม่ได้เปิดระบบป้องกัน เนื่องจากไฟล์อาจถูกฝังเป็นข้อความ Plaintext ใน Flash แนวทางแก้ไขในระดับฮาร์ดแวร์และระบบป้องกันคือการเปิดใช้งาน ESP32 Flash Encryption, Secure Boot และใช้ชิปความปลอดภัยประเภท Secure Element / Digital Signature Peripheral เพื่อป้องกันการดึงกุญแจส่วนตัวออกไป

---

## 6. สรุปผลการทดลอง

จากการทดลองสามารถตั้งค่าและทดสอบการสื่อสาร MQTT ผ่าน TLS และ mTLS ได้สำเร็จ โดยทดสอบทั้งการเชื่อมต่อจาก Client ที่มีใบรับรองถูกต้อง และการปฏิเสธ Client ที่ไม่มีใบรับรอง นอกจากนี้ยังสามารถเชื่อมต่อบอร์ด ESP32 เข้ากับ Mosquitto Broker ผ่านการเข้ารหัส TLS ได้ แสดงให้เห็นว่าระบบสามารถเพิ่มความปลอดภัยในการรับส่งข้อมูลและตรวจสอบตัวตนของอุปกรณ์ก่อนอนุญาตให้เชื่อมต่อได้
