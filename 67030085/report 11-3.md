# ใบงานการทดลองที่ 11.3
### การสื่อสาร MQTT ความปลอดภัยสูงด้วย TLS และการยืนยันตัวตนสองทาง (mTLS) บน Mosquitto และ ESP-IDF

---

## ผลการทดลองและการบันทึกภาพหน้าจอ

### 1. ภาพหน้าจอการทดสอบ mTLS สำเร็จผ่าน Mosquitto CLI (Sub/Pub พร้อม Certificate)
* แสดงผลลัพธ์ในหน้าต่าง Subscriber ที่ได้รับข้อความ `"Hello Secure World!"` พร้อม Log ฝั่ง Mosquitto Broker ที่แสดงการ Handshake และยืนยันใบรับรอง Client

<img width="1457" height="483" alt="image" src="https://github.com/user-attachments/assets/8a49668b-c315-4581-a482-5cb06e7fa539" />


---

### 2. ภาพหน้าจอการทดสอบปฏิเสธ Client ที่ไม่ได้รับอนุญาต (Unauthorized Client Attack)
* แสดงผลลัพธ์เมื่อส่ง Publish โดยไม่แนบ Client Certificate แล้ว Broker แจ้งปฏิเสธการเชื่อมต่อ `Connection refused: Not authorised` หรือ `peer did not return a certificate`

<img width="1456" height="152" alt="image" src="https://github.com/user-attachments/assets/8c52f09f-85a2-487c-9ca1-b824b8bf50a2" />


---

### 3. ภาพหน้าจอ ESP32 เชื่อมต่อ MQTTS แบบ mTLS สำเร็จ
* แสดงผลหน้าจอ Serial Monitor ของ ESP32 ขณะทำ TLS Handshake สำเร็จ และ Broker Log แสดงการยืนยันใบรับรองของ ESP32 (`Peer certificate verified: ... CN=esp32-secure-node`)

<img width="1462" height="488" alt="image" src="https://github.com/user-attachments/assets/6b29836d-32ad-4577-a0b3-f60d5d1f07b8" />


---

## คำถามท้ายการทดลอง (Post-Lab Questions)

### ข้อ 1: เพราะเหตุใดในคำสั่งสร้าง Server CSR ค่า Common Name (CN) จึงต้องระบุเป็น IP Address หรือ Hostname ของ Broker ให้ตรงกันพอดี? หากระบุเป็นชื่ออื่นจะเกิดข้อผิดพลาดใดขึ้น?
* **คำตอบ:**
  1. **เหตุผลที่ต้องระบุให้ตรงกันพอดี:**
     * ในมาตรฐานความปลอดภัย TLS/SSL ฝั่ง Client (บอร์ด ESP32 / MbedTLS หรือ Mosquitto CLI) จะต้องทำการตรวจสอบความถูกต้องและพิสูจน์ตัวตนของ Server เสมอ โดยจะนำหมายเลข IP Address หรือ Hostname ใน URL ปลายทางที่สั่งเชื่อมต่อไปเปรียบเทียบกับค่า **Common Name (CN)** หรือ **Subject Alternative Name (SAN)** ที่ระบุอยู่ในใบรับรองของ Server
     * เพื่อเป็นการยืนยันว่าเครื่องที่เราเชื่อมต่อเข้าไปนั้นเป็น Server ตัวจริง ป้องกันการถูกดักรับข้อมูลหรือโจมตีแบบสวมรอยเป็นคนกลาง (Man-in-the-Middle Attack: MitM)
  2. **ข้อผิดพลาดที่จะเกิดขึ้นหากระบุเป็นชื่ออื่น:**
     * ในขั้นตอน TLS Handshake ฝั่ง Client จะตรวจพบว่า Hostname/IP ไม่ตรงกับค่าในใบรับรอง เกิดข้อผิดพลาด **Hostname/CN Mismatch (Certificate Verification Failed)**
     * ไลบรารีความปลอดภัย (MbedTLS บน ESP32) จะทำการปฏิเสธและตัดการเชื่อมต่อทันที พร้อมรายงานข้อผิดพลาดระดับ Transport (เช่น `ESP_ERR_MBEDTLS_SSL_HANDSHAKE_FAILED` หรือ `X509 - Certificate verification failed, name mismatch`) ทำให้ไม่สามารถสร้างช่องทางการสื่อสาร MQTTS ได้

---

