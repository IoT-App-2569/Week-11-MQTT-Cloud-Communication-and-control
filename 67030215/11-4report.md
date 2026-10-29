# ใบงานการทดลองที่ 11.4
### การสร้างอุปกรณ์ Smart Light สู่ระบบคลาวด์ด้วย ESP RainMaker SDK, BLE Provisioning และการควบคุมผ่านสมาร์ตโฟน

> [!NOTE] **คำชี้แจง**
> ในใบงานนี้นักศึกษาจะได้สร้างผลิตภัณฑ์ IoT สู่ระบบคลาวด์เชิงพาณิชย์จริงโดยใช้แพลตฟอร์ม **ESP RainMaker** ของ Espressif Systems โดยจะได้เขียนโปรแกรมสร้าง **Node** และ **Device (Smart Light)** พร้อมพารามิเตอร์ **Power** และ **Brightness** จากนั้นทำการเปิดใช้งานระบบ **BLE Wi-Fi Provisioning**, การขอรับใบรับรองผ่านระบบ **Assisted Claiming** สแกนเชื่อมต่อผ่านแอปพลิเคชัน **ESP RainMaker บนสมาร์ตโฟน** และควบคุมอุปกรณ์ข้ามเครือข่ายอินเทอร์เน็ตได้จากทุกที่ทั่วโลก

---

## 1. วัตถุประสงค์การทดลอง
1. เข้าใจโครงสร้างและการทำงานของแพลตฟอร์ม ESP RainMaker บนสถาปัตยกรรม ESP-IDF
2. สามารถสร้าง Node, Device, และ Parameters (Power, Brightness) ผ่าน RainMaker Core API ได้
3. สามารถเขียน Write Callback เพื่อรับคำสั่งจากคลาวด์มาปรับระดับสถานะ GPIO ของ LED และรายงานสถานะกลับ (Report State)
4. สามารถเปิดใช้งานบริการระดับระบบ ได้แก่ บริการซิงก์เวลา SNTP และการตั้งเวลาทำงาน (Schedule Service) ได้
5. สามารถทำกระบวนการ Provisioning และ Assisted Claiming ผ่านแอปพลิเคชันสมาร์ตโฟนด้วยบลูทูธ (BLE) ได้สำเร็จ
6. สามารถทดสอบส่งคำสั่งควบคุมผ่านแอปพลิเคชันมือถือ และเครื่องมือบรรทัดคำสั่ง ESP RainMaker CLI ได้

---

## 2. อุปกรณ์และเครื่องมือที่ใช้
* บอร์ดไมโครคอนโทรลเลอร์ **ESP32-C3** หรือ **ESP32**
* หลอดไฟ LED พร้อมตัวต้านทาน 220Ω ต่อเข้ากับ **GPIO 8** (ESP32-C3) หรือ **GPIO 2** (ESP32)
* สายเชื่อมต่อ USB Data Cable
* เครื่องคอมพิวเตอร์พร้อมระบบพัฒนา **ESP-IDF v5.x / v6.x**
* สมาร์ตโฟน (Android หรือ iOS) ที่ติดตั้งแอปพลิเคชัน **ESP RainMaker** และเปิดบลูทูธ/Wi-Fi ไว้
* เราเตอร์ Wi-Fi (2.4 GHz) ที่เชื่อมต่อออกสู่อินเทอร์เน็ตได้จริง

---


### กิจกรรมที่ 11-4.3 การคอมไพล์ แฟลช และสังเกตการสร้าง QR Code

สั่งคอมไพล์และแฟลชโปรแกรม
```powershell
idf.py build
idf.py -p COMx flash monitor
```

**ผลลัพธ์ใน Serial Monitor**
* บอร์ดจะพิมพ์ **QR Code ในรูปแบบตัวอักษร ASCII** ออกมาทางหน้าจอ พร้อม URL สำหรับเปิดดูรูปภาพ QR Code
* บอร์ดจะเริ่มประกาศสัญญาณบลูทูธ (BLE Advertising) เพื่อรอรับการเชื่อมต่อ

---

### กิจกรรมที่ 11-4.4 การเชื่อมต่อผ่านแอปพลิเคชัน ESP RainMaker บนมือถือ

