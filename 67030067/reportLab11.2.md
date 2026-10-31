
# รายงานผลการทดลองที่ 11.2
## การพัฒนา ESP32 MQTT Client เพื่อรายงานข้อมูลเซนเซอร์ Telemetry และรับคำสั่งควบคุมอุปกรณ์ระยะไกล

---

## 4. บันทึกและวิเคราะห์ผลการทดลอง (Results & Verification)

### 4.1 บันทึกการทดสอบการทำงานของระบบ

#### 1. การตรวจสอบข้อมูล Telemetry จาก ESP32

เมื่อทำการแฟลชโปรแกรมลงบอร์ด ESP32 และรันคำสั่ง `mosquitto_sub` บนเครื่องคอมพิวเตอร์เพื่อเฝ้าดูหัวข้อ `iot/node01/telemetry` พบว่า ESP32 สามารถส่งข้อมูล Telemetry มายัง MQTT Broker ได้สำเร็จ

**คำสั่งที่ใช้ทดสอบ**

```powershell
mosquitto_sub -h 192.168.1.111 -p 1883 -t "iot/node01/telemetry" -v
```

**ผลลัพธ์การทำงานฝั่ง Subscriber Terminal**

```text
iot/node01/telemetry {"node_id":"esp32-node01","adc_raw":1786,"voltage_v":1.439,"led_state":false,"uptime_ms":1910367}
iot/node01/telemetry {"node_id":"esp32-node01","adc_raw":2450,"voltage_v":1.974,"led_state":false,"uptime_ms":1914367}
iot/node01/telemetry {"node_id":"esp32-node01","adc_raw":4095,"voltage_v":3.300,"led_state":false,"uptime_ms":1918367}
```

**การวิเคราะห์ผล**

- บอร์ด ESP32 สามารถอ่านค่าความต่างศักย์แอนะล็อกผ่านโมดูล ADC1 (GPIO 34) ได้สำเร็จ และจัดรูปแบบแพ็กเก็ตข้อมูล JSON ด้วยไลบรารี cJSON ก่อน Publish ขึ้นสู่ Broker ทุก ๆ 4 วินาทีอย่างสม่ำเสมอ
- เมื่อทดลองปรับหมุนก้าน Potentiometer ค่า `adc_raw` มีการเปลี่ยนแปลงตามระดับการหมุนตั้งแต่ 0 ถึง 4095 และส่งผลให้ค่าคำนวณแรงดันไฟฟ้า `voltage_v` เปลี่ยนแปลงสอดคล้องกันตั้งแต่ 0.0V ถึง 3.3V

#### 2. การทดสอบส่งคำสั่งควบคุมหลอดไฟ LED ระยะไกล

**การทดสอบที่ 2.1: ส่งคำสั่งข้อความธรรมดา ("ON" / "OFF")**

**คำสั่งเปิดไฟ (ON)**

```powershell
mosquitto_pub -h 192.168.1.111 -p 1883 -t "iot/node01/cmd/led" -m "ON" -q 1
```

**ผลการทดลอง:** หลอดไฟ LED ติดสว่างทันที

**Log บน ESP32 Serial Monitor**

```text
I (125400) LAB11_MQTT: MQTT_EVENT_DATA received:
I (125400) LAB11_MQTT: TOPIC=iot/node01/cmd/led
I (125400) LAB11_MQTT: DATA=ON
I (125405) LAB11_MQTT: LED State set to -> ON
```

**คำสั่งปิดไฟ (OFF)**

```powershell
mosquitto_pub -h 192.168.1.111 -p 1883 -t "iot/node01/cmd/led" -m "OFF" -q 1
```

**ผลการทดลอง:** หลอดไฟ LED ดับลงทันที

**Log บน ESP32 Serial Monitor**

```text
I (132800) LAB11_MQTT: MQTT_EVENT_DATA received:
I (132800) LAB11_MQTT: TOPIC=iot/node01/cmd/led
I (132800) LAB11_MQTT: DATA=OFF
I (132805) LAB11_MQTT: LED State set to -> OFF
```

**การทดสอบที่ 2.2: ส่งคำสั่งในรูปแบบ JSON Payload (`{"state": 1}`)**

**คำสั่งรูปแบบ JSON**

```powershell
mosquitto_pub -h 192.168.1.111 -p 1883 -t "iot/node01/cmd/led" -m "{\\"state\\": 1}" -q 1
```

**ผลการทดลอง:** ฟังก์ชัน `strstr()` ใน MQTT Event Handler สามารถตรวจสอบรูปแบบข้อความ JSON ที่มี `"state": 1` ได้อย่างถูกต้อง ทำให้หลอดไฟ LED เปลี่ยนสถานะเป็นเปิดสว่าง พร้อมทั้งส่งข้อความยืนยันสถานะตอบกลับทาง Topic `iot/node01/state/led`

