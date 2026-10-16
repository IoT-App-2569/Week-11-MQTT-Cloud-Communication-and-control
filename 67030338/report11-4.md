# ใบงานการทดลองที่ 11.4
### การสร้างอุปกรณ์ Smart Light สู่ระบบคลาวด์ด้วย ESP RainMaker SDK, BLE Provisioning และการควบคุมผ่านสมาร์ตโฟน

---

## ผลการทดลองและการบันทึกภาพหน้าจอ

### 1. ภาพหน้าจอ Serial Monitor ขณะสร้าง QR Code และเริ่ม BLE Provisioning
* บอร์ด ESP32 เริ่มต้นทำงาน ทำการอ่านค่า NVS และสร้าง QR Code (ASCII Art) พร้อม URL ทาง Serial Monitor เพื่อรอรับการจับคู่ผ่านสัญญาณ Bluetooth Low Energy (BLE Advertising)

*<img width="1256" height="557" alt="11-4-1" src="https://github.com/user-attachments/assets/3ee497e5-e974-41dc-a402-1afa27cc3eb5" />

---

### 2. ภาพหน้าจอการทำ Assisted Claiming และเชื่อมต่อ Wi-Fi บนแอป ESP RainMaker
* หน้าจอโทรศัพท์มือถือขณะสแกน QR Code, ระบบถามยืนยันการรับ Certificate ผ่าน Assisted Claiming, การเลือกเครือข่าย Wi-Fi 2.4 GHz และหน้า Dashboard ที่ปรากฏการ์ดอุปกรณ์ **"Living Room Light"**

<img width="1080" height="2400" alt="Screenshot_20261016_222923" src="https://github.com/user-attachments/assets/2983c8ee-213b-4986-9d1d-750a41f31803" />
<img width="1080" height="2400" alt="Screenshot_20261016_222416" src="https://github.com/user-attachments/assets/0481cf34-fc7c-4904-ad76-e455d805c90d" />

---

### 3. ภาพการทดสอบควบคุมหลอดไฟ LED บนบอร์ด และปรับความสว่าง (Power & Brightness)
* ภาพหลอดไฟ LED (GPIO 2) สว่างขึ้นเมื่อกดสวิตช์ Power ON ในแอปมือถือ และภาพ Serial Monitor ที่แสดง Callback Log การปรับค่า `Power` และ `Brightness`


<img width="1080" height="2400" alt="Screenshot_20261016_222947" src="https://github.com/user-attachments/assets/35f00283-498b-41ea-90f1-b6bf38656147" />
<img width="3072" height="4096" alt="IMG_20261016_222953" src="https://github.com/user-attachments/assets/dfa06ba3-3d53-4cef-846b-bee43a83e8fd" />


---

### 4. ภาพหน้าจอการทดสอบควบคุมระยะไกลข้ามอินเทอร์เน็ต (Remote 4G/5G Test)
* หน้าจอโทรศัพท์มือถือที่ **ปิด Wi-Fi แล้วเปิดเฉพาะสัญญาณ 4G/5G** และสั่งเปิด-ปิดหลอดไฟได้สำเร็จแบบ Real-time แสดงถึงการทำงานของ Cloud Backend ผ่าน AWS IoT Core

<img width="1080" height="2400" alt="Screenshot_20261016_222947" src="https://github.com/user-attachments/assets/f67fd996-1a4a-4d7a-9825-085701ebcf6a" />
<img width="1080" height="2400" alt="Screenshot_20261016_223019" src="https://github.com/user-attachments/assets/1c0969fb-6262-49db-898c-e57e6027ef0b" />


---

## คำถามท้ายการทดลอง (Post-Lab Questions)