<p align="center">
  <img src="Images/image13.png" width="280">
  &nbsp;&nbsp;&nbsp;&nbsp;
  <img src="Images/image14.png" width="280">
  &nbsp;&nbsp;&nbsp;&nbsp;
  <img src="Images/image15.png" width="280">
</p>
<p align="center"><b>รูปที่ 11.13</b> ขั้นตอนการเปิดใช้งาน Assisted Claiming และการจับคู่อุปกรณ์บนโทรศัพท์มือถือ</p>

1. **เข้าสู่ระบบ** เปิดแอปพลิเคชัน **ESP RainMaker** บนโทรศัพท์มือถือ เข้าสู่ระบบด้วยบัญชีที่ลงทะเบียนไว้
2. **เพิ่มอุปกรณ์** กดปุ่มเครื่องหมายบวก (`+ Add Device`) บนหน้าจอแอป
3. **สแกน QR Code** นำกล้องสมาร์ตโฟนสแกน QR Code ที่ปรากฏบน Serial Monitor
4. **กระบวนการ Assisted Claiming**
   * หากบอร์ดยังไม่มีใบรับรอง แอปจะขึ้นหน้าต่างยืนยันการขอรับ Certificate ให้กดปุ่มยืนยัน
   * แอปจะดึงใบรับรองจาก RainMaker Cloud แล้วส่งให้ ESP32 ผ่านช่องทาง BLE
5. **กรอกรหัส Wi-Fi** เลือกชื่อ SSID เครือข่าย Wi-Fi 2.4 GHz ในบ้านและกรอกรหัสผ่าน
6. **เสร็จสิ้นการผูกอุปกรณ์** บอร์ดจะเชื่อมต่อ Wi-Fi และผูกตัวเองเข้ากับบัญชีผู้ใช้ แอปจะแสดงการ์ด **"Living Room Light"** ขึ้นมาบนหน้าจอ

---

## 4. การทดสอบและการตรวจสอบผลลัพธ์ (Testing & Verification)

### 1. การควบคุมเปิด-ปิดผ่าน Mobile App
* กดสวิตช์ Power บนหน้าจอแอป สังเกตว่าหลอดไฟ LED บนบอร์ดเปิดและปิดทันทีตามการสั่งการ
* ทดลองเลื่อนแถบสไลเดอร์ Brightness สังเกตข้อความ Log บน Serial Monitor ว่าได้รับค่าความสว่างใหม่กี่เปอร์เซ็นต์

### 2. การควบคุมระยะไกลข้ามอินเทอร์เน็ต (Remote 4G/5G Test)
* **ปิดสัญญาณ Wi-Fi บนสมาร์ตโฟน** แล้วเปิดใช้งานเฉพาะ **สัญญาณอินเทอร์เน็ตมือถือ (4G/5G)**
* กดสั่งเปิด-ปิดหลอดไฟอีกครั้ง สังเกตว่าบอร์ด ESP32 ที่บ้านยังคงตอบสนองต่อคำสั่งได้อย่างรวดเร็ว แสดงถึงพลังของสถาปัตยกรรม Cloud Control

## ผลการทดลอง

### กิจกรรม 11-4.1 การสร้างโปรเจกต์และติดตั้ง Dependency

ค่าที่ใช้จริงในการทดลอง
- **ESP-IDF:** v6.0.2
- **Target:** ESP32
- **Component หลัก:** `espressif/esp_rainmaker` ^1.17.0
- **Component Wi-Fi Provisioning:** `espressif/rmaker_app_network` ^1.4.4
- **Flash / Partition:** 4 MB, `ota_0` + `ota_1` (1600K) และ `fctry` ที่ 0x340000

### กิจกรรม 11-4.3 การแฟลชและ QR Code

<img width="853" height="587" alt="image" src="https://github.com/user-attachments/assets/d9417625-9362-47c7-bfda-ab5830601e66" />


บอร์ดพิมพ์ QR Code พร้อมข้อมูล `{"ver":"v1","name":"PROV_51681e","username":"wifiprov","pop":"6c38a8d3","transport":"ble"}` และเริ่ม BLE Advertising รอการจับคู่ โดยมี Node ID คือ `SFyfCRezgekerpXw4se7yf`

### กิจกรรม 11-4.4 การจับคู่ผ่านแอป ESP RainMaker

