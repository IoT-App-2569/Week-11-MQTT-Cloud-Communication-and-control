# Answer Lab 11.3 – MQTTS: TLS และ Mutual Authentication

- ชื่อ-สกุล: ธีรณัฏฐ์ ภูติวณิชย์
- รหัสนักศึกษา: 67030098
- วันที่ทำการทดลอง: 13/10/2026
- โค้ด: [Code/Lab11-3](../../Code/Lab11-3-MQTTS-TLS-Mutual-Authentication-Security)
- รูปภาพ: [Answer/Image/Lab11-3](../Image/Lab11-3)

---

## 1. ผลการทดลอง

### 11-3.1 ใบรับรองที่สร้าง
| ไฟล์ | CN | อายุ (วัน) | ผู้ลงนาม |
|---|---|---|---|
| ca.crt | IoT Root CA | 3650 | IoT Root CA (self-signed) |
| server.crt | 10.131.32.114 | 3650 | IoT Root CA |
| client.crt | esp32-secure-node | 3650 | IoT Root CA |

![ไฟล์ cert และ key ที่สร้าง](../Image/Lab11-3/a.png)

*รูปที่ ไฟล์ cert และ key ที่สร้าง*: รัน `.\lab11-3.ps1 certs` สร้าง Root CA (`CN=IoT Root CA`), Server (`CN=10.131.32.114` พร้อม SAN IP เดียวกัน) และ Client (`CN=esp32-secure-node`) อายุ 3650 วัน (13/10/2026 ถึง 10/10/2036) แล้ว `.\lab11-3.ps1 show` ยืนยันว่า `server.crt` และ `client.crt` ผ่านการตรวจ chain (`OK`) กับ `ca.crt`

### 11-3.2 Mosquitto MQTTS (8883)
![Broker เปิดพอร์ต 8883](../Image/Lab11-3/f.png)

*รูปที่ Broker เปิดพอร์ต 8883*: รัน `.\lab11-3.ps1 broker` โหลดคอนฟิก `certs\mosquitto_ssl.conf` และขึ้น `Opening ipv4 listen socket on port 8883.` กับ `mosquitto version 2.1.2 running` (เปิดเฉพาะพอร์ต 8883) ใน log ยังเห็นการเชื่อมต่อของ `mosquitto_sub` (`10.131.32.114`) ที่ negotiate `TLSv1.3` สำเร็จ และ client ที่ใช้ใบรับรองไม่ถูกต้อง/ไม่มีใบรับรองถูกตัดด้วย `protocol error`

### 11-3.3 ทดสอบ CLI
| กรณีทดสอบ | ผลลัพธ์ | ข้อความ error/log |
|---|---|---|
| มี CA + client cert + key | สำเร็จ | Subscriber ได้รับ `secure/test Hello Secure World!` |
| ไม่มี client cert | ถูกปฏิเสธ | `Error: The connection was lost` (Broker ตัดการเชื่อมต่อในขั้น TLS handshake) |
| ใช้พอร์ต 1883 | ต่อไม่ได้ | `Error: No connection could be made because the target machine actively refused it.` |

**Subscriber (Terminal 2)**

![Subscriber รับข้อความผ่าน TLS](../Image/Lab11-3/b.png)

**Publisher: pub / attack / plain (Terminal 3)**

![ผลทดสอบ CLI ผ่าน TLS](../Image/Lab11-3/d.png)

*รูปที่ ผลทดสอบ CLI ผ่าน TLS*: คำสั่ง `pub` ที่ส่ง `ca.crt`, `client.crt`, `client.key` ครบ ส่งข้อความสำเร็จและ Subscriber ได้รับข้อความ ส่วน `attack` (ส่งเฉพาะ `ca.crt` ไม่มีใบรับรอง Client) ถูก Broker ตัดการเชื่อมต่อด้วย `The connection was lost` เพราะ `require_certificate true` และการเชื่อมต่อแบบไม่เข้ารหัสที่พอร์ต 1883 ถูกปฏิเสธ (`actively refused`) เนื่องจาก Broker เปิดเฉพาะพอร์ต 8883