### ข้อ 1: ในสถาปัตยกรรม ESP RainMaker กระบวนการ Assisted Claiming ช่วยแก้ปัญหาเรื่องการติดตั้งใบรับรองความปลอดภัย (Security Certificates) บนอุปกรณ์ในขั้นตอนพัฒนาต้นแบบได้อย่างไร?
* **คำตอบ:**
  1. **ปัญหาของการติดตั้งใบรับรองแบบดั้งเดิมในขั้นตอนพัฒนาต้นแบบ:**
     * ในระบบความปลอดภัยระดับ MQTTS / AWS IoT อุปกรณ์ทุกตัวต้องมีใบรับรองเฉพาะตัว (X.509 Device Certificate) และกุญแจส่วนตัว (Private Key) ซึ่งโดยทั่วไปนักพัฒนาจะต้องสร้างใบรับรองเอง (OpenSSL), แฟลชไฟล์ใบรับรองลงสู่ Flash Memory ทีละบอร์ด (เช่น พาร์ติชัน `fctry` หรือโค้ดแบบฝังไฟล์) ซึ่งเป็นขั้นตอนที่ซับซ้อน เสียเวลา และผิดพลาดได้ง่าย
  2. **กลไกการแก้ปัญหาของ Assisted Claiming:**
     * กระบวนการ **Assisted Claiming** เปลี่ยนภาระการขอและติดตั้งใบรับรองไปไว้ที่ **สมาร์ตโฟน (ESP RainMaker Phone App)** แทน
     * ตัวบอร์ด ESP32 ในสภาพโรงงานไม่จำเป็นต้องมีใบรับรองใดๆ ฝังอยู่ล่วงหน้า เพียงแค่เปิดโหมด BLE Provisioning
     * เมื่อผู้ใช้นำสมาร์ตโฟนที่ล็อกอินบัญชี RainMaker สแกน QR Code ของบอร์ด ตัวแอปมือถือจะตรวจพบว่าบอร์ดยังไม่ได้รับการเคลม (Unclaimed Device)
     * แอปมือถือจะทำหน้าที่เป็น "ผู้ช่วย (Assistant)" โดยติดต่อไปยัง **ESP RainMaker Claiming Service (บน AWS)** ขอดาวน์โหลดชุด Certificate และ Private Key ที่ลงนามอย่างถูกต้องสำหรับโหนดนั้นๆ
     * จากนั้นแอปมือถือจะส่งต่อ Certificate และ Key ผ่านช่องทางไร้สายบลูทูธความปลอดภัยสูง (BLE GATT Encrypted Channel) มาบันทึกลงสู่ NVS Flash ของบอร์ด ESP32 ให้โดยอัตโนมัติ
  3. **ประโยชน์ที่ได้รับ:**
     * นักพัฒนาสามารถเปิดกล่องบอร์ดใหม่ แฟลชเฟิร์มแวร์มาตรฐาน แล้วสแกนเชื่อมต่อได้ทันทีภายในไม่กี่วินาที โดยไม่ต้องยุ่งเกี่ยวกับเครื่องมือสร้าง Key บนคอมพิวเตอร์ ช่วยเร่งความเร็วในการพัฒนาและทดสอบผลิตภัณฑ์ต้นแบบได้อย่างมหาศาล

---

### ข้อ 2: เพราะเหตุใดฟังก์ชัน `write_cb` จึงต้องเรียกคำสั่ง `esp_rmaker_param_update_and_report()` ทุกครั้งที่มีการเปลี่ยนแปลงค่าพารามิเตอร์?
* **คำตอบ:**
  1. **การซิงโครไนซ์สถานะจริงของอุปกรณ์สู่ระบบคลาวด์ (State Synchronization):**
     * เมื่อผู้ใช้สั่งงานผ่านแอปมือถือ คำสั่งจะถูกส่งมายังคลาวด์และส่งต่อมายังบอร์ด ESP32 ผ่านช่องทาง MQTT เข้าสู่ฟังก์ชัน Callback `write_cb`
     * การที่บอร์ดเปลี่ยนสถานะ GPIO (เช่น เปิด/ปิด LED หรือปรับระดับแสง) เป็นเพียงการกระทำในระดับฮาร์ดแวร์ท้องถิ่น (Local Actuation)
     * การเรียกคำสั่ง `esp_rmaker_param_update_and_report(param, val)` เป็นการบันทึกสถานะใหม่ลงในโครงสร้างข้อมูลของโหนด (Internal Node DB) และ **ส่งข้อความ MQTT Publish เพื่อรายงานสถานะจริง (Reported State) กลับขึ้นสู่ RainMaker Cloud (AWS Device Shadow / Twin)**
  2. **การอัปเดตหน้าจอแอปพลิเคชันแบบสองทาง (Bi-directional State Confirmation):**
     * หากไม่มีการเรียก `esp_rmaker_param_update_and_report()` คลาวด์จะไม่ได้รับคำยืนยันว่าบอร์ดได้เปลี่ยนสถานะตามคำสั่งแล้วหรือไม่ ทำให้สถานะบนหน้าจอแอปมือถืออาจแสดงผลค้าง เด้งกลับ (State Reversion) หรือแสดงสถานะไม่ตรงกับความเป็นจริง
     * ช่วยให้แอปพลิเคชันบนมือถือเครื่องอื่นๆ หรือบุคคลในครอบครัวที่แชร์อุปกรณ์ร่วมกัน ได้รับการแจ้งเตือนและมองเห็นสถานะปัจจุบันที่ถูกต้องพร้อมกันแบบ Real-time
  3. **การรักษาความถูกต้องของระบบอัตโนมัติ (Automations & Scheduling):**
     * บริการอื่นๆ บนคลาวด์ เช่น ตัวตั้งเวลา (Scheduler) หรือการแจ้งเตือน (Node Alerts) จำเป็นต้องอาศัย Reported State ที่อัปเดตเพื่อนำไปประมวลผลตรรกะที่ถูกต้อง

