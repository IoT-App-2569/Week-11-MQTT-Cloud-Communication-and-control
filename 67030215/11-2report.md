## 4. การทดสอบและการตรวจสอบผลลัพธ์ (Testing & Verification)

> [!IMPORTANT] **ข้อสังเกตและแนวทางแก้ปัญหาสำคัญ (Troubleshooting Guide)**
> 1. **ปัญหา Broker ทำงานซ้ำซ้อน (Split-Brain Broker บน Windows)**:
>    * หากเครื่องคอมพิวเตอร์มีการติดตั้ง Mosquitto เป็น Windows Service ตัว Service ดังกล่าวจะแอบรันอยู่เบื้องหลังและผูกกับ IP `127.0.0.1` ตลอดเวลา
>    * เมื่อท่านเปิดรัน `mosquitto.exe -c mosquitto.conf -v` ใน Terminal อีกตัว บอร์ด ESP32 จะเชื่อมต่อเข้ากับ Terminal (ผ่าน IP วงแลน `192.168.1.xxx`) ในขณะที่คำสั่ง `mosquitto_pub` ที่ส่งไปยัง `127.0.0.1` จะวิ่งไปหา Windows Service แทน ทำให้ส่งคำสั่งไม่ถึงตัวบอร์ด!
>    * **วิธีแก้:** ให้ปิด Windows Service ก่อนเสมอด้วยคำสั่งใน PowerShell (Admin):  
>      `Stop-Service mosquitto; Set-Service mosquitto -StartupType Manual`
> 2. **ทำไมใน MQTT Explorer จึงแสดงเป็น QoS 0 ทั้งที่ส่งด้วย `-q 1`?**:
>    * ตามมาตรฐาน MQTT ระดับ QoS ที่ผู้รับจะได้รับจริงถูกคำนวณจาก **`min(QoS_publish, QoS_subscribe)`**
>    * โปรแกรม MQTT Explorer ทำการ Subscribe ทุกหัวข้อ (`#`) ด้วย **QoS 0** เป็นค่าเริ่มต้น เมื่อ Broker ได้รับข้อความระดับ QoS 1 จึงทำการลดระดับ (Downgrade) เป็น QoS 0 ส่งให้ MQTT Explorer แสดงผล
>    * หากต้องการให้ข้อความปรากฏค้างอยู่ใน MQTT Explorer เสมอแม้ส่งเสร็จไปแล้ว ให้เพิ่มแฟล็ก **`-r` (Retain)** ในคำสั่ง Publish
> 3. **การเปิดพอร์ต Firewall**: ตรวจสอบว่า Windows Defender Firewall อนุญาตให้พอร์ต TCP 1883 รับการเชื่อมต่อจาก Inbound LAN


## 4. ผลการทดสอบ (Testing & Verification)

### 4.1 การเชื่อมต่อ Wi-Fi และ MQTT Broker

<img width="918" height="302" alt="Screenshot 2026-10-23 204317" src="https://github.com/user-attachments/assets/251aa820-2046-4eda-929b-025b895521c5" />


| รายการ | ค่า |
|:---|:---|
| ESP-IDF | v6.0.2 |
| Target | ESP32 |
| Wi-Fi | Mobile Hotspot (2.4 GHz) |
| IP ของ ESP32 | 172.20.10.3 |
| IP ของ Broker (คอมพิวเตอร์) | 172.20.10.4 |
| Broker URI | `mqtt://172.20.10.4:1883` |

### 4.2 การตรวจสอบ Telemetry Data

```
mosquitto_sub -h 172.20.10.4 -p 1883 -t "iot/node01/telemetry" -v
```

<img width="1452" height="747" alt="image" src="https://github.com/user-attachments/assets/b514ea9d-d4d6-40e8-9481-6916e71ffed8" />


ESP32 ส่ง JSON Payload ทุก 4 วินาที เมื่อหมุน Potentiometer ค่าเปลี่ยนแปลงตามได้ถูกต้องครบทั้งช่วง

| ตำแหน่ง Potentiometer | adc_raw | voltage_v |
|:---|:---:|:---:|
| หมุนสุดด้านหนึ่ง | 4095 | 3.30 V |
| ตำแหน่งกลาง | ~2520 | ~2.03 V |
| หมุนสุดอีกด้าน | 0 | 0.00 V |

### 4.3 การส่งคำสั่งเปิด-ปิดหลอดไฟ LED

<img width="922" height="323" alt="image" src="https://github.com/user-attachments/assets/d6188137-8e6d-4abb-b8d8-57b011b54206" />
<img width="935" height="320" alt="image" src="https://github.com/user-attachments/assets/55851dea-d281-4ea1-a39f-241cb1e1056b" />