### 11-3.4–5 ESP32
![Serial monitor: TLS handshake](../Image/Lab11-3/e.png)

*รูปที่ Serial monitor: TLS handshake*: ESP32 เชื่อมต่อ Wi-Fi `Bismarck .` ได้ IP `10.131.32.203` (มี `reason=202` ที่ต่อครั้งแรก แล้วต่อใหม่สำเร็จเอง) จากนั้นเริ่ม `Initializing Secure MQTTS Client with mTLS...` และขึ้น `MQTTS TLS Handshake Successful! Connected to Secure Broker.` แสดงว่าบอร์ดตรวจใบรับรอง Broker ด้วย `ca.crt` ผ่าน และ Broker ยอมรับ `client.crt` ของบอร์ด

![Broker log: Peer certificate verified](../Image/Lab11-3/g.png)

*รูปที่ Broker log: Peer certificate verified*: Broker รับการเชื่อมต่อจากบอร์ด `10.131.32.203` พอร์ต 8883 และขึ้น `New client connected ... as esp32_mtls_client_node (..., u'esp32-secure-node')` แสดงว่า Broker ตรวจใบรับรองของบอร์ดผ่านและนำ CN `esp32-secure-node` มาเป็น username ต่อด้วย `negotiated TLSv1.2 cipher ECDHE-RSA-AES256-GCM-SHA384`, `Sending CONNACK (0, 0)`, `Received PUBLISH ... 'secure/esp32/status' (18 bytes)` (`SECURE_ONLINE_MTLS`) และ `SUBSCRIBE secure/esp32/cmd (QoS 1)` จากนั้นมี `PINGREQ/PINGRESP` ต่อเนื่องเพื่อรักษาการเชื่อมต่อ

> หมายเหตุ: ก่อนหน้านั้นใน log มี `tlsv1 alert unknown ca` จากบอร์ดหลายครั้ง เกิดจากเฟิร์มแวร์ที่ฝัง `ca.crt` ชุดเก่า ไม่ตรงกับ `server.crt` ที่ออกใหม่ หลังสร้างใบรับรองใหม่และ flash ใหม่จึงเชื่อมต่อสำเร็จ

## 2. คำถามท้ายการทดลอง

**1. เหตุใด CN ของ Server ต้องตรงกับ IP/Hostname ของ Broker? หากไม่ตรงจะเกิดอะไรขึ้น?**

> คำตอบ: ระหว่าง TLS handshake ฝั่ง Client ต้องตรวจว่า Server ที่ต่ออยู่ "คือเครื่องที่ตั้งใจจะต่อจริง" โดยเทียบชื่อ/IP ที่ใช้เชื่อมต่อกับ CN (หรือ SAN) ในใบรับรอง Server หากตรงเฉพาะลายเซ็น CA แต่ชื่อไม่ตรง ผู้โจมตีที่มีใบรับรองที่ CA เดียวกันออกให้ (หรือขโมยใบรับรองของเครื่องอื่นมา) จะสวมรอยเป็น Broker ได้ (Man-in-the-Middle) ดังนั้นเมื่อ CN ไม่ตรงกับ IP/Hostname ที่ใช้เชื่อมต่อ การตรวจสอบจะล้มเหลวด้วย Hostname Mismatch (เช่น `certificate verify failed` / `X509 - Certificate verification failed, CN mismatch`) และ Client จะยกเลิกการเชื่อมต่อทันทีโดยไม่ส่งข้อมูลใดๆ เนื่องจากใช้ IP ใน Lab นี้ จึงต้องระบุ IP ของ Broker ลงใน CN (และ SAN) ให้ตรงพอดี และเมื่อ IP เปลี่ยนต้องออกใบรับรองใหม่

**2. mTLS ต่างจาก HTTPS ทั่วไปอย่างไร?**