<img width="284" height="621" alt="image" src="https://github.com/user-attachments/assets/73e0774a-adfc-4d39-ae6a-95a00acc2b41" />

ลำดับเหตุการณ์จาก Serial Monitor
1. **แอปเชื่อมต่อ BLE และสร้าง Session เข้ารหัส (Security 2)** เห็น Log `app_network: Secured session established!`
2. **ยืนยันความเป็นเจ้าของ Node (Challenge-Response)** เห็น Log `esp_rmaker_chal_resp: Challenge-Response handler completed successfully`
3. **รับข้อมูล Wi-Fi จากแอปทาง BLE** เห็น Log `app_wifi: Received Wi-Fi credentials SSID: IPK_`
4. **เชื่อมต่อ Wi-Fi** เห็น Log `sta ip: 172.20.10.3` และ `Provisioning successful`
5. **เชื่อมต่อคลาวด์ AWS IoT Core ผ่าน MQTTS** เห็น Log `Connecting to a1p72mufdu6064-ats.iot.us-east-1.amazonaws.com` ตามด้วย `Certificate validated` และ `MQTT Connected`
6. **ผูก Node กับบัญชีผู้ใช้** เห็น Log `User node mapping done`
7. **รายงานสถานะเริ่มต้นขึ้นคลาวด์** เห็น Log `Reporting params (init): {"Living Room Light":{"Power":false,"Brightness":100},...}`
8. **ปิด BLE คืนหน่วยความจำ** เห็น Log `Provisioning stopped` และ `BTDM memory released`

---

## การทดสอบและการตรวจสอบผลลัพธ์

### 1. การควบคุมผ่าน Mobile App

<img width="868" height="1887" alt="image" src="https://github.com/user-attachments/assets/fd403b5f-6e38-456c-89af-7d8d4e419a96" />

<img width="1431" height="1099" alt="image" src="https://github.com/user-attachments/assets/f3581e0b-f704-49c3-88d8-b72e331a05c8" />


ผลการควบคุมจากแอป
- **เปิดสวิตช์ Power:** Log แสดง `Received params: {"Living Room Light":{"Power":true}}` ตามด้วย `Changing LED state to: ON` และ `Reporting params` ผลที่บอร์ดคือ LED ติด
- **ปิดสวิตช์ Power:** Log แสดง `Received params: {"Living Room Light":{"Power":false}}` ตามด้วย `Changing LED state to: OFF` และ `Reporting params` ผลที่บอร์ดคือ LED ดับ
- **เลื่อน Brightness:** Log แสดง `Changing Brightness to: ..%` บอร์ดรับค่าได้ แต่ความสว่างไม่เปลี่ยน เพราะโค้ดยังไม่ได้ใช้ LEDC PWM

### 2. การควบคุมระยะไกลข้ามอินเทอร์เน็ต

มือถือเชื่อมต่ออินเทอร์เน็ตผ่าน 4G และปิด Bluetooth ระหว่างทดสอบ ทุกคำสั่งแสดง `Request received via: Cloud` ยืนยันว่าคำสั่งเดินทางจากแอป → RainMaker Cloud (AWS IoT Core, us-east-1) → MQTT → ESP32 ไม่ได้ส่งตรงผ่านเครือข่ายท้องถิ่น

### 3. ปัญหาที่พบและการแก้ไข

**ปัญหาที่ 1: `add-dependency "espressif/esp-rainmaker"` ไม่พบ component** สาเหตุคือชื่อและเวอร์ชันในใบงานไม่ตรงกับ Component Registry ปัจจุบัน แก้โดยใช้ `espressif/esp_rainmaker` ^1.17.0

**ปัญหาที่ 2: ไม่พบ `app_wifi.h`** สาเหตุคือฟังก์ชัน Provisioning ถูกแยกเป็น component และเปลี่ยนชื่อ แก้โดยเพิ่ม `espressif/rmaker_app_network` แล้วใช้ `app_network.h`, `app_network_init()` และ `app_network_start()`

**ปัญหาที่ 3: ต้องกำหนด Partition Table เอง** สาเหตุคือ RainMaker ต้องใช้พื้นที่สำหรับ OTA และพาร์ติชัน `fctry` เก็บข้อมูลประจำเครื่อง แก้โดยสร้าง `partitions.csv` และ `sdkconfig.defaults` (Flash 4 MB, เปิด BLE แบบ NimBLE)