| คำสั่งที่ส่ง | DATA ที่ ESP32 ได้รับ | ผลที่ได้ | led_state ใน Telemetry |
|:---|:---|:---|:---:|
| `-m "ON"` | `ON` | LED ติด | `true` |
| `-m "OFF"` | `OFF` | LED ดับ | `false` |
| `-m '{\"state\": 1}'` | `{"state": 1}` | LED ติด | `true` |

ลำดับการทำงานที่สังเกตได้จาก Monitor: `MQTT_EVENT_DATA received` → แสดง `TOPIC` และ `DATA` → `LED State set to -> ON/OFF` → Telemetry รอบถัดไปรายงานสถานะใหม่

### 4.4 การทดสอบสถานะ Online/Offline (Retained + LWT)

```
mosquitto_sub -h 172.20.10.4 -p 1883 -t "iot/node01/#" -v
```

<img width="1360" height="347" alt="image" src="https://github.com/user-attachments/assets/46997599-d79f-4b85-a483-cbee25aac2bf" />



1. เมื่อเปิด Subscriber ได้รับ `iot/node01/status online` ทันที แม้ ESP32 ส่งไว้นานแล้ว เพราะส่งด้วย Retain
2. เมื่อถอดสาย USB ของบอร์ด Telemetry หยุด และ Broker ส่ง `iot/node01/status offline` แทนบอร์ดหลัง Keep Alive หมดเวลา (ประมาณ 1.5 × 60 วินาที)
3. เมื่อเสียบสายกลับ บอร์ดรีบูต (`uptime_ms` เริ่มนับใหม่) เชื่อมต่อใหม่และส่ง `online` อีกครั้ง โดย `led_state` กลับเป็น `false` เพราะสถานะ LED ไม่ได้ถูกบันทึกไว้

### 4.5 ปัญหาที่พบและวิธีแก้

| ปัญหา | สาเหตุ | วิธีแก้ |
|:---|:---|:---|
| คำสั่ง JSON `-m "{\"state\": 1}"` ไม่ทำให้ LED ติด | Windows PowerShell 5.1 ตัดเครื่องหมาย `"` ออกก่อนส่งให้ `mosquitto_pub` ทำให้ ESP32 ได้รับ `{state: 1}` | ใช้ `-m '{\"state\": 1}'` (ครอบด้วย `'` และใช้ `\"` ข้างใน) |
| ESP32 เชื่อมต่อ Broker ไม่ได้ (ป้องกันไว้ก่อน) | Windows Firewall บล็อกพอร์ต 1883 และ Mosquitto Service ทำงานซ้อน | เปิด Inbound Rule พอร์ต TCP 1883 และไม่ติดตั้ง Mosquitto Service |

---
---

### 1. การตรวจสอบ Telemetry Data
เปิดเทอร์มินัลบนเครื่องคอมพิวเตอร์ แล้วรันคำสั่งบอกรับข้อมูล (เปลี่ยน `192.168.1.xxx` เป็น IP ของคอมพิวเตอร์ที่รัน Broker):
```powershell
# สามารถใช้ IP ของเครื่อง หรือ 127.0.0.1 (หากปิด Windows Service แล้ว)
mosquitto_sub -h 192.168.1.111 -p 1883 -t "iot/node01/telemetry" -v
```
**ผลลัพธ์ที่คาดหวัง**: จะได้รับ JSON ข้อมูลเซนเซอร์ทุกๆ 4 วินาที
```text
iot/node01/telemetry {"node_id":"esp32-node01","adc_raw":1786,"voltage_v":1.439,"led_state":false,"uptime_ms":1910367}
```
*ทดลองหมุนก้านของ Potentiometer*: สังเกตว่าค่า `adc_raw` และ `voltage_v` เปลี่ยนแปลงตามการหมุนอย่างถูกต้อง

---

### 2. การส่งคำสั่งเปิด-ปิดหลอดไฟ LED
เปิดเทอร์มินัลอีกหน้าต่างหนึ่ง แล้วทดลองส่งคำสั่งสั่งงาน:

* **คำสั่งเปิดไฟ**
```powershell
mosquitto_pub -h 192.168.1.111 -p 1883 -t "iot/node01/cmd/led" -m "ON" -q 1
```
*สังเกตผล*: หลอดไฟ LED บนบอร์ดจะติดสว่างทันที และในรอบ Telemetry ถัดไป ค่า `"led_state"` จะเปลี่ยนเป็น `true`

* **คำสั่งปิดไฟ**
```powershell
mosquitto_pub -h 192.168.1.111 -p 1883 -t "iot/node01/cmd/led" -m "OFF" -q 1
```
*สังเกตผล*: หลอดไฟ LED จะดับลงทันที และค่า `"led_state"` ใน Telemetry จะเปลี่ยนกลับเป็น `false`