### ข้อ 2: การยืนยันตัวตนแบบ Mutual Authentication (mTLS) แตกต่างจากการใช้ HTTPS ทั่วไปบนเว็บเบราว์เซอร์อย่างไรในประเด็นเรื่องใบรับรองฝั่ง Client?
* **คำตอบ:**
  1. **HTTPS ทั่วไปบนเว็บเบราว์เซอร์ (One-Way TLS / Server Authentication):**
     * เป็นการตรวจสอบใบรับรอง **เพียงฝั่ง Server ฝ่ายเดียว** เพื่อให้ผู้ใช้มั่นใจว่าเข้าเว็บไซต์ที่ถูกต้อง
     * ฝั่ง **Client (เบราว์เซอร์ / ผู้ใช้) ไม่จำเป็นต้องมี Digital Certificate** โดยการระบุตัวตนของผู้ใช้จะถูกผลักไปทำที่ระดับแอปพลิเคชัน (Application Layer) แทน เช่น การกรอก Username, Password, Token, หรือ OTP
  2. **Mutual Authentication (mTLS / Two-Way Authentication):**
     * เป็นการ **ตรวจสอบใบรับรองดิจิทัลซึ่งกันและกันทั้งสองฝั่ง (Mutual Verification)**
     * ในระหว่างกระบวนการ TLS Handshake นอกจาก Client จะตรวจ Server Certificate แล้ว ตัว Server (Broker) จะส่งคำขอ `Certificate Request` บังคับให้ Client ต้องส่งใบรับรอง (`client.crt`) และพิสูจน์ความเป็นเจ้าของด้วยกุญแจลับ (`client.key`) กลับมาด้วย
     * หาก Client ไม่มีใบรับรอง หรือใบรับรองไม่ได้ถูกลงนามรับรองโดย Root CA ที่ Broker เชื่อถือ Server จะ **ปฏิเสธและตัดสายทิ้งทันทีตั้งแต่ระดับ Cryptographic Handshake** ก่อนจะเริ่มส่งข้อมูล MQTT ใดๆ
     * ให้ระดับความปลอดภัยสูงมาก เหมาะสำหรับระบบ IoT ระดับอุตสาหกรรม/องค์กร เพราะเป็นการยืนยันสิทธิ์ของอุปกรณ์ฮาร์ดแวร์โดยตรง (Hardware Device Identity) ป้องกันอุปกรณ์แปลกปลอมเข้ามาในเครือข่าย

---

### ข้อ 3: หากมีผู้ไม่หวังดีขโมยไฟล์เฟิร์มแวร์ไบนารี (`.bin`) ของบอร์ด ESP32 ไปได้ เขาจะสามารถกู้คืนไฟล์ `client.key` ได้หรือไม่ และในระบบ IoT เชิงพาณิชย์เรามีฮาร์ดแวร์ความปลอดภัยใด (เช่น ESP32 Secure Boot / Flash Encryption / Digital Signature Peripheral) มาช่วยแก้ไขปัญหานี้?
* **คำตอบ:**
  1. **การกู้คืนไฟล์ `client.key` จากไฟล์ไบนารี:**
     * **สามารถกู้คืนได้ง่ายมาก** หากไม่มีระบบป้องกันเสริม เพราะคำสั่ง `EMBED_TXTFILES` ใน CMake จะนำเนื้อหาของ `client.key` (RSA Private Key ในรูปแบบ ASCII PEM) ไปฝังลงใน Flash Binary เป็นข้อความธรรมดา (Plaintext)
     * หากผู้ไม่หวังดีดัมป์ไฟล์ไบนารี (`.bin`) ออกมาจาก Flash Memory หรือขโมยไฟล์เฟิร์มแวร์ไปได้ เพียงใช้เครื่องมือแยกข้อความธรรมดา เช่น คำสั่ง `strings firmware.bin` หรือ Hex Editor ก็จะพบข้อความ `-----BEGIN RSA PRIVATE KEY-----` และสามารถคัดลอกกุญแจลับออกมาสวมรอยเป็นอุปกรณ์ตัวนั้นได้อย่างสมบูรณ์
  2. **ฮาร์ดแวร์ความปลอดภัยของ ESP32 ที่นำมาช่วยแก้ไขปัญหาในเชิงพาณิชย์:**
     * **ESP32 Flash Encryption (AES-XTS 256-bit):** ระบบเข้ารหัสข้อมูลทั้งหมดใน Flash Memory ด้วยฮาร์ดแวร์ โดยกุญแจเข้ารหัสถูกฝังอยู่ใน eFuse ของชิปและไม่สามารถอ่านออกสู่ภายนอกได้ ทำให้ไฟล์ที่ถูกดูดออกไปกลายเป็นข้อมูลขยะที่อ่านไม่ออก ไม่สามารถสกัด Private Key ได้
     * **ESP32 Secure Boot:** ตรวจสอบลายมือชื่อดิจิทัล (Digital Signature) ของเฟิร์มแวร์ทุกครั้งที่เปิดเครื่อง เพื่อป้องกันไม่ให้ผู้ไม่หวังดีแฟลชเฟิร์มแวร์ดัดแปลงเพื่อแอบอ่านค่า Key หรือควบคุมบอร์ด
     * **Digital Signature Peripheral (DS) ร่วมกับ eFuse:** เป็นโมดูลฮาร์ดแวร์พิเศษในชิป ESP32 รุ่นใหม่ ที่นำ Private Key ไปจัดเก็บไว้ใน eFuse ซึ่งถูกเข้ารหัสด้วยฮาร์ดแวร์ ซอฟต์แวร์จะไม่สามารถเข้าถึงเนื้อ Private Key ได้โดยตรง แต่เมื่อต้องการทำ TLS Handshake ตัวฮาร์ดแวร์ DS นี้จะทำการคำนวณการเข้ารหัส/ลงลายมือชื่อดิจิทัลให้ภายในชิปโดยตรง ทำให้มั่นใจได้ว่ากุญแจลับจะไม่รั่วไหลออกสู่นอกชิปอย่างแน่นอน

