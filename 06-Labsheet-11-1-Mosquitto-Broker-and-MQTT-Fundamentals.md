# ใบงานการทดลองที่ 11.1
### การติดตั้ง Mosquitto Broker และการทดสอบโปรโตคอล MQTT ด้วย CLI (Pub/Sub, QoS, Retain, LWT)

> [!NOTE] **คำชี้แจง**
> ในใบงานนี้นักศึกษาจะได้เรียนรู้การติดตั้งและตั้งค่า **Eclipse Mosquitto MQTT Broker** บนเครื่องคอมพิวเตอร์เพื่อทำหน้าที่เป็นตัวกลางรับส่งข้อมูลของระบบ IoT จากนั้นใช้เครื่องมือบรรทัดคำสั่ง **`mosquitto_sub`** และ **`mosquitto_pub`** ในการทดลองคุณสมบัติสำคัญของโปรโตคอล MQTT ได้แก่ ระบบหัวข้อ (Topics & Wildcards), ระดับคุณภาพการให้บริการ (QoS 0, 1, 2), ข้อความคงค้าง (Retained Messages) และพินัยกรรมฉุกเฉิน (Last Will and Testament - LWT)

---

## 1. วัตถุประสงค์การทดลอง
1. สามารถติดตั้งและกำหนดค่าไฟล์คอนฟิกูเรชันของ Eclipse Mosquitto MQTT Broker ได้อย่างถูกต้อง
2. สามารถใช้คำสั่ง `mosquitto_sub` และ `mosquitto_pub` ในการบอกรับและเผยแพร่ข้อความผ่านเครือข่ายได้
3. เข้าใจความแตกต่างและสามารถใช้งานตัวกรองไวลด์การ์ดระดับเดียว (`+`) และหลายระดับ (`#`) ได้
4. สามารถเปรียบเทียบพฤติกรรมการรับส่งข้อมูลที่ระดับ QoS 0, QoS 1 และ QoS 2 พร้อมเข้าใจกลไก Packet Identifier และ ACKs
5. สามารถอธิบายและสาธิตการทำงานของ Retained Message และ Last Will and Testament (LWT) ได้

---

## 2. อุปกรณ์และเครื่องมือที่ใช้
* เครื่องคอมพิวเตอร์ระบบปฏิบัติการ Windows หรือ Linux/macOS
* ซอฟต์แวร์ **Eclipse Mosquitto** (พร้อมเครื่องมือ Client CLI) หรือ Docker Engine โดยโปรแกรมที่จะเรียกใช้ในใบงานนี้คือ 
  - mosquitto.exe   ทำหน้าที่เป็น broker
  - mosquitto_sub.exe   ทำหน้าที่เป็น subscriber
  - mosquitto_pub.exe   ทำหน้าที่เป็น publisher
