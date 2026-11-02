# Labsheet 11.3: MQTTS TLS & Mutual Authentication (OpenSSL X.509 Certificates)


#### 1. บันทึกผลการทดลอง / โค้ดที่พัฒนา

1. การสร้างกุญแจและใบรับรองดิจิทัล X.509 ด้วย OpenSSL:
```bash
# 1. Root CA
openssl genrsa -out ca.key 2048
openssl req -out ca.csr -key ca.key -new -subj "/C=TH/ST=Bangkok/O=IoT Security Lab/CN=IoT Root CA"
openssl x509 -req -in ca.csr -out ca.crt -sha256 -days 3650 -signkey ca.key

# 2. Server Certificate สำหรับ Broker (ระบุ CN เป็น IP ของ Broker)
openssl genrsa -out server.key 2048
openssl req -out server.csr -key server.key -new -subj "/C=TH/ST=Bangkok/O=IoT Broker/CN=192.168.1.100"
openssl x509 -req -in server.csr -out server.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650

# 3. Client Certificate สำหรับ ESP32
openssl genrsa -out client.key 2048
openssl req -out client.csr -key client.key -new -subj "/C=TH/ST=Bangkok/O=IoT Node/CN=esp32-secure-node"
openssl x509 -req -in client.csr -out client.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650
```

2. การตั้งค่า `mosquitto_ssl.conf` บนพอร์ต 8883 พร้อม mTLS:
```text
listener 8883 0.0.0.0
cafile C:/mqtt_certs/ca.crt
certfile C:/mqtt_certs/server.crt
keyfile C:/mqtt_certs/server.key
require_certificate true
tls_version tlsv1.2
log_type all
```

3. การฝังไฟล์ใบรับรองลงในไบนารีผ่าน `main/CMakeLists.txt`:
```cmake
idf_component_register(
    SRCS "Lab11-3_MQTTS_Secure.c"
    INCLUDE_DIRS "."
    EMBED_TXTFILES 
        "certs/ca.crt"
        "certs/client.crt"
        "certs/client.key"
    REQUIRES mqtt esp_wifi esp_event nvs_flash
)
```

4. การกำหนดค่า MQTTS Client ในโค้ด ESP32:
```c
extern const uint8_t ca_crt_start[] asm("_binary_ca_crt_start");
extern const uint8_t client_crt_start[] asm("_binary_client_crt_start");
extern const uint8_t client_key_start[] asm("_binary_client_key_start");

esp_mqtt_client_config_t mqtt_cfg = {
    .broker = {
        .address.uri = "mqtts://192.168.1.100:8883",
        .verification.certificate = (const char *)ca_crt_start,
    },
    .credentials = {
        .client_id = "esp32_mtls_client_node",
        .authentication = {
            .certificate = (const char *)client_crt_start,
            .key = (const char *)client_key_start,
        },
    },
};
```

#### 2. ผลลัพธ์จาก Serial Monitor / หน้าจอแสดงผล

ผลการทดสอบการเชื่อมต่อแบบถูกต้องใน Serial Monitor:
```text
I (1850) LAB11_SEC: Wi-Fi Connected! Initializing Secure MQTTS Client with mTLS...
I (2120) LAB11_SEC: Connecting to mqtts://192.168.1.100:8883...
I (2650) LAB11_SEC: MQTTS TLS Handshake Successful! Connected to Secure Broker.
I (2670) LAB11_SEC: Published to secure/esp32/status: SECURE_ONLINE_MTLS
```

บันทึกจากหน้าต่าง Mosquitto Broker Verbose Log:
```text
New connection from 192.168.1.181:54321 on port 8883.
Peer certificate verified: /C=TH/ST=Bangkok/O=IoT Node/CN=esp32-secure-node
New client connected from 192.168.1.181:54321 as esp32_mtls_client_node (p2, c1, k60).
```

