
# รายงานผลการทดลองที่ 11.4
## การสร้างอุปกรณ์ Smart Light สู่ระบบคลาวด์ด้วย ESP RainMaker SDK, BLE Provisioning และการควบคุมผ่านสมาร์ตโฟน

---

## 4. บันทึกและวิเคราะห์ผลการทดลอง (Results & Verification)

### 4.1 บันทึกผลการทดสอบการทำงานของระบบ (Testing Results)

#### 1. การสร้าง Node, Device และการ Provisioning ผ่าน BLE

เมื่อสั่งคอมไพล์และรันโปรแกรมลงบนบอร์ด ESP32/ESP32-C3 บอร์ดจะทำการเปิดระบบ BLE Advertising และสร้าง QR Code ออกมาทาง Serial Monitor

**ผลลัพธ์บน Serial Monitor (Bootup Sequence & Provisioning)**

```text
I (1250) RMAKER_LIGHT: Wi-Fi Initialization finished.
I (1820) esp_rmaker_core: Node ID: 884a180d52bc
I (1980) esp_rmaker_core: Node initialized successfully.
I (2100) app_wifi: Provisioning Started.
I (2105) app_wifi: Scan this QR Code to provision:
https://rainmaker.espressif.com/qrcode.html?data={"ver":"v1","name":"PROV_LIGHT","pop":"abcd1234","transport":"ble"}
[ASCII QR CODE RENDERED HERE]
```

#### 2. การควบคุมอุปกรณ์ผ่านแอปพลิเคชันมือถือ (Mobile App Control)

**2.1 การทดสอบสวิตช์ Power (ON / OFF)**

เมื่อกดสวิตช์เปิดไฟ (Power = ON) บนแอปพลิเคชัน พบว่าหลอดไฟ LED ที่ต่อกับขา GPIO (GPIO 8 บน ESP32-C3 / GPIO 2 บน ESP32) ติดสว่างทันที

**Log บน Serial Monitor**

```text
I (45210) RMAKER_LIGHT: Request received via: Cloud
I (45210) RMAKER_LIGHT: Device: Living Room Light | Param: Power
I (45215) RMAKER_LIGHT: Changing LED state to: ON
```

เมื่อกดสวิตช์ปิดไฟ (Power = OFF) บนแอปพลิเคชัน พบว่าหลอดไฟ LED ดับลงทันที

**Log บน Serial Monitor**

```text
I (52400) RMAKER_LIGHT: Request received via: Cloud
I (52400) RMAKER_LIGHT: Device: Living Room Light | Param: Power
I (52405) RMAKER_LIGHT: Changing LED state to: OFF
```

**2.2 การทดสอบแถบปรับความสว่าง (Brightness)**

เมื่อทดลองปรับแถบความสว่างบนแอปพลิเคชัน พบว่าบอร์ด ESP32 ได้รับค่าความสว่างที่กำหนด

**Log บน Serial Monitor**

```text
I (68120) RMAKER_LIGHT: Request received via: Cloud
I (68120) RMAKER_LIGHT: Device: Living Room Light | Param: Brightness
I (68125) RMAKER_LIGHT: Changing Brightness to: 75%
```

#### 3. การทดสอบควบคุมระยะไกลข้ามเครือข่าย (Remote Control 4G/5G Test)

เมื่อทำการปิด Wi-Fi บนสมาร์ตโฟน แล้วเปลี่ยนไปใช้เครือข่ายมือถือ 4G/5G พบว่ายังคงสามารถส่งคำสั่งเปิด-ปิดหลอดไฟ LED บนบอร์ด ESP32 ได้อย่างสม่ำเสมอ โดยมีค่าเวลาแฝง (Latency) ในการตอบสนองน้อยกว่า 1 วินาที

### 4.2 ตารางสรุปโครงสร้าง Node, Device และ Parameters ในระบบ ESP RainMaker

| โครงสร้างระดับ | ชื่อรายการ (Name) | ชนิด / ประเภท | หน้าที่การทำงาน |
| :--- | :--- | :--- | :--- |
| **Node** | `Smart Light Node` | Root Container | เป็นออบเจกต์หลักที่แทนตัวอุปกรณ์ ESP32 ทั้งเครื่อง |
| **Device** | `Living Room Light` | Lightbulb Device | เป็นออบเจกต์อุปกรณ์ย่อยประเภทหลอดไฟที่แสดงผลบนแอปพลิเคชัน |
| **Parameter** | `Power` | Standard Boolean | รับ-ส่งสถานะเปิด/ปิดไฟ (`true` / `false`) เพื่อควบคุมระดับสัญญาณ GPIO |
| **Parameter** | `Brightness` | Standard Integer (0-100) | รับ-ส่งระดับความสว่างไฟเพื่อนำไปปรับควบคุมค่าความสว่าง (PWM) |
| **Service** | Time Sync & Schedule | System Services | บริการซิงก์เวลาโลก (SNTP) และระบบตั้งเวลาทำงานอัตโนมัติ |