* ซอฟต์แวร์ MQTTExplorer หรือ Thinksboard หรือ any other MQTT client เพื่อใช้ในการตรวจสอบการทำงานของ MQTT Broker โดยจะใช้ client ใด client หนึ่งก็ได้ 
  - ศึกษาวิธีการใช้โปรแกรม MQTTExplorer ได้จาก  [ภาคผนวก 1 MQTTExplorer_user_guide](10-Appendix-1-MQTTExplorer_user_guide.md)
  - เว็บ Thingsboard สามารถเข้าถึงได้ที่ [thingsboard web client](https://test.thingsboard.io/login)
* หน้าต่างเทอร์มินัล (PowerShell หรือ Bash Terminal) อย่างน้อย 3 หน้าต่าง


<p align="center">
<img src="Images/Lab11_01_01.svg" >
</p>

<p align="center">
<b>รูปที่ L11.01_01</b> แผนภาพการทำงานของ MQTT Broker
</p>

---

## 3. ขั้นตอนการทดลอง

### กิจกรรมที่ 11-1.1: การติดตั้งและเริ่มต้น Mosquitto Broker

#### 1. การติดตั้งโปรแกรม
* **สำหรับ Windows** ให้ดาวน์โหลดตัวติดตั้ง `mosquitto-2.x.x-install-windows-x64.exe` จาก [mosquitto.org](https://mosquitto.org/download/) และติดตั้งลงในเครื่อง

ให้จดบันทึกไว้ด้วยว่า installer ติดตั้งไว้ที่ folder ใด 
เนื่องจากใน Lab จะมีการเรียกใช้โปรแกรมผ่าน `&` ซึ่งเป็น **Call Operator** ใน PowerShell ดังนั้นหากติดตั้งที่อื่น หรือใช้ชื่ออื่น ต้องแก้ไขคำสั่งในใบงานให้ถูกต้อง
> ในใบงานนี้ ใช้ค่า default คือ `"C:\Program Files\mosquitto\mosquitto.exe"` หรือเรียกโดยไม่ต้องใส่ path เป็น `mosquitto.exe` ก็ได้


* **สำหรับ Linux (Ubuntu/Debian)**
  ```bash
  sudo apt update && sudo apt install -y mosquitto mosquitto-clients
  ```
* **หรือรันผ่าน Docker**
  ```bash
  docker run -d --name mosquitto-test -p 1883:1883 eclipse-mosquitto:2
  ```

การเรียกคำสั่ง ให้เรียกตามวิธีการที่เหมาะสมของ Linux  หรือ Mac 

**Linux/Mac**
```bash
mosquitto_sub -h [IP_ADDRESS] -t "home/livingroom/light" -v
```

#### 2. การสร้างไฟล์คอนฟิกูเรชันเฉพาะ (`mosquitto.conf`)
สร้างโฟลเดอร์สำหรับแล็บ เช่น `C:\mqtt_lab\` (หรือ `~/mqtt_lab/`) แล้วสร้างไฟล์ชื่อ `mosquitto.conf`:

```text
# กำหนดพอร์ตและอนุญาตให้ทุกอินเทอร์เฟซเชื่อมต่อได้
listener 1883 0.0.0.0

# อนุญาตการเชื่อมต่อแบบ Anonymous (สำหรับการทดลองในห้องปฏิบัติการ)
allow_anonymous true

# แสดงบันทึกข้อความอย่างละเอียด
log_type all
```


#### 3. เปิด Terminal (Powershell) จำนวน 4 หน้าต่าง

#### 3.1 กรณีใช้ powershell windows

- หน้าต่างที่ 1  สำหรับรัน Mosquitto Broker  ตั้งชื่อหน้าต่างว่า "Mosquitto Broker"
- หน้าต่างที่ 2  สำหรับรัน mosquitto_sub  ตั้งชื่อหน้าต่างว่า "Subscriber"
- หน้าต่างที่ 3  สำหรับรัน mosquitto_pub  ตั้งชื่อหน้าต่างว่า "Publisher"
- หน้าต่างที่ 4  สำหรับรัน mosquitto_pub  ตั้งชื่อหน้าต่างว่า "Publisher 2"

วิธีการตั้งชื่อหน้าต่าง ให้คลิกขวาที่ไอคอนที่แถบหัวข้อ > คลิก "Properties" > คลิก "Options" > ที่ช่อง "Window Title" พิมพ์ชื่อหน้าต่างที่ต้องการ

#### 3.2 กรณีใช้ vsc terminal

เปิด Terminal (Powershell) ใน VSCode  จำนวน 4 หน้าต่าง
เปลี่ยนชื่อโดยการคลิกขวาที่ชื่อ terminal แล้วเลือก `Rename...`

- หน้าต่างที่ 1  สำหรับรัน Mosquitto Broker  ตั้งชื่อหน้าต่างว่า "Mosquitto Broker"
- หน้าต่างที่ 2  สำหรับรัน mosquitto_sub  ตั้งชื่อหน้าต่างว่า "Subscriber"
- หน้าต่างที่ 3  สำหรับรัน mosquitto_pub  ตั้งชื่อหน้าต่างว่า "Publisher"
- หน้าต่างที่ 4  สำหรับรัน mosquitto_pub  ตั้งชื่อหน้าต่างว่า "Publisher 2"


#### 4. เริ่มต้นการทำงานของ Mosquitto Broker
ใน **เทอร์มินัลที่ 1 (Mosquitto Broker)**  สั่งรัน Broker ในโหมดแสดงผลละเอียด (Verbose)

```powershell
# บน Windows (ระบุ Path ไฟล์คอนฟิกที่สร้างไว้)
& mosquitto.exe -c "C:\mqtt_lab\mosquitto.conf" -v

# บน Linux
mosquitto -c ~/mqtt_lab/mosquitto.conf -v
```

**หมายเหตุ** 
1. ถ้ารัน mosquitto.exe บนวินโดวส์ไม่ได้ ให้ตรวจสอบ path ในตัวแปรระบบ โดยเพิ่มเส้นทางการค้นหา "c:\Program Files\Mosquitto\" เข้าไป แล้วเรียก Powershell ใหม่อีกครั้ง 
2. `C:\mqtt_lab\mosquitto.conf` คือไฟล์ที่เก็บ configuration ของ mqtt broker ให้ใส่เส้นทางที่อยู่ของไฟล์จริง ๆ 

> [!WARNING] **ข้อควรระวังสำคัญมากสำหรับ Windows (ปัญหา Mosquitto Service ซ้ำซ้อน / Split-Brain Broker)**
> ตัวติดตั้ง Mosquitto บน Windows จะทำการติดตั้งและเปิดใช้งาน **Windows Service** ชื่อ `mosquitto` ให้รันอยู่เบื้องหลังอัตโนมัติตลอดเวลา ซึ่ง Service ตัวนี้จะผูกกับ `127.0.0.1:1883` 
> หากเปิด Terminal มารัน `mosquitto.exe -c mosquitto.conf -v` อีกตัว จะทำให้เกิด **Broker ทำงานซ้อนกัน 2 ตัวในเครื่องเดียว**:
> * `127.0.0.1` จะวิ่งไปหา Windows Service
> * `0.0.0.0` (IP ของบอร์ด ESP32 วงแลน เช่น `192.168.1.xxx`) จะวิ่งไปหา Terminal Mosquitto
> ส่งผลให้อุปกรณ์ ESP32 และคำสั่ง CLI คุยกันคนละตัวและไม่ส่งข้อมูลถึงกัน!
> 
> **แนวทางปฏิบัติที่ถูกต้อง:** ให้เปิด PowerShell (Run as Administrator) แล้วปิด Service นี้ก่อนเสมอ:
> ```powershell
> Stop-Service mosquitto
> Set-Service mosquitto -StartupType Manual
> ```

ผลลัพธ์ที่ควรเห็นใน **เทอร์มินัล Mosquitto Broker**:
```text
mosquitto version 2.0.18 starting
Config loaded from C:\mqtt_lab\mosquitto.conf.
Opening ipv4 listen socket on port 1883.
mosquitto version 2.0.18 running
```
<img width="1391" height="751" alt="image" src="https://github.com/user-attachments/assets/98ee8fec-ca51-40fc-b039-334e0b0c7b07" />

#### 5. หยุดการทำงานของ Mosquitto Broker
ใน **เทอร์มินัลที่ 1 (Mosquitto Broker)**  ให้กด Ctrl+C เพื่อหยุดการทำงานของ Mosquitto Broker
ในหน้าต่างแสดงผลของ Mosquitto Broker จะปรากฏ Log แสดงการปิดการเชื่อมต่อและหยุดการทำงานของ Broker
```text

...

mosquitto version 2.1.2 terminating
```

---

### กิจกรรมที่ 11-1.2  การทดสอบ Publish / Subscribe พื้นฐาน และการใช้ Wildcards

#### 1. การ Subscribe หัวข้อเฉพาะเจาะจง
ใน **เทอร์มินัลที่ 2 (Subscriber)**  สั่งรันคำสั่งต่อไปนี้
```powershell
& mosquitto_sub.exe -h 127.0.0.1 -p 1883 -t "home/livingroom/light" -v
```

เปิด **เทอร์มินัลที่ 3 (Publisher)**  ส่งข้อความสั่งงานดังนี้
```powershell
& mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "home/livingroom/light" -m "ON"
```

*สังเกตผล*  ในเทอร์มินัลที่ 2  (Subscriber) จะได้รับข้อความ `home/livingroom/light ON`
ถ้าไม่ได้ ให้ตรวจสอบว่าทั้งสอง terminal   รันอยู่ที่  IP Address  และ  Port  เดียวกันหรือไม่
เพราะนี่คือจุดเริ่มต้นของการทำงานพื้นฐานที่ต้องรันได้อย่างถูกต้อง

#### 2. การทดสอบไวลด์การ์ดระดับเดียว (Single-Level Wildcard: `+`)
ใน **เทอร์มินัลที่ 2 (Subscriber)**  กด `Ctrl + C` เพื่อออกจากโปรแกรม subscriber แล้วเปลี่ยนคำสั่ง Subscribe เป็น
```powershell
& mosquitto_sub.exe -h 127.0.0.1 -p 1883 -t "home/+/light" -v
```

ใน **เทอร์มินัลที่ 3 (Publisher)**  ทดลอง Publish ข้อความ 3 ครั้ง
```powershell
& mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "home/livingroom/light" -m "ON"
& mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "home/kitchen/light" -m "OFF"
& mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "home/bedroom/balcony/light" -m "ON"
```

> [!NOTE]
> บันทึกผลการทดลอง สังเกตว่าข้อความใดบ้างที่ปรากฏในเทอร์มินัลที่ 2 และข้อความใดที่ไม่ปรากฏ พร้อมอธิบายเหตุผลว่าทำไม `home/bedroom/balcony/light` จึงไม่แสดงผล

<img width="1472" height="400" alt="image" src="https://github.com/user-attachments/assets/d405c705-cc03-4479-9d9e-99d2915fae67" />

```
ข้อความที่ปรากฏ: home/livingroom/light และ home/kitchen/light
ข้อความที่ไม่ปรากฏ: home/bedroom/balcony/light

เหตุผล: เครื่องหมาย + แทนได้เพียง 1 ระดับ (1 ช่องระหว่างเครื่องหมาย /) เท่านั้น
home/+/light จึงตรงกับ topic ที่มี 3 ระดับ คือ home / ??? / light
แต่ home/bedroom/balcony/light มี 4 ระดับ ช่องกลางมี 2 ระดับคือ bedroom และ balcony
จึงไม่ตรงกับรูปแบบ และ Broker ไม่ส่งข้อความนี้ให้ Subscriber
```

#### 3. การทดสอบไวลด์การ์ดหลายระดับ (Multi-Level Wildcard: `#`)
ใน **เทอร์มินัลที่ 2 (Subscriber)** กด `Ctrl + C` เพื่อออกจากโปรแกรม subscriber แล้วเปลี่ยนคำสั่ง Subscribe เป็น
```powershell
& mosquitto_sub.exe -h 127.0.0.1 -p 1883 -t "home/#" -v
```

ใน **เทอร์มินัลที่ 3 (Publisher)** ทดลอง Publish ข้อความไปยังหัวข้อต่างๆ
```powershell
& mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "home/livingroom/temp" -m "26.4"
& mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "home/bedroom/balcony/light" -m "ON"
```

*สังเกตผล* ทุกข้อความที่ขึ้นต้นด้วย `home/` จะถูกส่งมาแสดงผลทั้งหมด

<img width="1111" height="355" alt="image" src="https://github.com/user-attachments/assets/5698065f-7c93-44de-a32a-4681b3bf3939" />
<img width="1457" height="747" alt="image" src="https://github.com/user-attachments/assets/bf309046-fe77-4040-99eb-b33d03ecd571" />

---

### กิจกรรมที่ 11-1.3 การทดสอบระดับคุณภาพการส่งข้อมูล (QoS 0, QoS 1, QoS 2)

ในกิจกรรมนี้ ให้สังเกตข้อความ Log ใน **เทอร์มินัลที่ 1 (Mosquitto Broker)** ควบคู่กับคำสั่งในเทอร์มินัลที่ 2 และ 3

#### 1. ทดสอบ QoS 0 (At Most Once)
* ในเทอร์มินัลที่ 2 (Subscriber)  `& "C:\Program Files\mosquitto\mosquitto_sub.exe" -h 127.0.0.1 -t "test/qos" -q 0 -v`
* ในเทอร์มินัลที่ 3 (Publisher)  `& "C:\Program Files\mosquitto\mosquitto_pub.exe" -h 127.0.0.1 -t "test/qos" -m "QoS 0 Message" -q 0`
* *หมายเหตุ* การรันโปรแกรมทั้ง subscriber และ publisher จะทำให้หน้าต่างค้างและรอแสดงข้อความที่ publisher ส่งมาให้ ดังนั้นการเปลี่ยนคำสั่ง ต้องกด  `Ctrl+C`  ทุกครั้งเพื่อออกจาก  โปรแกรม subscriber ก่อนเสมอ แล้วเปลี่ยนคำสั่ง subscriber ใหม่ จากนั้นจึงรันคำสั่ง publisher ใหม่
* *สังเกต Log ฝั่ง Broker*  มีเพียงแพ็กเก็ต `PUBLISH` ส่งผ่านไปโดยไม่มีแพ็กเก็ตตอบรับใดๆ

<img width="1457" height="747" alt="image" src="https://github.com/user-attachments/assets/14cf8ae6-dad2-4975-a714-3c1d13386c60" />

#### 2. ทดสอบ QoS 1 (At Least Once)
* ในเทอร์มินัลที่ 2 (Subscriber)  `& "C:\Program Files\mosquitto\mosquitto_sub.exe" -h 127.0.0.1 -t "test/qos" -q 1 -v`
* ในเทอร์มินัลที่ 3 (Publisher)  `& "C:\Program Files\mosquitto\mosquitto_pub.exe" -h 127.0.0.1 -t "test/qos" -m "QoS 1 Message" -q 1`
* *สังเกต Log ฝั่ง Broker*  จะปรากฏแพ็กเก็ต `PUBLISH (d0, q1, m1, ...)` ตามด้วยแพ็กเก็ต `PUBACK (Mid: 1)` เพื่อยืนยันการรับส่งข้อมูล

<img width="1460" height="747" alt="image" src="https://github.com/user-attachments/assets/4012089a-d230-4f49-931c-21b1c639b5f6" />

#### 3. ทดสอบ QoS 2 (Exactly Once)
* ในเทอร์มินัลที่ 2 (Subscriber)  `& mosquitto_sub.exe -h 127.0.0.1 -t "test/qos" -q 2 -v`
* ในเทอร์มินัลที่ 3 (Publisher)  `& mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "test/qos" -m "QoS 2 Message" -q 2`
* *สังเกต Log ฝั่ง Broker*  จะปรากฏกระบวนการ 4-way Handshake อย่างชัดเจน
  1. `Received PUBLISH ... q2`
  2. `Sending PUBREC`
  3. `Received PUBREL`
  4. `Sending PUBCOMP`

<img width="1458" height="750" alt="image" src="https://github.com/user-attachments/assets/8c80dac3-ee1b-4c44-8a40-b5105dbb1f69" />


### ตารางเปรียบเทียบลำดับแพ็กเก็ต QoS 0, 1, 2 จาก Broker Log

| QoS | ช่วง Publisher → Broker | ช่วง Broker → Subscriber | Message ID | จำนวนแพ็กเก็ตทั้งหมด |
|:---:|:---|:---|:---:|:---:|
| **0** (At most once) | PUBLISH | PUBLISH | ไม่มี (m0) | 2 |
| **1** (At least once) | PUBLISH → PUBACK | PUBLISH → PUBACK | m1 | 4 |
| **2** (Exactly once) | PUBLISH → PUBREC → PUBREL → PUBCOMP | PUBLISH → PUBREC → PUBREL → PUBCOMP | m1 | 8 |

**วิเคราะห์จาก Log**
- **QoS 0:** Broker ได้รับ PUBLISH แล้วส่งต่อทันที ไม่มีแพ็กเก็ตตอบรับใดๆ หากหายระหว่างทางก็ไม่มีใครรู้
- **QoS 1:** มี Message ID (m1) และตอบรับด้วย PUBACK ทั้ง 2 ช่วง หากผู้ส่งไม่ได้ PUBACK จะส่งซ้ำ จึงรับประกันว่าถึงอย่างน้อย 1 ครั้ง แต่อาจได้รับซ้ำ
- **QoS 2:** มีการยืนยัน 4 ขั้นในแต่ละช่วง (PUBREC = ได้รับแล้ว, PUBREL = ปล่อยข้อความได้, PUBCOMP = เสร็จสมบูรณ์) จาก Log จะเห็นว่า Broker ส่ง PUBLISH ต่อให้ Subscriber หลังจากได้ PUBREL แล้วเท่านั้น ทำให้รับประกันว่าข้อความถึงเพียงครั้งเดียวไม่ซ้ำ แลกกับจำนวนแพ็กเก็ตที่มากที่สุด (8 แพ็กเก็ต เทียบกับ QoS 0 ที่ใช้ 2)

---

### กิจกรรมที่ 11-1.4 การทดสอบข้อความคงค้าง (Retained Messages)

#### 1. ทดสอบข้อความคงค้าง (Retained Messages)
1. ปิด Subscriber ใน **เทอร์มินัลที่ 2 (Subscriber)** ทั้งหมด (ไม่มีใคร Subscribe อยู่)
2. ใน **เทอร์มินัลที่ 3 (Publisher)** ส่งข้อความธรรมดา (ไม่ใส่พารามิเตอร์ `-r`)
   ```powershell
   & mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "sensor/state" -m "Normal"
   ```
3. เปิด Subscriber ใน **เทอร์มินัลที่ 2 (Subscriber)**
   ```powershell
   & mosquitto_sub.exe -h 127.0.0.1 -p 1883 -t "sensor/state" -v
   ```
   *ผลที่ได้*  หน้าจอว่างเปล่า ไม่ได้รับข้อความใดๆ เพราะข้อความถูกส่งผ่านไปแล้วก่อนหน้านี้

4. ใน **เทอร์มินัลที่ 3 (Publisher)** สั่ง Publish โดยเพิ่มพารามิเตอร์ **`-r` (Retain)**
   ```powershell
   & mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "sensor/state" -m "CRITICAL_ALERT" -r
   ```
   *ผลที่ได้* ในเทอร์มินัลที่ 2 (Subscriber)ได้รับข้อความ `sensor/state CRITICAL_ALERT` ทันที

5. ปิดเทอร์มินัลที่ 2 (Subscriber) (`Ctrl + C`) แล้วรัน subscriber ใหม่อีกครั้ง
   ```powershell
   & mosquitto_sub.exe -h 127.0.0.1 -p 1883 -t "sensor/state" -v
   ```
   *ผลที่ได้* ทันทีที่เชื่อมต่อสำเร็จ เทอร์มินัลที่ 2 (Subscriber) จะได้รับข้อความ `CRITICAL_ALERT` ในทันที แม้ Publisher จะไม่ได้ส่งข้อความใหม่เลยก็ตาม
   
<img width="1472" height="751" alt="image" src="https://github.com/user-attachments/assets/fcf4bf8f-b995-45ea-993d-b7396a37fc63" />

 ```
- ข้อความ "Normal" ส่งโดยไม่ใช้ -r ขณะที่ไม่มี Subscriber จึงไม่ถูกเก็บไว้
  เมื่อเปิด Subscriber ภายหลังจึงไม่ได้รับข้อความนี้
- ข้อความ "CRITICAL_ALERT" ส่งพร้อมแฟล็ก -r (Retain) Subscriber ที่เปิดอยู่ได้รับทันที
- เมื่อปิดแล้วเปิด Subscriber ใหม่ ได้รับ "CRITICAL_ALERT" ทันทีที่เชื่อมต่อ
  แม้ไม่มีใครส่งใหม่ เพราะ Broker เก็บข้อความล่าสุดของ topic นั้นไว้
  และส่งให้ทุก Subscriber ที่เพิ่งเข้ามา
 ```

> [!TIP]
> **วิธีลบ Retained Message ออกจาก Broker**  ส่งข้อความที่มีเนื้อหาว่างเปล่า (Empty Payload) พร้อมเปิดแฟล็ก `-r`
> ```powershell
> & mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "sensor/state" -n -r
> ```

---

### กิจกรรมที่ 11-1.5 การทดสอบพินัยกรรมฉุกเฉิน (Last Will and Testament - LWT)

1. ใน **เทอร์มินัลที่ 2 (Subscriber)** เปิด Subscribe เพื่อรอเฝ้าดูสถานะของโหนด
   ```powershell
   & mosquitto_sub.exe -h 127.0.0.1 -p 1883 -t "node/esp32/status" -v
   ```
2. ใน **เทอร์มินัลที่ 3 (Publisher)** เปิด Client ที่จำลองเป็น ESP32 โดยกำหนดค่า **LWT** ไว้ล่วงหน้า
   ```powershell
   & mosquitto_sub.exe -h 127.0.0.1 -p 1883 -i "Node_Client_01" -t "dummy/listen" --will-topic "node/esp32/status" --will-payload "LOST_CONNECTION_OFFLINE" --will-qos 1
   ```
3. ส่งสถานะเริ่มต้นว่า "ONLINE" จากอุปกรณ์
   (เปิดเทอร์มินัลที่ 4 (Publisher 2) แล้วพิมพ์ `& mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "node/esp32/status" -m "ONLINE"`)
   *สังเกตผล* เทอร์มินัลที่ 2 แสดงสถานะ `ONLINE`

4. **จำลองเหตุการณ์ไฟดับหรือเครือข่ายหลุดกะทันหัน**  
   ในเทอร์มินัลที่ 3 ให้ทำการ **บังคับปิดหน้าต่างเทอร์มินัลทันที** หรือกดปิดโปรเซส เพื่อไม่ให้ Client ส่งแพ็กเก็ต `DISCONNECT`
5. *สังเกตผลในเทอร์มินัลที่ 2*  
   เมื่อ Broker ตรวจพบว่า TCP Connection ขาดหาย Broker จะเป็นผู้ส่งข้อความ
   ```text
   node/esp32/status LOST_CONNECTION_OFFLINE
   ```
   มายังเทอร์มินัลที่ 2 โดยอัตโนมัติ


<img width="1306" height="741" alt="image" src="https://github.com/user-attachments/assets/f4ff6578-9af8-4588-a31c-013bdd32b9c3" />

 ```
ผลการทดสอบ Last Will and Testament (LWT)

Client "Node_Client_01" ลงทะเบียน LWT ไว้กับ Broker ตั้งแต่ตอนเชื่อมต่อ (topic `node/esp32/status`, payload `LOST_CONNECTION_OFFLINE`, QoS 1)
เมื่อปิดหน้าต่างทันทีโดยไม่ส่งแพ็กเก็ต DISCONNECT Broker ตรวจพบว่า TCP Connection ขาดแบบผิดปกติ
จึงเผยแพร่ข้อความ LWT แทน Client โดยอัตโนมัติ ทำให้ Subscriber รู้ทันทีว่าโหนดออฟไลน์
 ```

---

## 4. การบันทึกและวิเคราะห์ผลการทดลอง

ให้นักศึกษาบันทึกภาพหน้าจอ (Screenshots) และตอบคำถามต่อไปนี้ลงในรายงาน
1. ภาพหน้าจอการทำงานของ Mosquitto Broker Verbose Log ขณะที่มีการเชื่อมต่อและ Subscribe ข้อความ
<img width="1391" height="751" alt="image" src="https://github.com/user-attachments/assets/98ee8fec-ca51-40fc-b039-334e0b0c7b07" />
<img width="1111" height="355" alt="image" src="https://github.com/user-attachments/assets/5698065f-7c93-44de-a32a-4681b3bf3939" />


ลำดับเหตุการณ์ที่สังเกตได้จาก Log เมื่อ Subscriber เชื่อมต่อเข้ามา

```
New connection from 127.0.0.1:50656 on port 1883.
New client connected from 127.0.0.1:50656 as auto-B3F1... (p4, c1, k60).
No will message specified.
Sending CONNACK to auto-B3F1... (0, 0)
Received SUBSCRIBE from auto-B3F1...
    test/qos (QoS 0)
Sending SUBACK to auto-B3F1...
```

| ขั้นตอน | แพ็กเก็ต | ความหมาย |
|:---:|:---|:---|
| 1 | New connection | สร้าง TCP Connection ที่พอร์ต 1883 |
| 2 | CONNECT | Client ขอเชื่อมต่อ (p4 = MQTT v3.1.1, c1 = Clean Session, k60 = Keep Alive 60 วินาที) |
| 3 | CONNACK | Broker ตอบรับการเชื่อมต่อ (0 = สำเร็จ) |
| 4 | SUBSCRIBE | Client ขอรับข้อความจาก topic `test/qos` |
| 5 | SUBACK | Broker ยืนยันการ Subscribe |
| 6 | PINGREQ / PINGRESP | Client ส่งสัญญาณ Keep Alive เป็นระยะเพื่อบอกว่ายังเชื่อมต่ออยู่ |

2. ตารางเปรียบเทียบลำดับแพ็กเก็ตที่เกิดขึ้นระหว่าง QoS 0, QoS 1 และ QoS 2 ที่สังเกตได้จาก Broker Log
<img width="1457" height="747" alt="image" src="https://github.com/user-attachments/assets/14cf8ae6-dad2-4975-a714-3c1d13386c60" />
<img width="1460" height="747" alt="image" src="https://github.com/user-attachments/assets/4012089a-d230-4f49-931c-21b1c639b5f6" />
<img width="1458" height="750" alt="image" src="https://github.com/user-attachments/assets/8c80dac3-ee1b-4c44-8a40-b5105dbb1f69" />

| QoS | ช่วง Publisher → Broker | ช่วง Broker → Subscriber | Message ID | จำนวนแพ็กเก็ตทั้งหมด |
|:---:|:---|:---|:---:|:---:|
| **0** (At most once) | PUBLISH | PUBLISH | ไม่มี (m0) | 2 |
| **1** (At least once) | PUBLISH → PUBACK | PUBLISH → PUBACK | m1 | 4 |
| **2** (Exactly once) | PUBLISH → PUBREC → PUBREL → PUBCOMP | PUBLISH → PUBREC → PUBREL → PUBCOMP | m1 | 8 |

**วิเคราะห์จาก Log**
- **QoS 0:** Broker ได้รับ PUBLISH แล้วส่งต่อทันที ไม่มีแพ็กเก็ตตอบรับใดๆ หากหายระหว่างทางก็ไม่มีใครรู้
- **QoS 1:** มี Message ID (m1) และตอบรับด้วย PUBACK ทั้ง 2 ช่วง หากผู้ส่งไม่ได้ PUBACK จะส่งซ้ำ จึงรับประกันว่าถึงอย่างน้อย 1 ครั้ง แต่อาจได้รับซ้ำ
- **QoS 2:** มีการยืนยัน 4 ขั้นในแต่ละช่วง (PUBREC = ได้รับแล้ว, PUBREL = ปล่อยข้อความได้, PUBCOMP = เสร็จสมบูรณ์) จาก Log จะเห็นว่า Broker ส่ง PUBLISH ต่อให้ Subscriber หลังจากได้ PUBREL แล้วเท่านั้น ทำให้รับประกันว่าข้อความถึงเพียงครั้งเดียวไม่ซ้ำ แลกกับจำนวนแพ็กเก็ตที่มากที่สุด (8 แพ็กเก็ต เทียบกับ QoS 0 ที่ใช้ 2)

3. ภาพการทดสอบ Retained Message เมื่อเปิด Subscriber ขึ้นมาใหม่
<img width="1472" height="751" alt="image" src="https://github.com/user-attachments/assets/fcf4bf8f-b995-45ea-993d-b7396a37fc63" />
- ข้อความ `Normal` ส่งโดยไม่ใช้ `-r` ขณะที่ไม่มี Subscriber จึงไม่ถูกเก็บไว้ เมื่อเปิด Subscriber ภายหลังจึงไม่ได้รับข้อความนี้
- ข้อความ `CRITICAL_ALERT` ส่งพร้อมแฟล็ก `-r` (Retain) Subscriber ที่เปิดอยู่ได้รับทันที
- เมื่อปิดแล้วเปิด Subscriber ใหม่ ได้รับ `CRITICAL_ALERT` ทันทีที่เชื่อมต่อ แม้ไม่มีใครส่งใหม่ เพราะ Broker เก็บข้อความล่าสุดของ topic นั้นไว้และส่งให้ทุก Subscriber ที่เพิ่งเข้ามา
- เมื่อล้างด้วยคำสั่ง `-n -r` Subscriber ได้รับ `sensor/state (null)` แสดงว่า Retained Message ถูกลบออกจาก Broker แล้ว

---

## 5. คำถามท้ายการทดลอง (Post-Lab Questions)
1. เพราะเหตุใดสัญลักษณ์ `#` จึงต้องวางไว้ที่ตำแหน่งท้ายสุดของ Topic เสมอ? หากกำหนด Topic Filter เป็น `home/#/light` จะเกิดข้อผิดพลาดอย่างไร?
```
เพราะ `#` หมายถึง "ทุกระดับที่เหลือต่อจากนี้ทั้งหมด" (ศูนย์ระดับหรือมากกว่า) จึงไม่มีขอบเขตสิ้นสุด หากมีระดับอื่นต่อท้ายจะไม่สามารถตีความได้ว่า `#` ครอบคลุมถึงตรงไหน มาตรฐาน MQTT จึงกำหนดให้ `#` อยู่ท้ายสุดและต้องอยู่ระดับของตัวเองเท่านั้น

หากใช้ `home/#/light` ถือเป็น Topic Filter ที่ไม่ถูกต้อง (Invalid) โดย `mosquitto_sub` จะแจ้ง error และไม่ยอม Subscribe หรือ Broker จะปฏิเสธคำขอ SUBSCRIBE และอาจตัดการเชื่อมต่อ หากต้องการกรองแบบนี้ให้ใช้ `+` แทน เช่น `home/+/light` หรือ `home/+/+/light`
```
2. ในระบบงานติดตามยานพาหนะ (GPS Tracking) ที่ส่งพิกัดรถยนต์ทุกๆ 1 วินาที ท่านคิดว่าควรเลือกระดับ QoS ใดระหว่าง QoS 0, 1 หรือ 2 เพราะเหตุใด?
```
ควรเลือก QoS 0 เพราะพิกัดถูกส่งใหม่ทุก 1 วินาที หากพิกัดใดหาย ค่าใหม่ก็จะมาแทนในวินาทีถัดไป การส่งซ้ำพิกัดเก่าจึงไม่มีประโยชน์และเป็นข้อมูลที่ล้าสมัยแล้ว

จากการทดลองพบว่า QoS 0 ใช้เพียง 2 แพ็กเก็ต ขณะที่ QoS 1 ใช้ 4 และ QoS 2 ใช้ถึง 8 แพ็กเก็ต การใช้ QoS 0 จึงประหยัดแบนด์วิดท์ของซิมมือถือ ลดภาระของ Broker เมื่อมียานพาหนะจำนวนมาก และไม่เกิดความหน่วงจากการรอ ACK ทั้งนี้ข้อความสำคัญอื่นในระบบเดียวกัน เช่น การแจ้งเตือนอุบัติเหตุ หรือคำสั่งดับเครื่องยนต์ ควรใช้ QoS 1 แทน
```

3. หาก Client ส่งแพ็กเก็ต `DISCONNECT` ไปยัง Broker อย่างถูกต้องก่อนปิดโปรแกรม ข้อความ Last Will and Testament (LWT) จะถูกเผยแพร่ออกไปหรือไม่ เพราะเหตุใด?
```
ไม่ถูกเผยแพร่ เพราะ LWT ออกแบบมาเพื่อแจ้งการหลุดแบบผิดปกติเท่านั้น เช่น ไฟดับ เครือข่ายขาด หรือโปรแกรมค้างจน Keep Alive หมดเวลา เมื่อ Broker ได้รับแพ็กเก็ต DISCONNECT แสดงว่า Client ตั้งใจออกอย่างปกติ Broker จะลบ Will Message ที่ลงทะเบียนไว้ทิ้งทันที ในทางปฏิบัติ อุปกรณ์ที่ปิดตัวอย่างปกติจึงควร Publish สถานะ `OFFLINE` เองก่อนส่ง DISCONNECT
```
4. Retained Message มีประโยชน์อย่างไรในการออกแบบระบบแดชบอร์ด Smart Home สำหรับผู้ใช้งานที่เพิ่งเปิดแอปพลิเคชันขึ้นมาเป็นครั้งแรก?
```
เมื่อผู้ใช้เปิดแอปและ Subscribe topic ต่างๆ Broker จะส่งค่าล่าสุดที่ถูก Retain ไว้ให้ทันที แอปจึงแสดงสถานะปัจจุบันได้ทันที เช่น ไฟห้องนั่งเล่นเปิดอยู่ อุณหภูมิ 26.4 °C โดยไม่ต้องรอให้อุปกรณ์ส่งค่ารอบถัดไป ซึ่งอาจนานหลายนาทีสำหรับเซนเซอร์ที่ส่งไม่บ่อย

หากไม่มี Retained Message หน้าจอจะว่างเปล่าหรือแสดงค่าไม่ถูกต้องจนกว่าจะมีข้อความใหม่ นอกจากนี้ยังนิยมใช้ Retain ร่วมกับ LWT เพื่อให้สถานะ ONLINE/OFFLINE ของอุปกรณ์แสดงถูกต้องตั้งแต่เปิดแอป
```