### 4.2 ตารางสรุปโครงสร้าง MQTT Topics ในระบบ

| ชื่อ Topic | ทิศทางการส่งข้อมูล | ระดับ QoS | แฟล็ก Retain | หน้าที่การทำงาน |
| :--- | :--- | :---: | :---: | :--- |
| `iot/node01/status` | ESP32 → Broker | 1 | **1 (True)** | แจ้งสถานะการเชื่อมต่อ (`online` เมื่อต่อสำเร็จ / `offline` ผ่านกลไก LWT) |
| `iot/node01/telemetry` | ESP32 → Broker | 0 | 0 (False) | รายงานค่า ADC, แรงดัน (Voltage) และสถานะ LED ในรูปแบบ JSON ทุก ๆ 4 วินาที |
| `iot/node01/cmd/led` | Broker → ESP32 | 1 | 0 (False) | รับคำสั่งเปิด-ปิด LED จากระยะไกล (รองรับทั้ง "ON", "OFF" และ `{"state":1}`) |
| `iot/node01/state/led` | ESP32 → Broker | 1 | 0 (False) | ส่งข้อความยืนยันสถานะ LED ปัจจุบันตอบกลับทันทีเมื่อประมวลผลคำสั่งสำเร็จ (State Acknowledgment) |

---

## 5. คำถามท้ายการทดลอง (Post-Lab Questions)

### ข้อ 1. เพราะเหตุใดเราจึงต้องส่งข้อความสถานะ `iot/node01/status = "online"` ด้วยแฟล็ก `retain = 1` ควบคู่กับการตั้งค่า Last Will and Testament (LWT) ว่า `"offline"`?

**คำตอบ:** เพื่อให้ Broker เก็บบันทึกสถานะ `"online"` ค้างไว้ ทำให้ Dashboard หรือ Client ที่เพิ่งเปิดขึ้นมาใหม่ทราบสถานะอุปกรณ์ได้ทันที และหากอุปกรณ์เกิดไฟดับหรือดับกะทันหัน Broker จะส่งพินัยกรรม `"offline"` ทับให้อัตโนมัติ ป้องกันข้อมูลสถานะค้าง

### ข้อ 2. หากเครือข่าย Wi-Fi หลุดชั่วคราว คอมโพเนนต์ `esp-mqtt` มีพฤติกรรมการจัดการอย่างไร และเมื่อสัญญาณกลับมา โค้ดจำเป็นต้องเรียกฟังก์ชันเชื่อมต่อใหม่ด้วยตนเองหรือไม่?

**คำตอบ:** ระบบจะพยายาม Reconnect ให้อัตโนมัติ ไม่จำเป็นต้องเขียนโค้ดสั่งเชื่อมต่อใหม่เอง เมื่อ Wi-Fi กลับมาและได้รับ IP ใหม่ ตัว Client จะทำการ Re-subscribe หัวข้อเดิมให้อัตโนมัติทันที

### ข้อ 3. ในฟังก์ชัน `telemetry_task` เพราะเหตุใดหลังจากเรียก `cJSON_PrintUnformatted(root)` แล้วเราจึงจำเป็นต้องเรียกฟังก์ชัน `free(json_str)` และ `cJSON_Delete(root)` เสมอ? หากละเลยจะมีผลเสียอย่างไรต่อหน่วยความจำของ ESP32?

**คำตอบ:** เพื่อคืนพื้นที่หน่วยความจำบน Heap Memory หากละเลย จะเกิดปัญหา **Memory Leak** สะสมจน RAM หมด และส่งผลให้ ESP32 เกิด Crash หรือ Restart ตนเองในที่สุด

---

## 6. สรุปผลการทดลอง

จากการทดลองสามารถพัฒนา ESP32 ให้ทำงานร่วมกับ MQTT Broker ได้สำเร็จ โดยสามารถอ่านค่าจาก Potentiometer ผ่าน ADC และส่งข้อมูล Telemetry ในรูปแบบ JSON ไปยัง Broker ทุก ๆ 4 วินาที นอกจากนี้ยังสามารถรับคำสั่งควบคุม LED จากระยะไกลผ่าน MQTT ได้ทั้งรูปแบบข้อความธรรมดาและ JSON Payload พร้อมส่งข้อความยืนยันสถานะกลับไปยัง Broker แสดงให้เห็นว่าระบบสามารถสื่อสารข้อมูลและควบคุมอุปกรณ์ผ่าน MQTT ได้ตามวัตถุประสงค์ของการทดลอง