**ปัญหาที่ 4: แอปค้างที่ "Confirming Node Association" และขึ้น `ESPSessionError`** สาเหตุคือเกิด `***ERROR*** A stack overflow in task nimble_host` ขณะบอร์ดคำนวณลายเซ็น ECDSA ในขั้น Challenge-Response ทำให้บอร์ดรีบูต แก้โดยเพิ่ม `CONFIG_BT_NIMBLE_HOST_TASK_STACK_SIZE=8192` และตั้งค่าตามตัวอย่างทางการ เช่น `CONFIG_BTDM_CTRL_MODE_BLE_ONLY=y` และ `CONFIG_MBEDTLS_DYNAMIC_BUFFER=y`

**ปัญหาที่ 5: `NimBLE: ATT write: invalid handle`** สาเหตุคือ iPhone จำโครงสร้าง GATT ของบอร์ดจากการเชื่อมต่อครั้งก่อน (GATT Cache) แก้โดยรีสตาร์ต iPhone ก่อนจับคู่ใหม่

---

---

### 3. (ทางเลือกขั้นสูง) การทดสอบผ่าน RainMaker CLI บนคอมพิวเตอร์

เปิดเทอร์มินัลใหม่บนคอมพิวเตอร์

```powershell
# ติดตั้งเครื่องมือ CLI
pip install esp-rainmaker-cli

# เข้าสู่ระบบ
rainmaker.py login

# ดึงรายชื่ออุปกรณ์และ Node ID
rainmaker.py getnodes

# สั่งเปิดหลอดไฟผ่าน Command Line
rainmaker.py setparams --nodeid <YOUR_NODE_ID> '{"Living Room Light": {"Power": true}}'

# สั่งปิดหลอดไฟผ่าน Command Line
rainmaker.py setparams --nodeid <YOUR_NODE_ID> '{"Living Room Light": {"Power": false}}'
```

---

## 5. คำถามท้ายการทดลอง (Post-Lab Questions)