ผลการทดสอบปฏิเสธผู้บุกรุก (Unauthorized Client Attack):
เมื่อใช้คำสั่ง `mosquitto_pub` พยายามส่งข้อความโดยไม่แนบใบรับรอง Client:
```powershell
mosquitto_pub -h 192.168.1.100 -p 8883 -t "secure/test" -m "Attack" --cafile ca.crt
```
- เทอร์มินัลแจ้งเตือน: `OpenSSL Error: ssl handshake failure` หรือ `Connection refused: Not authorised`
- ฝั่ง Broker Log แจ้ง: `Client disconnected due to TLS error: peer did not return a certificate` แสดงว่าระบบความปลอดภัย mTLS ทำงานได้อย่างสมบูรณ์

#### 3. สรุปผลและการตอบคำถามท้ายใบงาน

- **สรุปผลการทดลอง:**
สามารถติดตั้งระบบความปลอดภัยระดับสูงบน MQTT ด้วยการเข้ารหัส TLS v1.2 ร่วมกับการยืนยันตัวตนแบบสองทาง (Mutual TLS / mTLS) บนพอร์ต 8883 ได้สำเร็จ โดย ESP32 ใช้ Root CA ในการตรวจสอบความถูกต้องของ Broker และส่ง Client Certificate ที่สร้างจาก OpenSSL เพื่อยืนยันตัวตนกลับไปยัง Mosquitto Broker ทำให้ช่องทางการสื่อสารทั้งหมดถูกเข้ารหัสลับอย่างสมบูรณ์ ป้องกันการดักฟังข้อมูล (Eavesdropping) และสามารถปฏิเสธการเชื่อมต่อจากอุปกรณ์ภายนอกที่ไม่ได้รับอนุญาต (Unauthorized Clients) ได้อย่างมีประสิทธิภาพ

- **คำถามท้ายการทดลอง (Post-Lab Questions):**
1. **เพราะเหตุใดในคำสั่งสร้าง Server CSR ค่า Common Name (CN) จึงต้องระบุเป็น IP Address หรือ Hostname ของ Broker ให้ตรงกันพอดี? หากระบุเป็นชื่ออื่นจะเกิดข้อผิดพลาดใดขึ้น?**
     - **คำตอบ:** เพื่อป้องกันการโจมตีแบบดักรับข้อมูลกลางทาง (Man-in-the-Middle: MitM) โดย Client จะตรวจสอบว่าโฮสต์ที่กำลังเชื่อมต่อตรงกับตัวตนที่ระบุไว้ในใบรับรองหรือไม่ หากระบุไม่ตรงจะเกิดข้อผิดพลาด Hostname / IP Mismatch (เช่น Certificate verification failed) และ TLS Handshake จะถูกยกเลิกทันที

2. **การยืนยันตัวตนแบบ Mutual Authentication (mTLS) แตกต่างจากการใช้ HTTPS ทั่วไปบนเว็บเบราว์เซอร์อย่างไรในประเด็นเรื่องใบรับรองฝั่ง Client?**
     - **คำตอบ:** HTTPS ทั่วไปเป็นแบบทางเดียว (One-way TLS) โดยตรวจสอบเฉพาะใบรับรองของ Server ฝั่งเดียว ส่วน Client มักยืนยันตัวตนด้วย Username/Password ขณะที่ mTLS บังคับให้ฝั่ง Client ต้องส่งใบรับรองดิจิทัล (Client Certificate) และใช้ Private Key ยืนยันตัวตนกลับไปยัง Server ด้วยเสมอ

3. **หากมีผู้ไม่หวังดีขโมยไฟล์เฟิร์มแวร์ไบนารี (`.bin`) ของบอร์ด ESP32 ไปได้ เขาจะสามารถกู้คืนไฟล์ `client.key` ได้หรือไม่ และในระบบ IoT เชิงพาณิชย์เรามีฮาร์ดแวร์ความปลอดภัยใดมาช่วยแก้ไขปัญหานี้?**
     - **คำตอบ:** กู้คืนได้ เพราะคำสั่ง EMBED_TXTFILES เป็นการฝัง Private Key ลงไปเป็น Plaintext ใน Flash Memory จึงสามารถใช้คำสั่ง Dump/Reverse Engineering ดึงออกมาได้โดยตรง วิธีป้องกันในระดับฮาร์ดแวร์คือการเปิดใช้งาน Flash Encryption + Secure Boot บนตัวชิป ESP32 หรือใช้อุปกรณ์เสริมประเภท

---