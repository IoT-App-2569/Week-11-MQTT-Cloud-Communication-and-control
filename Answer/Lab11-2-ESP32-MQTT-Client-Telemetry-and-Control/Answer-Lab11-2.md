# Answer Lab 11.2 – ESP32 MQTT Client: Telemetry และควบคุม LED

- ชื่อ-สกุล: 
- รหัสนักศึกษา: 67030098
- วันที่ทำการทดลอง: 
- โค้ด: [Code/Lab11-2](../../Code/Lab11-2-ESP32-MQTT-Client-Telemetry-and-Control)
- รูปภาพ: [Answer/Image/Lab11-2](../Image/Lab11-2)

---

## 1. ผลการทดลอง

### 11-2.1 Build / Flash
![ผล idf.py build flash](../Image/Lab11-2/11-2-1_build_flash.png)

*รูปที่ ผล idf.py build flash*: 

### 11-2.2 Serial Monitor (เชื่อม Wi-Fi และ MQTT)
![Serial monitor](../Image/Lab11-2/11-2-2_serial_monitor.png)

*รูปที่ Serial monitor*: 

### 11-2.3 Telemetry (mosquitto_sub)
![JSON telemetry](../Image/Lab11-2/11-2-3_telemetry.png)

*รูปที่ JSON telemetry*: 

| ค่า Potentiometer (ตำแหน่ง) | ค่า ADC / JSON ที่ได้ |
|---|---|
| ต่ำสุด | |
| กลาง | |
| สูงสุด | |

### 11-2.4 ควบคุม LED
| คำสั่ง Publish | สถานะ LED | หมายเหตุ |
|---|---|---|
| `ON` | | |
| `OFF` | | |
| `{"state": 1}` | | |

![ผลการสั่ง LED](../Image/Lab11-2/11-2-4_led_control.png)

*รูปที่ ผลการสั่ง LED*: 

## 2. คำถามท้ายการทดลอง

**1. เหตุใดต้องส่ง `status = "online"` แบบ retain ควบคู่กับ LWT `"offline"`?**

> คำตอบ: 

**2. เมื่อ Wi-Fi หลุดชั่วคราว `esp-mqtt` ทำงานอย่างไร ต้องเรียกเชื่อมต่อใหม่เองหรือไม่?**

> คำตอบ: 

**3. เหตุใดต้องเรียก `free(json_str)` และ `cJSON_Delete(root)` และผลเสียหากละเลย?**

> คำตอบ: 

## 3. สรุปผลการทดลอง