---

### ข้อ 3: เปรียบเทียบความแตกต่างระหว่างการพัฒนาอุปกรณ์ IoT ด้วย Plain MQTT บน Mosquitto เอง (ในแล็บ 11.2) กับการใช้ ESP RainMaker SDK (ในแล็บ 11.4) ในแง่ของความพยายามในการพัฒนา (Development Effort), ความปลอดภัย (Security), และการขยายระบบ (Scalability)
* **คำตอบ:**

| มิติการเปรียบเทียบ | Plain MQTT บน Mosquitto เอง (แล็บ 11.2) | ESP RainMaker SDK (แล็บ 11.4) |
| :--- | :--- | :--- |
| **1. ความพยายามในการพัฒนา (Development Effort)** | **สูงมาก (High Effort):**<br>• ผู้พัฒนาต้องออกแบบโครงสร้าง MQTT Topic และโครงสร้าง JSON Payload เองทั้งหมด<br>• ต้องเขียนโค้ดจัดการการเชื่อมต่อ Wi-Fi และฮาร์ดโค้ดรหัสผ่าน Wi-Fi ลงในบอร์ด<br>• ต้องเขียนแอปพลิเคชันมือถือ แดชบอร์ด และระบบบริหารจัดการผู้ใช้ (User Auth) ขึ้นมาใหม่ทั้งหมดเอง | **ต่ำมาก (Low Effort / Out-of-the-box):**<br>• มีโครงสร้างข้อมูลมาตรฐาน (Node, Device, Param) และ Dynamic UI มาให้พร้อมใช้<br>• มีระบบ BLE Wi-Fi Provisioning สแกน QR Code จากแอปพลิเคชันมือถือมาตรฐานได้ทันทีโดยไม่ต้องเขียนแอปเอง<br>• มีระบบตั้งเวลา (Scheduler) และซิงก์เวลา SNTP ระดับระบบพร้อมใช้งาน |
| **2. ด้านความปลอดภัย (Security)** | **มีความเสี่ยงสูงหากตั้งค่าไม่รัดกุม (Low to Medium):**<br>• ในแล็บ 11.2 ข้อมูลส่งแบบ Plaintext (พอร์ต 1883) ไม่มี TLS ข้อมูลถูกดักจับได้<br>• แม้เปิด TLS (แล็บ 11.3) ก็ต้องบริหารจัดการแจกจ่าย Client Key ด้วยตนเอง หากคีย์หลุดจะเกิดความเสียหายทั้งระบบ | **ความปลอดภัยระดับมาตรฐานอุตสาหกรรม (Enterprise Grade):**<br>• บังคับใช้ MQTTS ผ่าน TLS 1.2/1.3 ทุกเส้นทางบน AWS IoT Core<br>• มีระบบ Assisted Claiming และ Factory Claiming เพื่อจัดการกุญแจและใบรับรอง X.509 อย่างปลอดภัย<br>• มีระบบยืนยันตัวตนผู้ใช้ระดับสากลผ่าน AWS Cognito และระบบสิทธิ์ผูกบัญชี (User-Node Association) |
| **3. การขยายระบบ (Scalability)** | **มีข้อจำกัดและต้องการการดูแลสูง (Limited Scalability):**<br>• เซิร์ฟเวอร์ Mosquitto ตัวเดียว (Single-broker) รองรับอุปกรณ์พร้อมกันได้จำกัดตามสเปกของเครื่องคอมพิวเตอร์/เซิร์ฟเวอร์<br>• เสี่ยงต่อปัญหา Single Point of Failure (หากเซิร์ฟเวอร์ล่ม ทั้งระบบใช้งานไม่ได้)<br>• ต้องตั้งค่า Port Forwarding หรือ Dynamic DNS เพื่อให้เข้าถึงจากภายนอกบ้านได้ | **ไร้ขีดจำกัดแบบ Serverless (Massive Scalability):**<br>• แบ็กเอนด์ทำงานบนโครงสร้าง Serverless ของ AWS (AWS IoT Core, Lambda, DynamoDB)<br>• ขยายตัวรองรับอุปกรณ์และผู้ใช้งานนับล้านตัวได้โดยอัตโนมัติ (Elastic Auto-scaling)<br>• มี High Availability (HA) ระดับ 99.99% กระจายอยู่ทั่วโลก เข้าถึงจากเครือข่าย 4G/5G ได้จากทุกที่โดยไม่ต้องตั้งค่าเราเตอร์ |