* **คำสั่งแบบ JSON Payload**
```powershell
mosquitto_pub -h 192.168.1.111 -p 1883 -t "iot/node01/cmd/led" -m "{\"state\": 1}" -q 1
```

---

## 5. คำถามท้ายการทดลอง (Post-Lab Questions)

**1. เพราะเหตุใดเราจึงต้องส่งข้อความสถานะ `iot/node01/status = "online"` ด้วยแฟล็ก `retain = 1` ควบคู่กับการตั้งค่า Last Will and Testament (LWT) ว่า `"offline"`**

เพราะสองอย่างนี้ทำงานเป็นคู่กันเพื่อให้สถานะบน Topic เดียวกันตรงกับความจริงอยู่เสมอ เมื่อ ESP32 เชื่อมต่อสำเร็จ จะส่ง `"online"` พร้อม `retain = 1` เพื่อให้ Broker จำสถานะนี้ไว้ ใครที่เพิ่งมา Subscribe ทีหลังก็เห็นว่าอุปกรณ์ออนไลน์อยู่ทันที ไม่ต้องรอข้อความรอบถัดไป ส่วนตอนที่อุปกรณ์หลุดแบบกะทันหัน Broker จะส่ง LWT `"offline"` (ซึ่งต้องตั้ง retain = 1 ด้วยเช่นกัน) ไปทับค่าเดิมที่ค้างอยู่ ทำให้สถานะที่ค้างไว้เปลี่ยนเป็น offline ตามจริง ถ้าไม่ใช้ retain กับ `"online"` ผู้ที่เข้ามาใหม่จะไม่รู้สถานะของอุปกรณ์เลย และถ้าไม่มี LWT สถานะ `"online"` ที่ค้างอยู่จะยังอยู่อย่างนั้นทั้งที่อุปกรณ์ตายไปแล้ว ทำให้ผู้ใช้เข้าใจผิด

**2. หากเครือข่าย Wi-Fi หลุดชั่วคราว คอมโพเนนต์ `esp-mqtt` มีพฤติกรรมการจัดการอย่างไร และเมื่อสัญญาณกลับมา โค้ดจำเป็นต้องเรียกฟังก์ชันเชื่อมต่อใหม่ด้วยตนเองหรือไม่**

เมื่อการเชื่อมต่อหลุด คอมโพเนนต์ `esp-mqtt` จะส่งอีเวนต์ `MQTT_EVENT_DISCONNECTED` ให้โค้ดทราบ จากนั้นจะพยายามเชื่อมต่อกับ Broker ใหม่เองโดยอัตโนมัติเป็นระยะๆ ตามค่า reconnect timeout ที่ตั้งไว้ เมื่อสัญญาณกลับมาและต่อสำเร็จก็จะส่งอีเวนต์ `MQTT_EVENT_CONNECTED` กลับมาอีกครั้ง ดังนั้นโค้ดไม่จำเป็นต้องเรียกฟังก์ชันเชื่อมต่อใหม่ด้วยตนเอง สิ่งที่ควรทำคือเขียนใน event handler ของ `MQTT_EVENT_CONNECTED` ให้ส่งสถานะ `"online"` และ Subscribe Topic ที่ต้องการใหม่ทุกครั้ง เพราะหลังเชื่อมต่อใหม่ งานเหล่านี้ต้องทำซ้ำ

**3. ในฟังก์ชัน `telemetry_task` เพราะเหตุใดหลังจากเรียก `cJSON_PrintUnformatted(root)` แล้วเราจึงจำเป็นต้องเรียกฟังก์ชัน `free(json_str)` และ `cJSON_Delete(root)` เสมอ? หากละเลยจะมีผลเสียอย่างไรต่อหน่วยความจำของ ESP32**

เพราะทั้ง `cJSON_CreateObject` และ `cJSON_PrintUnformatted` จองหน่วยความจำจาก Heap ขึ้นมาใหม่ทุกครั้งที่เรียก โดยโครงสร้าง JSON (`root`) ต้องคืนด้วย `cJSON_Delete(root)` ส่วนสตริงที่ได้จากการพิมพ์ (`json_str`) ต้องคืนด้วย `free(json_str)` ระบบจะไม่คืนให้เองอัตโนมัติ เนื่องจาก `telemetry_task` ทำงานวนซ้ำเป็นรอบๆ ถ้าไม่คืนหน่วยความจำ Heap จะค่อยๆ ลดลงทุกรอบ เรียกว่า Memory Leak จนในที่สุดหน่วยความจำว่างหมด การจองครั้งใหม่ล้มเหลว ESP32 อาจค้าง รีเซ็ตตัวเอง หรือทำงานผิดพลาด ซึ่งเป็นปัญหาใหญ่สำหรับอุปกรณ์ IoT ที่ต้องรันต่อเนื่องเป็นวันเป็นเดือน
