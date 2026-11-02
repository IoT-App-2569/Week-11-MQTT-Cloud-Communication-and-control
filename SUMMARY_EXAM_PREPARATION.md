# 📑 สรุปเนื้อหาและสาระสำคัญสำหรับเตรียมสอบ
## สัปดาห์ที่ 11: MQTT Cloud Communication and Control & ESP RainMaker Ecosystem
> **อ้างอิงรายวิชา:** สถาปัตยกรรม IoT, ESP-IDF และหนังสือ *ESP32-C3 Wireless Adventure (Chapter 9)*  
> **รูปแบบเอกสาร:** ออกแบบเพื่อการอ่านทบทวนเตรียมสอบฉบับเร่งรัด มีสารบัญชัดเจน สามารถคัดลอก (Copy & Paste) ลงโปรแกรม Microsoft Word ได้อย่างสมบูรณ์แบบ

---

## 📌 สารบัญ (Table of Contents)
- [บทที่ 1: สถาปัตยกรรม Cloud Control และโปรโตคอล MQTT พื้นฐาน](#บทที่-1-สถาปัตยกรรม-cloud-control-และโปรโตคอล-mqtt-พื้นฐาน)
  - [1.1 Local Control เทียบกับ Cloud Control](#11-local-control-เทียบกับ-cloud-control)
  - [1.2 สถาปัตยกรรม Publish/Subscribe และการ Decoupling](#12-สถาปัตยกรรม-publishsubscribe-และการ-decoupling)
  - [1.3 ระบบหัวข้อ (Topics) และไวลด์การ์ด (+ และ #)](#13-ระบบหัวข้อ-topics-และไวลด์การ์ด--และ-)
  - [1.4 โครงสร้างแพ็กเก็ต MQTT และ Remaining Length Encoding](#14-โครงสร้างแพ็กเก็ต-mqtt-และ-remaining-length-encoding)
  - [1.5 ระดับคุณภาพการให้บริการ (QoS 0, 1, 2) และ Packet Handshake](#15-ระดับคุณภาพการให้บริการ-qos-0-1-2-และ-packet-handshake)
  - [1.6 คุณสมบัติขั้นสูง: Retained Message, Keep-Alive, LWT](#16-คุณสมบัติขั้นสูง-retained-message-keep-alive-lwt)
  - [1.7 การเปรียบเทียบ MQTT vs HTTP vs CoAP](#17-การเปรียบเทียบ-mqtt-vs-http-vs-coap)
- [บทที่ 2: การพัฒนา ESP-IDF MQTT Client และการติดตั้ง Broker](#บทที่-2-การพัฒนา-esp-idf-mqtt-client-และการติดตั้ง-broker)
  - [2.1 การติดตั้งและคอนฟิก Mosquitto Broker บน Windows/Linux](#21-การติดตั้งและคอนฟิก-mosquitto-broker-บน-windowslinux)
  - [2.2 โครงสร้างคอมโพเนนต์ esp-mqtt บน ESP-IDF](#22-โครงสร้างคอมโพเนนต์-esp-mqtt-บน-esp-idf)
  - [2.3 วงจรชีวิต Event Loop และ Event ID สำคัญ](#23-วงจรชีวิต-event-loop-และ-event-id-สำคัญ)
  - [2.4 FreeRTOS Task, cJSON Serializer และการจัดการ Memory](#24-freertos-task-cjson-serializer-และการจัดการ-memory)
  - [2.5 ข้อควรระวังระดับปฏิบัติการ (Split-Brain Broker, Buffer Null-terminator)](#25-ข้อควรระวังระดับปฏิบัติการ-split-brain-broker-buffer-null-terminator)
- [บทที่ 3: ความปลอดภัย MQTT, โครงสร้างพื้นฐาน TLS และ mTLS](#บทที่-3-ความปลอดภัย-mqtt-โครงสร้างพื้นฐาน-tls-และ-mtls)
  - [3.1 ช่องโหว่ของ Plaintext MQTT และหลักการสามเหลี่ยม CIA](#31-ช่องโหว่ของ-plaintext-mqtt-และหลักการสามเหลี่ยม-cia)
  - [3.2 การเข้ารหัสแบบสมมาตร vs อสมมาตร และ Hybrid TLS](#32-การเข้ารหัสแบบสมมาตร-vs-อสมมาตร-และ-hybrid-tls)
  - [3.3 โครงสร้าง Public Key Infrastructure (PKI) และใบรับรอง X.509](#33-โครงสร้าง-public-key-infrastructure-pki-และใบรับรอง-x509)
  - [3.4 ลำดับคำสั่ง OpenSSL ในการออกใบรับรอง (Root CA, Server, Client)](#34-ลำดับคำสั่ง-openssl-ในการออกใบรับรอง-root-ca-server-client)
  - [3.5 กฎเหล็กของ Common Name (CN) และการป้องกัน Hostname Mismatch](#35-กฎเหล็กของ-common-name-cn-และการป้องกัน-hostname-mismatch)
  - [3.6 การฝังใบรับรองผ่าน CMake EMBED_TXTFILES บน ESP-IDF](#36-การฝังใบรับรองผ่าน-cmake-embed_txtfiles-บน-esp-idf)
  - [3.7 One-Way TLS เทียบกับ Mutual TLS (mTLS)](#37-one-way-tls-เทียบกับ-mutual-tls-mtls)
  - [3.8 การรักษาความปลอดภัยฮาร์ดแวร์ ESP32 (Flash Encryption, Secure Boot, DS)](#38-การรักษาความปลอดภัยฮาร์ดแวร์-esp32-flash-encryption-secure-boot-ds)
- [บทที่ 4: แพลตฟอร์มคลาวด์ ESP RainMaker Ecosystem](#บทที่-4-แพลตฟอร์มคลาวด์-esp-rainmaker-ecosystem)
  - [4.1 สถาปัตยกรรมและ 4 องค์ประกอบหลักของ ESP RainMaker](#41-สถาปัตยกรรมและ-4-องค์ประกอบหลักของ-esp-rainmaker)
  - [4.2 แบบจำลองข้อมูลอุปกรณ์ (Data Model: Node, Device, Param, Service)](#42-แบบจำลองข้อมูลอุปกรณ์-data-model-node-device-param-service)
  - [4.3 กลไกการขอรับสิทธิ์และการติดตั้งใบรับรอง (Claiming Mechanisms)](#43-กลไกการขอรับสิทธิ์และการติดตั้งใบรับรอง-claiming-mechanisms)
  - [4.4 การทำงานของ Write Callback และการซิงโครไนซ์สถานะ Device Shadow](#44-การทำงานของ-write-callback-และการซิงโครไนซ์สถานะ-device-shadow)
  - [4.5 การผสานงานระบบเสียง Google Assistant / Alexa (Cloud-to-Cloud Integration)](#45-การผสานงานระบบเสียง-google-assistant--alexa-cloud-to-cloud-integration)
- [บทที่ 5: สรุปจุดเน้นการทดลองปฏิบัติการ 4 ใบงาน (Lab Essentials)](#บทที่-5-สรุปจุดเน้นการทดลองปฏิบัติการ-4-ใบงาน-lab-essentials)
  - [5.1 Lab 11.1: Mosquitto Broker and MQTT Fundamentals](#51-lab-111-mosquitto-broker-and-mqtt-fundamentals)
  - [5.2 Lab 11.2: ESP32 MQTT Client Telemetry and Control](#52-lab-112-esp32-mqtt-client-telemetry-and-control)
  - [5.3 Lab 11.3: MQTTS TLS Mutual Authentication Security](#53-lab-113-mqtts-tls-mutual-authentication-security)
  - [5.4 Lab 11.4: ESP RainMaker Cloud IoT and Smart Device](#54-lab-114-esp-rainmaker-cloud-iot-and-smart-device)
- [บทที่ 6: คลังคำศัพท์สำคัญและการนิยาม (Key Terminology)](#บทที่-6-คลังคำศัพท์สำคัญและการนิยาม-key-terminology)

---

# บทที่ 1: สถาปัตยกรรม Cloud Control และโปรโตคอล MQTT พื้นฐาน

### 1.1 Local Control เทียบกับ Cloud Control
ในการสื่อสาร IoT ขอบเขตทางกายภาพของระบบแบ่งออกเป็น 2 รูปแบบสำคัญ:
* **Local Control (LAN):** สื่อสารตรงผ่าน IP วงแลนเดียวกัน (เช่น HTTP REST, UDP, CoAP, mDNS) ข้อดีคือหน่วงเวลาต่ำมาก (< 10 ms) ไม่ต้องพึ่งเน็ตภายนอก แต่มีข้อจำกัดคือ "ผู้สั่งการและอุปกรณ์ต้องอยู่ใน Wi-Fi วงเดียวกันเท่านั้น"
* **Cloud Control (WAN):** การสื่อสารข้ามเครือข่ายอินเทอร์เน็ตผ่าน **Cloud Server / Broker กลาง** 
  * **สาเหตุที่ต้องมี Broker กลาง:** เราเตอร์ตามบ้านทำงานผ่าน **NAT (Network Address Translation)** และมีไฟร์วอลล์บล็อกการเชื่อมต่อขาเข้า (Inbound) จากอินเทอร์เน็ต ทำให้มือถือภายนอกไม่สามารถยิงคำสั่งตรงหา IP ท้องถิ่นของ ESP32 ได้
  * **ทางแก้:** ให้ทั้งมือถือและ ESP32 ทำการสร้างการเชื่อมต่อขาออก (**Outbound TCP Connection**) ขึ้นไปเกาะที่ Broker กลางที่มี Public IP/Domain ถาวร

| มิติการเปรียบเทียบ | Local Control (LAN) | Cloud Control (WAN) |
| :--- | :--- | :--- |
| **ขอบเขตการเข้าถึง** | วงแลนเดียวกันเท่านั้น | ไร้ขีดจำกัดจากทั่วโลก (4G/5G/WAN) |
| **ความหน่วงเวลา (Latency)** | ต่ำมาก (1–10 ms) | 50–300 ms (ตามระยะทางอินเทอร์เน็ต) |
| **การพึ่งพาอินเทอร์เน็ต** | ไม่พึ่งพา ทำงานได้แม้เน็ตตัด | ต้องพึ่งพาอินเทอร์เน็ตทั้งสองฝั่ง |
| **ความปลอดภัย** | เสี่ยงเฉพาะคนในวงแลน | ต้องเข้ารหัสข้อมูล (จำเป็นต้องใช้ TLS/mTLS) |

---

### 1.2 สถาปัตยกรรม Publish/Subscribe และการ Decoupling
โปรโตคอล **MQTT (Message Queuing Telemetry Transport)** เป็นมาตรฐานเปิด (ISO/IEC 20922) คิดค้นโดย Andy Stanford-Clark และ Arlen Nipper ทำงานบน **TCP/IP** โดยใช้โมเดล **Publish/Subscribe**:
1. **Publisher (ผู้เผยแพร่):** ไคลเอนต์ที่ส่งข้อความ (Payload) พร้อมระบุหัวข้อ (Topic) ขึ้นสู่ Broker
2. **Broker (ตัวกลาง):** เซิร์ฟเวอร์ที่รับแพ็กเก็ตจาก Publisher และคัดกรองส่งต่อ (Forward) ไปยัง Subscriber
3. **Subscriber (ผู้บอกรับ):** ไคลเอนต์ที่ลงทะเบียนหัวข้อที่สนใจไว้กับ Broker

> **จุดเด่น: การตัดขาดความสัมพันธ์โดยตรง (Decoupling) 3 มิติ:**
> 1. **Space Decoupling (เชิงพื้นที่):** ทั้งสองฝั่งไม่ต้องรู้ IP/Port ของกันและกัน รู้จักแค่ Broker
> 2. **Time Decoupling (เชิงเวลา):** ผู้ส่งและผู้รับไม่ต้องออนไลน์พร้อมกัน (อาศัย Retained Message / Persistent Session)
> 3. **Synchronization Decoupling (เชิงการประสานเวลา):** การส่งข้อมูลเป็นแบบ Asynchronous ผู้ส่งไม่ต้องค้างรอผู้รับประมวลผล

---

### 1.3 ระบบหัวข้อ (Topics) และไวลด์การ์ด (+ และ #)
Topic เป็นสตริงข้อความแบบลำดับชั้นคั่นด้วยเครื่องหมาย slash (`/`) เช่น `home/livingroom/temp`
* **กฎการตั้งชื่อ:** ไวต่อตัวพิมพ์เล็ก-ใหญ่ (**Case Sensitive**), ห้ามขึ้นต้นด้วย `$SYS/` (เป็นหัวข้อสงวนของ Broker), ไม่ควรขึ้นต้นด้วย `/` (จะเกิด root layer ว่างเปล่า)
* **ไวลด์การ์ด (Wildcards) - ใช้ได้เฉพาะขั้นตอน SUBSCRIBE เท่านั้น (ห้ามใช้ตอน PUBLISH):**
  1. **Single-Level Wildcard (`+`):** แทนที่ชื่อหัวข้อย่อยได้ **1 ระดับชั้นพอดี** ณ ตำแหน่งนั้น
     * เช่น `home/+/light` $\rightarrow$ ตรงกับ `home/kitchen/light` แต่ **ไม่ตรง** กับ `home/bedroom/balcony/light`
  2. **Multi-Level Wildcard (`#`):** แทนที่ชื่อหัวข้อย่อยได้ **ตั้งแต่ศูนย์ถึงไม่จำกัดระดับชั้น** และ **ต้องอยู่ท้ายสุดของ Topic เสมอ**
     * เช่น `home/bedroom/#` $\rightarrow$ ตรงกับ `home/bedroom/light`, `home/bedroom/ac/temp`, `home/bedroom`
     * หากกำหนด `home/#/light` จะถือว่า **ผิดไวยากรณ์ (Malformed Syntax)** โบรกเกอร์จะ Reject ทันที

---

### 1.4 โครงสร้างแพ็กเก็ต MQTT และ Remaining Length Encoding
แพ็กเก็ตควบคุม (Control Packet) มี Overhead ต่ำมาก เริ่มต้นที่ Fixed Header เพียง **2 ไบต์**:
* **Byte 1:**
  * Bit 7–4: **Packet Type** (4 บิต รวมได้ 16 ชนิด เช่น 1=`CONNECT`, 2=`CONNACK`, 3=`PUBLISH`, 4=`PUBACK`, 12=`PINGREQ`, 14=`DISCONNECT`)
  * Bit 3–0: **Flags** เฉพาะของแพ็กเก็ต (DUP, QoS, RETAIN)
* **Byte 2–5: Remaining Length (ความยาวที่เหลือ):**
  * เข้ารหัสแบบ **7-bit Base-128 Variable Length Encoding**
  * บิต 0–6 เก็บค่าตัวเลข (0–127)
  * บิต 7 เป็น **Continuation Bit** (ถ้าเป็น 1 แปลว่ามีไบต์ถัดไป, ถ้าเป็น 0 แปลว่าไบต์สุดท้าย)
  * รองรับความยาวข้อมูลได้สูงสุด **256 MB** (ใช้ 4 ไบต์: `0xFF 0xFF 0xFF 0x7F`)

---

### 1.5 ระดับคุณภาพการให้บริการ (QoS 0, 1, 2) และ Packet Handshake
MQTT ให้ผู้พัฒนาเลือกระดับการันตีการส่งข้อมูลได้ 3 ระดับ:

```
[QoS 0: At most once]          [QoS 1: At least once]             [QoS 2: Exactly once]
   Client        Broker           Client        Broker               Client        Broker
     |             |                |             |                    |             |
     |--PUBLISH--->|                |--PUBLISH--->| (มี Packet ID)      |--PUBLISH--->|
     |             |                |<-PUBACK-----|                    |<-PUBREC-----|
     |             |                |             | (ถ้าไม่ตอบจะส่งซ้ำ   |--PUBREL---->|
     |             |                |             |  เปิดบิต DUP=1)    |<-PUBCOMP----|
```

1. **QoS 0 (At Most Once):** ส่งครั้งเดียวแบบ Fire-and-forget ไม่มีการตอบรับ ACK ข้อมูลอาจหายได้ หน่วงเวลาต่ำสุด กินแบนด์วิดท์น้อยสุด เหมาะกับข้อมูล GPS หรือ Telemetry เซนเซอร์ที่ส่งถี่ทุกๆ วินาที
2. **QoS 1 (At Least Once):** ผู้รับต้องตอบ `PUBACK` หากหลุดหรือ Time-out จะส่งซ้ำ (บิต DUP=1) การันตีว่าข้อความถึงแน่ แต่อาจเกิดข้อความซ้ำซ้อน (Duplicate) ได้ เหมาะกับการแจ้งเตือน Alarm
3. **QoS 2 (Exactly Once):** การันตีถึงครั้งเดียวแน่นอน ไม่ขาดและไม่เกิน ผ่านกระบวนการ 4-Way Handshake (`PUBLISH` $\rightarrow$ `PUBREC` $\rightarrow$ `PUBREL` $\rightarrow$ `PUBCOMP`) มี Overhead สูงสุด เหมาะกับธุรกรรมการเงิน คำสั่งเปิดประตูดิจิทัล หรือระบบดับเพลิง

> **สมการ Delivered QoS:**  
> $$\text{Delivered QoS} = \min(\text{QoS}_{\text{publish}}, \text{QoS}_{\text{subscribe}})$$  
> *(ตัวอย่าง: ส่งมา QoS 1 แต่ผู้รับ Sub ไว้ด้วย QoS 0 โบรกเกอร์จะลดระดับส่งให้ผู้รับเป็น QoS 0)*

---

### 1.6 คุณสมบัติขั้นสูง: Retained Message, Keep-Alive, LWT
1. **Retained Message (`-r` flag):** สั่งให้ Broker บันทึกข้อความล่าสุดของหัวข้อนั้นไว้ใน RAM อย่างถาวร เมื่อมี Subscriber หน้าใหม่เพิ่งเชื่อมต่อเข้ามา Broker จะยิงข้อความ Retained ล่าสุดนี้ให้ทันที แก้ปัญหาหน้าจอ Dashboard ว่างเปล่า (Blank Dashboard)
   * *วิธีล้างข้อความ Retained:* ส่ง Payload ว่าง (Zero-byte) พร้อมเปิดแฟล็ก `-r`
2. **Keep-Alive & Heartbeat (`PINGREQ` / `PINGRESP`):** กำหนดช่วงเวลานับถอยหลัง (เช่น 60 วินาที) หากไม่มีข้อมูลวิ่ง Client ต้องส่งแพ็กเก็ต 2 ไบต์ `PINGREQ` ไปสะกิด Broker หาก Broker ไม่ได้รับข้อมูลเกิน 1.5 เท่าของ Keep-Alive (90 วินาที) จะถือว่าลิงก์ขาด (Dead Connection) และสั่งตัดทันที
3. **Last Will and Testament (LWT):** พินัยกรรมฉุกเฉินที่ Client ฝากไว้กับ Broker ตั้งแต่แพ็กเก็ต `CONNECT` หาก Client หลุดอย่างผิดปกติ (Unclean Disconnect เช่น ไฟดับ เน็ตหลุด) Broker จะ Publish ข้อความ Will นี้ไปยังหัวข้อที่ระบุแทนให้อัตโนมัติ (แต่ถ้า Client ส่ง `DISCONNECT` อย่างถูกต้อง พินัยกรรมนี้จะถูกยกเลิกทิ้งทันที)

---

### 1.7 การเปรียบเทียบ MQTT vs HTTP vs CoAP
| ปัจจัย | MQTT | HTTP REST (1.1) | CoAP |
| :--- | :--- | :--- | :--- |
| **รูปแบบ** | Publish / Subscribe | Client / Server (Req/Resp) | REST-like (Req/Resp, Observe) |
| **Transport Layer** | **TCP** | **TCP** | **UDP** |
| **Fixed Header** | **2 ไบต์** | ตัวอักษร Text (> 100 ไบต์) | **4 ไบต์** |
| **การทะลุผ่าน NAT** | ดีเยี่ยม (Outbound connection) | ลำบาก (อุปกรณ์ทำตัวเป็น Server ไม่ได้) | ลำบากมากบน WAN |
| **การกระจาย 1 ต่อ N** | มีประสิทธิภาพสูงสุดในตัว | ต้องวนส่งทีละเครื่อง | รองรับ Multicast วงแลน |
| **พอร์ตมาตรฐาน** | 1883 (Plain) / 8883 (TLS) | 80 (HTTP) / 443 (HTTPS) | 5683 (CoAP) / 5684 (DTLS) |

---

# บทที่ 2: การพัฒนา ESP-IDF MQTT Client และการติดตั้ง Broker

### 2.1 การติดตั้งและคอนฟิก Mosquitto Broker บน Windows/Linux
**Eclipse Mosquitto** เป็นโอเพนซอร์สโบรกเกอร์มาตรฐานสากล ตั้งแต่เวอร์ชัน 2.0 ขึ้นไป มีนโยบายความปลอดภัยเข้มงวด (ปิด Anonymous และฟังแค่ localhost) จึงต้องเขียนไฟล์ `mosquitto.conf`:
```text
listener 1883 0.0.0.0
allow_anonymous true
log_type all
```
* **คำสั่งรัน Verbose:** `mosquitto.exe -c mosquitto.conf -v`
* **คำสั่ง CLI สำคัญ:**
  * `mosquitto_sub -h <IP> -p 1883 -t "iot/#" -v`
  * `mosquitto_pub -h <IP> -p 1883 -t "iot/led" -m "ON" -q 1 -r`

---

### 2.2 โครงสร้างคอมโพเนนต์ esp-mqtt บน ESP-IDF
บน ESP-IDF ใช้ไลบรารี `esp-mqtt` (`mqtt_client.h`) การกำหนดค่าใช้โครงสร้าง `esp_mqtt_client_config_t`:
```c
esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.uri = "mqtt://192.168.1.100:1883",
    .credentials.client_id = "esp32_sensor_01",
    .session.keepalive = 60,
    .session.last_will = {
        .topic = "iot/node01/status",
        .msg = "offline",
        .qos = 1,
        .retain = 1
    }
};
```

---

### 2.3 วงจรชีวิต Event Loop และ Event ID สำคัญ
`esp-mqtt` ขับเคลื่อนด้วย Event-driven Architecture ผ่านฟังก์ชัน Callback:
* `MQTT_EVENT_CONNECTED`: เชื่อมต่อ Broker สำเร็จ $\rightarrow$ **จังหวะนี้ต้องสั่ง `esp_mqtt_client_subscribe()` และส่งสถานะ Online ทันที**
* `MQTT_EVENT_DATA`: มีข้อความใหม่เข้ามา หัวข้ออยู่ที่ `event->topic` (ยาว `event->topic_len`) และข้อมูลอยู่ที่ `event->data` (ยาว `event->data_len`)
* `MQTT_EVENT_DISCONNECTED`: ลิงก์ขาดหาย (ตัวไลบรารีมี State Machine ทำการ Auto-reconnect ให้เอง)
* `MQTT_EVENT_PUBLISHED`: ส่งข้อมูลสำเร็จ (ได้รับ ACK จาก Broker)

---

### 2.4 FreeRTOS Task, cJSON Serializer และการจัดการ Memory
* ในการส่งข้อมูลเป็นจังหวะต่อเนื่อง จะแยกเป็น FreeRTOS Task (`telemetry_task`) ทำงานวนลูปผ่าน `vTaskDelay()`
* การสร้างสตริง JSON ใช้ไลบรารี `cJSON`:
  * `cJSON_CreateObject()` และ `cJSON_AddNumberToObject()`
  * แปลงเป็นสตริงด้วย `cJSON_PrintUnformatted(root)`
* **⚠️ กฎเหล็กการป้องกัน Memory Leak:** เนื่องจาก cJSON จองพื้นที่บน Heap เมื่อ Publish ข้อมูลเสร็จ **ต้องเรียก `free(json_str)` และ `cJSON_Delete(root)` ทุกรอบเสมอ** มิฉะนั้น Heap จะหมดลง (Heap Exhaustion) จน ESP32 แครชรีสตาร์ต

---

### 2.5 ข้อควรระวังระดับปฏิบัติการ (Gotchas)
1. **ปัญหา Split-Brain Broker บน Windows:** ตัวติดตั้ง Mosquitto บน Windows จะสร้าง Background Windows Service ที่ผูกกับ `127.0.0.1` เอาไว้ หากเปิด PowerShell รัน `mosquitto -v` อีกตัว จะกลายเป็น Broker 2 ตัวซ้อนกัน ทำให้ ESP32 ในวงแลนมองไม่เห็นข้อความจาก Terminal **วิธีแก้:** ต้องสั่ง `Stop-Service mosquitto` ก่อนเสมอ
2. **Buffer Null-Terminator (`\0`):** ค่า `event->data` และ `event->topic` ใน `MQTT_EVENT_DATA` **ไม่มี `\0` ปิดท้ายสตริง** ห้ามใช้ `printf("%s", event->data)` หรือ `strcmp()` ตรงๆ ให้ใช้ `printf("%.*s", event->data_len, event->data)` หรือ `strncmp()`

---

# บทที่ 3: ความปลอดภัย MQTT, โครงสร้างพื้นฐาน TLS และ mTLS

### 3.1 ช่องโหว่ของ Plaintext MQTT และหลักการสามเหลี่ยม CIA
Plaintext MQTT (พอร์ต 1883) ส่งข้อมูลดิบผ่านเครือข่ายอินเทอร์เน็ต เสี่ยงต่อ:
1. **Eavesdropping (ดักฟัง):** โปรแกรม Packet Sniffer (Wireshark) ดักอ่าน Payload และรหัสผ่านใน `CONNECT` ได้
2. **Man-in-the-Middle (MITM / ปลอมแปลง):** แก้ไขค่าคำสั่งเปิดปิดอุปกรณ์กลางทาง
3. **Rogue Devices (สวมรอย):** เครื่องแปลกปลอมยิงข้อความป่วนระบบ
* **เสาหลักความปลอดภัย (The CIA Triad):**
  * **Confidentiality (การรักษาความลับ):** ข้อมูลต้องถูกเข้ารหัส
  * **Integrity (ความถูกต้องสมบูรณ์):** ข้อมูลห้ามถูกแก้ไขระหว่างทาง
  * **Availability (ความพร้อมใช้งาน):** ระบบต้องไม่ล่มจากการโจมตี
  * **Authentication (การยืนยันตัวตน):** ต้องพิสูจน์ได้ว่าคู่สนทนาเป็นตัวจริง

---

### 3.2 การเข้ารหัสแบบสมมาตร vs อสมมาตร และ Hybrid TLS
* **Symmetric Encryption (กุญแจสมมาตร เช่น AES):** ใช้กุญแจดอกเดียวกันเข้ารหัสและถอดรหัส คำนวณเร็วมาก แต่มีจุดตายเรื่อง **"Key Distribution Problem"** (จะส่งกุญแจให้กันอย่างไรไม่ให้ถูกดักจับ)
* **Asymmetric Encryption (กุญแจอสมมาตร เช่น RSA, ECC):** ประกอบด้วย **กุญแจคู่ (Key-Pair)**:
  * **Public Key (กุญแจสาธารณะ):** แจกจ่ายได้ ใช้สำหรับ "เข้ารหัสข้อมูล" หรือ "ตรวจลายมือชื่อดิจิทัล"
  * **Private Key (กุญแจส่วนตัว):** เก็บเป็นความลับสูงสุด ใช้สำหรับ "ถอดรหัสข้อมูล" หรือ "เซ็นลายมือชื่อดิจิทัล"
* **Hybrid TLS Handshake:** รวมข้อดีของทั้งสองระบบเข้าด้วยกัน:
  1. ใช้ **Asymmetric Key** ยืนยันตัวตนและเข้ารหัสแลกเปลี่ยนกุญแจชั่วคราว (**Session Key**)
  2. เมื่อตกลง Session Key สำเร็จ จะสลับไปใช้ **Symmetric Key (AES)** ส่งข้อมูลจริงเพื่อความรวดเร็ว

---

### 3.3 โครงสร้าง Public Key Infrastructure (PKI) และใบรับรอง X.509
* **ปัญหา:** จะรู้ได้อย่างไรว่า Public Key ที่ได้รับเป็นของ Broker จริง ไม่ใช่แฮกเกอร์ปลอมตัวมา?
* **ทางแก้:** **ใบรับรองดิจิทัล (Digital Certificate มาตรฐาน X.509)** เสมือนพาสปอร์ตดิจิทัลที่ระบุ Subject, Public Key, Issuer, วันหมดอายุ และมี **Digital Signature** ของ **Certificate Authority (Root CA)** ประทับตราการันตีความถูกต้อง
* **Chain of Trust:** หาก Client เชื่อถือ Root CA (`ca.crt`) และ Broker มีใบรับรองที่ Root CA นั้นเซ็นรับรอง (`server.crt`) ไคลเอนต์ก็จะเชื่อถือ Broker นั้นทันที

---

### 3.4 ลำดับคำสั่ง OpenSSL ในการออกใบรับรอง (Root CA, Server, Client)
```bash
# 1. สร้าง Root CA (Self-signed 10 ปี)
openssl genrsa -out ca.key 2048
openssl req -out ca.csr -key ca.key -new -subj "/C=TH/ST=Bangkok/O=IoT Lab/CN=IoT Root CA"
openssl x509 -req -in ca.csr -out ca.crt -sha256 -days 3650 -signkey ca.key

# 2. สร้าง Server Cert (CN ต้องเป็น IP/Domain ของ Broker)
openssl genrsa -out server.key 2048
openssl req -out server.csr -key server.key -new -subj "/C=TH/ST=Bangkok/O=IoT Broker/CN=192.168.1.100"
openssl x509 -req -in server.csr -out server.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650

# 3. สร้าง Client Cert (สำหรับ ESP32)
openssl genrsa -out client.key 2048
openssl req -out client.csr -key client.key -new -subj "/C=TH/ST=Bangkok/O=IoT Node/CN=esp32-node01"
openssl x509 -req -in client.csr -out client.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650
```

---

### 3.5 กฎเหล็กของ Common Name (CN) และการป้องกัน Hostname Mismatch
ในขั้นตอนสร้าง Server CSR ค่า **Common Name (CN)** จะต้องตรงกับ **IP Address หรือ Hostname/Domain ที่ Client ใช้เชื่อมต่อจริง** เพื่อให้ผ่านกระบวนการ **Server Name Indication (SNI) / Hostname Verification** หากระบุไม่ตรงกัน Client จะตัดการเชื่อมต่อทันทีเพราะคิดว่ากำลังคุยกับเซิร์ฟเวอร์สวมรอย

---

### 3.6 การฝังใบรับรองผ่าน CMake EMBED_TXTFILES บน ESP-IDF
ไม่ต้องเขียนโค้ดอ่านไฟล์จาก Flash โดยวางไฟล์ไว้ที่ `main/certs/` แล้วระบุใน `main/CMakeLists.txt`:
```cmake
idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    EMBED_TXTFILES "certs/ca.crt" "certs/client.crt" "certs/client.key"
)
```
ในโค้ด C สามารถอ้างอิงผ่านสัญลักษณ์ Assembler:
```c
extern const uint8_t ca_crt_start[]     asm("_binary_ca_crt_start");
extern const uint8_t client_crt_start[] asm("_binary_client_crt_start");
extern const uint8_t client_key_start[] asm("_binary_client_key_start");
```

---

### 3.7 One-Way TLS เทียบกับ Mutual TLS (mTLS)
```
[One-Way TLS (เช่น HTTPS ปกติ)]         [Mutual TLS / mTLS (MQTTS Port 8883)]
Client                Server            Client                Server
  |                      |                |                      |
  |<--- ส่ง server.crt ---|                |<--- ส่ง server.crt ---|  (Client ตรวจ Server)
  | (ตรวจว่า Server จริง)  |                |                      |
  |--- ส่ง Key เข้ารหัส -->|                |---- ส่ง client.crt ->|  (Server บังคับตรวจ Client)
                                          |                      |   ผ่าน require_certificate true
```
* **ทำไม IoT อุตสาหกรรมต้องใช้ mTLS:**
  1. ไม่ต้องเก็บรหัสผ่าน Plaintext ในอุปกรณ์
  2. อุปกรณ์ปลอมแปลง (Cloned Device) ที่ไม่มี Private Key ที่ CA เซ็น จะถูกเตะออกจากเครือข่ายทันทีที่ระดับ TLS Handshake
  3. สามารถเพิกถอนสิทธิ์รายตัว (Device Revocation) ได้ทันที

---

### 3.8 การรักษาความปลอดภัยฮาร์ดแวร์ ESP32
หากคนร้ายขโมยเฟิร์มแวร์ `.bin` ไป จะสามารถสกัด `client.key` ออกมาได้ จึงต้องใช้ระบบป้องกันระดับฮาร์ดแวร์:
1. **Flash Encryption:** เข้ารหัสชิปแฟลชภายนอกด้วยฮาร์ดแวร์ AES-256 ป้องกันการ Dump ข้อมูล
2. **Secure Boot:** ตรวจลายเซ็นดิจิทัลของเฟิร์มแวร์ก่อนบูต ป้องกันการแฟลชโค้ดแปลกปลอม
3. **Digital Signature (DS) Peripheral:** ฮาร์ดแวร์ความปลอดภัยพิเศษใน ESP32 ที่ประมวลผล Private Key ภายในวงจรรวมโดยตรง ซอฟต์แวร์ไม่สามารถอ่านค่า Key ออกมาได้

---

# บทที่ 4: แพลตฟอร์มคลาวด์ ESP RainMaker Ecosystem

### 4.1 สถาปัตยกรรมและ 4 องค์ประกอบหลักของ ESP RainMaker
ESP RainMaker เป็นโซลูชันแบบ End-to-End จาก Espressif Systems ที่ผสานงานคลาวด์ครบวงจร:
1. **Device Firmware:** สแตก ESP-IDF + RainMaker SDK เชื่อมต่อคลาวด์ผ่าน MQTTS
2. **Cloud Backend:** สถาปัตยกรรม Serverless บน **Amazon Web Services (AWS)** (AWS IoT Core, AWS Cognito, AWS DynamoDB, AWS Lambda)
3. **Phone Application:** แอป iOS/Android มีระบบ Dynamic UI สร้างหน้าจอสั่งงานตามประเภทอุปกรณ์อัตโนมัติ
4. **Command Line Interface (CLI):** เครื่องมือ Python `rainmaker.py` สำหรับดีบักและสั่งงานจากคอมพิวเตอร์

---

### 4.2 แบบจำลองข้อมูลอุปกรณ์ (Data Model: Node, Device, Param, Service)
โครงสร้างข้อมูลจัดเป็นลำดับชั้น 4 ชั้น:
* **Node (โหนด):** ตัวแทนของชิปไมโครคอนโทรลเลอร์ ESP32 หนึ่งตัว (มี Node ID เฉพาะตัว)
* **Device (อุปกรณ์ตรรกะ):** ฟังก์ชันเครื่องใช้ไฟฟ้าภายใน Node (เช่น โหนดหนึ่งมี 2 อุปกรณ์: หลอดไฟ และ พัดลม)
* **Parameter (พารามิเตอร์):** คุณลักษณะที่ปรับค่าได้ เช่น Power (boolean), Brightness (integer 0–100), Hue (0–360)
* **Service (บริการส่วนกลาง):** บริการระดับระบบ เช่น Time Service (SNTP), Schedule Service (ตั้งเวลา), OTA Service (อัปเกรดเฟิร์มแวร์)

---

### 4.3 กลไกการขอรับสิทธิ์และการติดตั้งใบรับรอง (Claiming Mechanisms)
การนำอุปกรณ์เข้าสู่ระบบ AWS IoT Core ต้องใช้ X.509 Certificate:
1. **Assisted Claiming:** **เหมาะกับงานทดลองและ Prototype** บอร์ดไม่มีใบรับรองติดมา แต่ขณะเปิดแอปมือถือจับคู่ผ่านบลูทูธ (BLE) แอปจะไปขอใบรับรองจาก RainMaker Cloud แล้วยิงผ่าน BLE มาแฟลชลง NVS ของ ESP32 อัตโนมัติ
2. **Self Claiming:** ตัวบอร์ดที่มีเน็ตจะร้องขอใบรับรองจาก Espressif Server โดยตรง
3. **Host-driven / Factory Claiming:** เบิร์นใบรับรองลงพาร์ติชัน `fctry` มาจากโรงงานผลิต เหมาะสำหรับ Mass Production

---

### 4.4 การทำงานของ Write Callback และการซิงโครไนซ์สถานะ Device Shadow
เมื่อผู้ใช้กดสวิตช์ในแอปมือถือ คำสั่งจะวิ่งผ่าน AWS IoT ลงมายังฟังก์ชัน `write_cb`:
```c
static esp_err_t write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
                          const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    if (strcmp(esp_rmaker_param_get_name(param), ESP_RMAKER_DEF_POWER_NAME) == 0) {
        gpio_set_level(LED_PIN, val.val.b ? 1 : 0);
        // สำคัญมาก: ต้องรายงานสถานะจริงกลับไปอัปเดต Device Shadow บน Cloud ทันที
        esp_rmaker_param_update_and_report(param, val);
    }
    return ESP_OK;
}
```
> **ทำไมต้องเรียก `esp_rmaker_param_update_and_report()`:** เพื่อยืนยัน Reported State กลับไปยัง Cloud Shadow มิฉะนั้นหน้าจอแอปมือถือจะแสดงผลไม่ตรงกับสถานะจริงของฮาร์ดแวร์

---

### 4.5 การผสานงานระบบเสียง Google Assistant / Alexa (Cloud-to-Cloud Integration)
ESP RainMaker รองรับการสั่งงานด้วยเสียงโดย **ไม่ต้องแก้โค้ดบนบอร์ด ESP32 เลยแม้แต่บรรทัดเดียว**:
* ทำงานผ่าน **Cloud-to-Cloud Integration**: ผู้ใช้ผูกบัญชี (Account Linking) ระหว่าง Google Home/Alexa กับ ESP RainMaker
* ข้อมูลมาตรฐาน (เช่น Device: Lightbulb, Param: Power) จะถูกแมปเข้ากับ Smart Home Traits ของ Google/Amazon โดยอัตโนมัติ คำสั่งเสียงจะวิ่งจาก Google/Amazon Cloud มายัง RainMaker Cloud แล้วแปลงเป็น MQTT Shadow ส่งลงมาที่บอร์ดเหมือนเดิม

---

# บทที่ 5: สรุปจุดเน้นการทดลองปฏิบัติการ 4 ใบงาน (Lab Essentials)

### 5.1 Lab 11.1: Mosquitto Broker and MQTT Fundamentals
* **สาระการทดลอง:** รัน Mosquitto Broker แบบ Verbose (`-v`), ใช้ CLI `mosquitto_sub` และ `mosquitto_pub` ทดสอบ Pub/Sub, Wildcards (`+`, `#`), QoS 0/1/2, Retained Message (`-r`), และ LWT
* **จุดสังเกตใน Log:**
  * QoS 0: มีเฉพาะแพ็กเก็ต `PUBLISH`
  * QoS 1: มีแพ็กเก็ต `PUBLISH` และตอบรับด้วย `PUBACK` (มี Mid Packet ID)
  * QoS 2: มีการจับมือ 4 ขั้นตอน `PUBLISH` $\rightarrow$ `PUBREC` $\rightarrow$ `PUBREL` $\rightarrow$ `PUBCOMP`
* **พินัยกรรม LWT:** ส่งเมื่อเชื่อมต่อหลุดกะทันหัน แต่ถ้าส่ง `DISCONNECT` อย่างถูกต้อง LWT จะ **ไม่ทำงาน**

---

### 5.2 Lab 11.2: ESP32 MQTT Client Telemetry and Control
* **สาระการทดลอง:** เขียนเฟิร์มแวร์ ESP-IDF ต่อวงจร LED (GPIO 2/8) และ Potentiometer (GPIO 34/0) ส่งข้อมูล Telemetry แบบ JSON ทุกๆ 4 วินาที และรับคำสั่งเปิด-ปิดไฟ
* **จุดเน้น:**
  * เชื่อมต่อ Wi-Fi สำเร็จก่อนเริ่มสร้าง MQTT Client
  * ส่งสถานะ `iot/node01/status = "online"` แบบ `retain = 1` ควบคู่กับ LWT `"offline"`
  * จัดการคืนหน่วยความจำ cJSON ด้วย `free(json_str)` และ `cJSON_Delete(root)` ป้องกัน Memory Leak

---

### 5.3 Lab 11.3: MQTTS TLS Mutual Authentication Security
* **สาระการทดลอง:** สร้าง Local CA และใบรับรองด้วย OpenSSL, รัน Mosquitto พอร์ต 8883 บังคับ `require_certificate true`, ฝังไฟล์ใบรับรองลงบอร์ดด้วย `EMBED_TXTFILES`
* **จุดเน้น:**
  * Common Name (CN) ของ Server Certificate **ต้องเป็นหมายเลข IP ของคอมพิวเตอร์ที่รัน Broker เท่านั้น**
  * ทดสอบปฏิเสธผู้บุกรุก: หาก Client ไม่ส่ง `client.crt` และ `client.key` โบรกเกอร์จะฟ้อง `peer did not return a certificate` และตัดการเชื่อมต่อทันที

---

### 5.4 Lab 11.4: ESP RainMaker Cloud IoT and Smart Device
* **สาระการทดลอง:** พัฒนา Smart Light บน ESP-IDF เชื่อมต่อ AWS IoT ผ่าน RainMaker SDK, ทำ Wi-Fi Provisioning ผ่านบลูทูธ (BLE), สแกน QR Code ด้วยแอป RainMaker บนมือถือ และทดสอบสั่งงานผ่านเครือข่าย 4G/5G
* **จุดเน้น:**
  * Assisted Claiming ติดตั้งใบรับรองผ่านบลูทูธโดยไม่ต้องแฟลชกุญแจเอง
  * ทดสอบสั่งงานข้ามโครงข่ายอินเทอร์เน็ตจริงโดยปิด Wi-Fi บนมือถือแล้วใช้สัญญาณมือถือสั่งเปิด-ปิดไฟ

---

# บทที่ 6: คลังคำศัพท์สำคัญและการนิยาม (Key Terminology)

| คำศัพท์ (Term) | ความหมายเชิงลึกสำหรับการสอบ |
| :--- | :--- |
| **MQTT** | โปรโตคอลน้ำหนักเบาแบบ Publish/Subscribe ทำงานบน TCP/IP ออกแบบสำหรับเครือข่ายแบนด์วิดท์จำกัด |
| **Broker** | เซิร์ฟเวอร์ส่วนกลางทำหน้าที่รับและกระจายข้อความไปยังผู้บอกรับ (Subscriber) |
| **Wildcards (`+` และ `#`)** | สัญลักษณ์คัดกรองหัวข้อตอน Subscribe โดย `+` แทน 1 ระดับชั้น ส่วน `#` แทนทุกระดับชั้นถัดไปและต้องอยู่ท้ายสุด |
| **QoS (Quality of Service)** | ระดับการันตีการส่งข้อความ: QoS 0 (ไม่การันตี), QoS 1 (ถึงอย่างน้อย 1 ครั้ง มี ACK), QoS 2 (ถึงครั้งเดียวแน่นอน 4-way) |
| **Retained Message** | ข้อความที่ Broker เก็บไว้ล่าสุด เมื่อมี Subscriber ใหม่เข้ามาจะได้รับทันทีโดยไม่ต้องรอรอบส่งใหม่ |
| **LWT (Last Will and Testament)** | พินัยกรรมฉุกเฉินที่ Broker จะ Publish แทนให้ทันทีหากตรวจพบว่าอุปกรณ์หลุดการเชื่อมต่อผิดปกติ |
| **Keep-Alive / PINGREQ** | สัญญาณชีพตรวจสอบการเชื่อมต่อ หากไม่มีการส่งข้อมูลต้องส่งแพ็กเก็ต PING ไปสะกิดไม่ให้ TCP Socket ถูกตัด |
| **MQTTS** | โปรโตคอล MQTT ที่ห่อหุ้มด้วยการเข้ารหัสความปลอดภัย TLS ผ่านพอร์ตมาตรฐาน 8883 |
| **PKI & X.509** | โครงสร้างพื้นฐานกุญแจสาธารณะ และมาตรฐานสากลของไฟล์ใบรับรองดิจิทัล |
| **Root CA** | หน่วยงานรับรองความถูกต้องสูงสุดที่ทำหน้าที่เซ็นออกใบรับรอง เป็นจุดเริ่มต้นของ Chain of Trust |
| **mTLS (Mutual TLS)** | การยืนยันสิทธิ์สองทาง ทั้ง Server และ Client ต่างต้องแสดงใบรับรองดิจิทัลที่ผ่านการเซ็นจาก CA เดียวกัน |
| **Common Name (CN)** | ฟิลด์ในใบรับรอง Server ที่ต้องระบุให้ตรงกับ IP หรือ Hostname ของเซิร์ฟเวอร์เพื่อป้องกัน Hostname Mismatch |
| **Flash Encryption & Secure Boot** | กลไกความปลอดภัยระดับฮาร์ดแวร์ของ ESP32 สำหรับเข้ารหัสหน่วยความจำและตรวจลายเซ็นโค้ดก่อนบูต |
| **ESP RainMaker** | แพลตฟอร์ม Cloud IoT แบบครบวงจรของ Espressif Systems ที่สร้างบน AWS Serverless มีแอปมือถือและ CLI พร้อมใช้ |
| **Node, Device, Param** | แบบจำลองข้อมูลของ RainMaker: Node คือบอร์ด, Device คืออุปกรณ์ตรรกะ, Param คือคุณลักษณะที่ปรับค่าได้ |
| **Assisted Claiming** | กระบวนการที่แอปบนมือถือช่วยขอใบรับรองจาก Cloud แล้วส่งต่อให้ ESP32 ผ่าน BLE ในการตั้งค่าครั้งแรก |
| **Device Shadow** | กลไกเสมือนบนคลาวด์ที่คอยบันทึกสถานะล่าสุดของอุปกรณ์ ทำให้อุปกรณ์และแอปมือถือซิงก์ข้อมูลตรงกันแบบ Real-time |