---

## 5. คำถามท้ายการทดลอง (Post-Lab Questions)

### ข้อ 1. ในสถาปัตยกรรม ESP RainMaker กระบวนการ Assisted Claiming ช่วยแก้ปัญหาเรื่องการติดตั้งใบรับรองความปลอดภัย (Security Certificates) บนอุปกรณ์ในขั้นตอนพัฒนาต้นแบบได้อย่างไร?

**คำตอบ:** ช่วยให้อุปกรณ์ขอรับใบรับรองดิจิทัล (X.509 Certificate) จากคลาวด์ผ่านแอปพลิเคชันมือถือ (BLE) ได้โดยอัตโนมัติในขั้นตอน Provisioning ทำให้นักพัฒนาไม่ต้องสร้าง Server CA หรือแฟลช Certificate ลงบอร์ดเอง

### ข้อ 2. เพราะเหตุใดฟังก์ชัน `write_cb` จึงต้องเรียกคำสั่ง `esp_rmaker_param_update_and_report()` ทุกครั้งที่มีการเปลี่ยนแปลงค่าพารามิเตอร์?

**คำตอบ:** เพื่ออัปเดตค่าพารามิเตอร์ในหน่วยความจำของบอร์ด และส่งข้อความ MQTT รายงานสถานะกลับไปยังคลาวด์ เพื่อให้หน้าจอแอปพลิเคชันมือถืออัปเดตสถานะปุ่มและแถบสไลเดอร์ให้ตรงกับความเป็นจริงทันที

### ข้อ 3. เปรียบเทียบความแตกต่างระหว่างการพัฒนาอุปกรณ์ IoT ด้วย Plain MQTT บน Mosquitto เอง (ในแล็บ 11.2) กับการใช้ ESP RainMaker SDK (ในแล็บ 11.4) ในแง่ของความพยายามในการพัฒนา (Development Effort), ความปลอดภัย (Security) และการขยายระบบ (Scalability)

**คำตอบ:**

- **Development Effort:** Plain MQTT ต้องเขียนระบบ Provisioning, JSON และ UI เองทั้งหมด (Effort สูง) ส่วน RainMaker มีระบบจัดการและแอปพลิเคชันสำเร็จรูปให้ (Effort ต่ำมาก)
- **Security:** Plain MQTT ต้องตั้งค่า TLS/mTLS และจัดการ Certificate เอง ส่วน RainMaker บริหารจัดการการเข้ารหัสและ Certificate ให้อัตโนมัติผ่าน Assisted Claiming
- **Scalability:** Plain MQTT ต้องดูแล Server Broker และ Infrastructure เองทั้งหมด ส่วน RainMaker ใช้ระบบ Cloud (AWS) รองรับการขยายตัวระดับอุตสาหกรรมโดยไม่ต้องดูแลระบบหลังบ้านเอง

### ข้อ 4. อธิบายขั้นตอนที่ทำให้ ESP RainMaker สามารถเชื่อมโยงการทำงานร่วมกับระบบสั่งงานด้วยเสียงของ Google Home หรือ Amazon Alexa ได้โดยไม่ต้องเขียนโค้ดเพิ่มเติมบนตัวบอร์ด ESP32

**คำตอบ:** ทำงานผ่านระบบ **Cloud-to-Cloud Integration** โดยเมื่อผู้ใช้ทำ Account Linking บอร์ด ESP32 ที่ใช้ Standard Data Model จะถูกคลาวด์ส่งคำสั่งเสียงมาแปลงเป็นแพ็กเก็ต MQTT พารามิเตอร์มาตรฐาน (`Power`, `Brightness`) ลงมายังบอร์ดได้ทันทีผ่าน `write_cb` เดิม โดยไม่ต้องแก้ไขโค้ดบนบอร์ด

---

## 6. สรุปผลการทดลอง

จากการทดลองสามารถพัฒนาอุปกรณ์ Smart Light ด้วย ESP RainMaker SDK ได้สำเร็จ โดยบอร์ด ESP32 สามารถสร้าง Node และ Device พร้อม Provisioning ผ่าน BLE เพื่อเชื่อมต่อกับ Wi-Fi และระบบคลาวด์ได้ นอกจากนี้ยังสามารถควบคุมการเปิด-ปิด LED และปรับระดับความสว่างผ่านแอปพลิเคชันบนสมาร์ตโฟน รวมถึงทดสอบการควบคุมจากเครือข่ายมือถือ 4G/5G ได้ แสดงให้เห็นถึงการนำ ESP RainMaker มาใช้พัฒนาอุปกรณ์ IoT ที่สามารถควบคุมผ่านระบบคลาวด์ได้