---

### ข้อ 4: อธิบายขั้นตอนที่ทำให้ ESP RainMaker สามารถเชื่อมโยงการทำงานร่วมกับระบบสั่งงานด้วยเสียงของ Google Home หรือ Amazon Alexa ได้โดยไม่ต้องเขียนโค้ดเพิ่มเติมบนตัวบอร์ด ESP32
* **คำตอบ:**
  1. **การใช้แบบจำลองข้อมูลมาตรฐาน (Standardized Data Model):**
     * ESP RainMaker กำหนดนิยามชนิดของอุปกรณ์และพารามิเตอร์เป็นประเภทมาตรฐานระดับสากล เช่น `esp.device.lightbulb`, พารามิเตอร์ `esp.param.power` และ `esp.param.brightness`
     * ชิป ESP32 ส่ง Schema ข้อมูลมาตรฐานเหล่านี้ขึ้นไปประกาศตัวตนไว้บน RainMaker Cloud ตั้งแต่ตอนเริ่มทำงาน
  2. **การเชื่อมต่อระหว่างคลาวด์สู่คลาวด์ (Cloud-to-Cloud Integration):**
     * การสั่งงานด้วยเสียงไม่ได้เกิดขึ้นโดยตรงระหว่างลำโพงอัจฉริยะ (เช่น Google Nest / Amazon Echo) กับตัวบอร์ด ESP32 แต่เป็นการเชื่อมต่อผ่าน **API ระดับคลาวด์ (Cloud-to-Cloud Linking)**
     * ผู้ใช้งานเพียงเปิดแอป Google Home หรือ Alexa แล้วทำการ **Link Account** เข้ากับบัญชี ESP RainMaker ผ่านโปรโตคอล OAuth 2.0
  3. **การแปลงคำสั่งผ่าน Serverless Smart Home Skill / Action:**
     * คลาวด์ของ RainMaker มี Smart Home Skill ที่สร้างไว้บน AWS Lambda ซึ่งทำหน้าที่จับคู่และแปลความหมายคำสั่งเสียงของ Google/Alexa ให้เข้ากับ Data Model ของ RainMaker โดยอัตโนมัติ
     * **ตัวอย่าง:** เมื่อผู้ใช้สั่ง *"Hey Google, turn on the Living Room Light"*
       1. Google Assistant แปลงเสียงพูดเป็นคำสั่ง Smart Home ส่งต่อไปยัง RainMaker Cloud
       2. RainMaker Cloud นำคำสั่งไปแปลงเป็นพารามิเตอร์ `{"Power": true}` ของอุปกรณ์ "Living Room Light"
       3. RainMaker Cloud ส่งข้อความผ่าน MQTT ไปยังบอร์ด ESP32 ผ่านการเชื่อมต่อที่มีอยู่เดิม
       4. บอร์ด ESP32 ได้รับค่าในฟังก์ชัน `write_cb` และสั่งเปิดไฟ LED ตามปกติ
  4. **สรุป:** เฟิร์มแวร์บนตัวบอร์ด ESP32 จึงทำหน้าที่เพียงแค่ตอบสนองต่อคำสั่งพารามิเตอร์มาตรฐานใน `write_cb` เท่านั้น โดยการรู้จำเสียงพูด การตีความภาษาธรรมชาติ (NLP) และการแปลงโปรโตคอลทั้งหมด ถูกจัดการบนระบบคลาวด์แบบเบ็ดเสร็จ