> คำตอบ: HTTPS ทั่วไปเป็นการยืนยันตัวตนทางเดียว: เบราว์เซอร์ตรวจใบรับรองของเว็บเซิร์ฟเวอร์ แต่ Client (เบราว์เซอร์) ไม่ต้องมีใบรับรอง ผู้ใช้พิสูจน์ตัวตนภายหลังด้วยชื่อผู้ใช้/รหัสผ่านหรือ token ส่วน mTLS ทั้งสองฝ่ายต้องแสดงใบรับรอง: Broker ตั้ง `require_certificate true` เพื่อบังคับให้ Client (ESP32) ส่ง `client.crt` ที่ลงนามโดย CA ที่เชื่อถือ และพิสูจน์ว่าครอบครอง `client.key` ด้วย ทำให้อุปกรณ์ที่ไม่มีใบรับรองถูกตัดการเชื่อมต่อตั้งแต่ขั้น handshake ก่อนส่ง MQTT CONNECT ใดๆ จึงเหมาะกับอุปกรณ์ IoT ที่ไม่มีผู้ใช้พิมพ์รหัสผ่าน และตัวตนของอุปกรณ์ผูกกับใบรับรองรายตัว (ยกเลิกสิทธิ์ได้ด้วยการเพิกถอนใบรับรอง)

**3. หากไฟล์ `.bin` ถูกขโมย จะกู้ `client.key` ได้หรือไม่ และควรป้องกันอย่างไร?**

> คำตอบ: **กู้ได้** เพราะ `client.key` ถูกฝังเป็นข้อความธรรมดา (PEM, plaintext) ใน section ข้อมูลของไฟล์ `.bin` ผู้ที่ได้ firmware (หรืออ่าน flash ผ่าน `esptool read_flash`) สามารถค้นสตริง `-----BEGIN PRIVATE KEY-----` แล้วสวมรอยเป็นอุปกรณ์ได้ การป้องกันในระบบเชิงพาณิชย์ ได้แก่ (1) **Flash Encryption** เข้ารหัสเนื้อหาใน flash ด้วยกุญแจที่อยู่ใน eFuse อ่านจากภายนอกไม่ได้ (2) **Secure Boot** ให้บอร์ดรันเฉพาะ firmware ที่ลงนาม ป้องกันการแก้ไข firmware เพื่อดึงกุญแจ (3) **Digital Signature Peripheral (DS)** เก็บ private key ไว้ในฮาร์ดแวร์ ใช้เซ็นได้แต่อ่านกุญแจออกมาไม่ได้ และ (4) เก็บกุญแจใน NVS ที่เข้ารหัส/ใช้ใบรับรองรายอุปกรณ์ที่ provision ตอนผลิต และมีการเพิกถอน (CRL) เมื่ออุปกรณ์ถูกขโมย

## 3. สรุปผลการทดลอง

สร้าง Root CA และใบรับรอง Server (CN/SAN = 10.131.32.114) และ Client (`esp32-secure-node`) ด้วย OpenSSL แล้วตั้ง Mosquitto ให้เปิดพอร์ต 8883 พร้อม `require_certificate true` (mTLS) ผลทดสอบพบว่า Client ที่มี `ca.crt`, `client.crt`, `client.key` ครบเชื่อมต่อและรับ-ส่งข้อความได้ ส่วน Client ที่ไม่มีใบรับรอง Client ถูกตัดการเชื่อมต่อตั้งแต่ขั้น TLS handshake และพอร์ต 1883 แบบไม่เข้ารหัสต่อไม่ได้ ESP32 ฝังใบรับรองด้วย `EMBED_TXTFILES` และเชื่อมต่อ `mqtts://10.131.32.114:8883` สำเร็จ (TLSv1.2) โดย Broker ยืนยันตัวตนบอร์ดจากใบรับรอง (username `esp32-secure-node`) การทดลองยังแสดงว่า CN/SAN ของ Server ต้องตรงกับ IP ที่ใช้เชื่อมต่อ และเฟิร์มแวร์ต้องฝังใบรับรองชุดเดียวกับที่ Broker ใช้ ไม่เช่นนั้นจะเกิด `unknown ca`