**1. ในสถาปัตยกรรม ESP RainMaker กระบวนการ Assisted Claiming ช่วยแก้ปัญหาเรื่องการติดตั้งใบรับรองความปลอดภัย (Security Certificates) บนอุปกรณ์ในขั้นตอนพัฒนาต้นแบบได้อย่างไร?**
```
ปกติอุปกรณ์ที่จะเชื่อมต่อคลาวด์แบบ mTLS ต้องมีใบรับรองและกุญแจส่วนตัวของตัวเอง ซึ่งต้องสร้างและฝังลงบอร์ดทีละตัว เป็นงานยุ่งยากสำหรับช่วงทำต้นแบบ Assisted Claiming ย้ายงานนี้ไปให้แอปมือถือทำแทน บอร์ดสร้างกุญแจส่วนตัวในตัวเอง แล้วแอปจะขอใบรับรองจาก RainMaker Cloud ในนามผู้ใช้ และส่งให้บอร์ดผ่าน BLE ตอนจับคู่อุปกรณ์ ผู้พัฒนาจึงไม่ต้องสร้าง CA ออกใบรับรอง หรือฝังไฟล์ลงเฟิร์มแวร์เอง และบอร์ดที่ยังไม่มีใบรับรองก็ใช้งานได้ทันทีเมื่อจับคู่ผ่านแอป ส่วนกุญแจส่วนตัวไม่ต้องออกจากบอร์ดเลย ปลอดภัยกว่าการฝังไฟล์กุญแจลงโค้ดในแล็บ 11.3
```
**2. เพราะเหตุใดฟังก์ชัน `write_cb` จึงต้องเรียกคำสั่ง `esp_rmaker_param_update_and_report()` ทุกครั้งที่มีการเปลี่ยนแปลงค่าพารามิเตอร์?**
```
เพราะ `write_cb` มีหน้าที่แค่รับคำสั่งจากคลาวด์มาสั่งงานฮาร์ดแวร์ เช่น เปิดปิด LED แต่ค่าพารามิเตอร์ที่เก็บไว้ในตัว Node และบนคลาวด์ยังเป็นค่าเดิมอยู่ ถ้าไม่เรียก `esp_rmaker_param_update_and_report()` สถานะในแอปจะไม่ตรงกับสถานะจริงของอุปกรณ์ เช่น LED ติดแล้วแต่แอปยังโชว์ว่าปิด หรือเมื่อเปิดแอปใหม่ก็จะเห็นค่าเก่า คำสั่งนี้จะอัปเดตค่าในตัว Node และรายงานขึ้นคลาวด์ในครั้งเดียว ทำให้ทุกอุปกรณ์ที่ควบคุมบัญชีเดียวกัน เช่น มือถือหลายเครื่อง หรือ Google Home เห็นสถานะตรงกันเสมอ
```
**3. เปรียบเทียบความแตกต่างระหว่างการพัฒนาอุปกรณ์ IoT ด้วย Plain MQTT บน Mosquitto เอง (ในแล็บ 11.2) กับการใช้ ESP RainMaker SDK (ในแล็บ 11.4) ในแง่ของความพยายามในการพัฒนา (Development Effort), ความปลอดภัย (Security), และการขยายระบบ (Scalability)**
```
ด้านความพยายามในการพัฒนา Plain MQTT ต้องติดตั้งและตั้งค่า Broker เอง ออกแบบ Topic และรูปแบบข้อความ JSON เอง และต้องทำแอปหรือแดชบอร์ดสำหรับควบคุมเองด้วย ส่วน RainMaker มีคลาวด์ แอปมือถือ และระบบจับคู่อุปกรณ์ให้พร้อม ผู้พัฒนาเขียนแค่ประกาศ Device กับ Parameter และ Callback ก็ควบคุมได้ทันที ด้านความปลอดภัย Plain MQTT ที่ตั้งค่าแบบพื้นฐานไม่เข้ารหัส ถ้าจะให้ปลอดภัยต้องทำ TLS และ mTLS เองเหมือนแล็บ 11.3 และดูแลใบรับรองเอง ขณะที่ RainMaker เชื่อมต่อด้วย MQTTS พร้อมใบรับรองรายอุปกรณ์และระบบผูกบัญชีผู้ใช้มาให้ตั้งแต่ต้น ด้านการขยายระบบ Broker ที่ดูแลเองต้องรับภาระเรื่องเซิร์ฟเวอร์ การรองรับผู้ใช้จำนวนมาก และการเข้าถึงจากภายนอกเครือข่ายเอง ส่วน RainMaker อยู่บนโครงสร้างคลาวด์ของ AWS ที่ขยายตามจำนวนอุปกรณ์ได้ และมี OTA กับ Schedule ให้ใช้ แต่แลกกับการพึ่งพาแพลตฟอร์มและความยืดหยุ่นในการปรับแต่งที่น้อยกว่า
```
**4. อธิบายขั้นตอนที่ทำให้ ESP RainMaker สามารถเชื่อมโยงการทำงานร่วมกับระบบสั่งงานด้วยเสียงของ Google Home หรือ Amazon Alexa ได้โดยไม่ต้องเขียนโค้ดเพิ่มเติมบนตัวบอร์ด ESP32**
```
การเชื่อมต่อทำที่ฝั่งคลาวด์ทั้งหมด บอร์ดไม่ต้องรู้จัก Google หรือ Alexa เลย ขั้นตอนคือ เปิดแอป Alexa หรือ Google Home แล้วเพิ่มบริการ ESP RainMaker จากนั้นล็อกอินด้วยบัญชี RainMaker เดียวกับที่ผูกอุปกรณ์ไว้ เพื่ออนุญาตให้เข้าถึงอุปกรณ์ เมื่อเชื่อมสำเร็จ แอปจะดึงรายการ Device ที่ประกาศไว้ใน Node มาแสดง เช่น Living Room Light โดยแปลงตามชนิดอุปกรณ์และพารามิเตอร์มาตรฐาน เช่น Power และ Brightness เป็นความสามารถที่ผู้ช่วยเสียงเข้าใจ ผู้ใช้สั่งว่า "เปิดไฟห้องนั่งเล่น" คำสั่งจะไปที่คลาวด์ของ Google หรือ Amazon ต่อไปที่ RainMaker Cloud แล้วส่งมายังบอร์ดทาง MQTT เป็นการเขียนพารามิเตอร์ตามปกติ `write_cb` เดิมที่เขียนไว้ก็ทำงานต่อได้เลย ไม่ต้องแก้โค้ดบนบอร์ด
```
