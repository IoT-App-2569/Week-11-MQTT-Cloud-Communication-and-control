# ใบงานการทดลองที่ 11.1
### การติดตั้ง Mosquitto Broker และการทดสอบโปรโตคอล MQTT ด้วย CLI (Pub/Sub, QoS, Retain, LWT)

> [!NOTE] **คำชี้แจง**
> ในใบงานนี้นักศึกษาจะได้เรียนรู้การติดตั้งและตั้งค่า **Eclipse Mosquitto MQTT Broker** บนเครื่องคอมพิวเตอร์เพื่อทำหน้าที่เป็นตัวกลางรับส่งข้อมูลของระบบ IoT จากนั้นใช้เครื่องมือบรรทัดคำสั่ง **`mosquitto_sub`** และ **`mosquitto_pub`** ในการทดลองคุณสมบัติสำคัญของโปรโตคอล MQTT ได้แก่ ระบบหัวข้อ (Topics & Wildcards), ระดับคุณภาพการให้บริการ (QoS 0, 1, 2), ข้อความคงค้าง (Retained Messages) และพินัยกรรมฉุกเฉิน (Last Will and Testament - LWT)

> **ข้อมูลผู้จัดทำ**
> * **รหัสนักศึกษา:** 67030260
> * **อีเมล:** 67030260@kmitl.ac.th

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

![alt text](Images/image-2.png)
![alt text](Images/image-3.png)

**บันทึกผลการทดลองและการวิเคราะห์:**

1. **ข้อความที่ปรากฏในเทอร์มินัลที่ 2 (Subscriber):**
   - `home/livingroom/light ON`
   - `home/kitchen/light OFF`

2. **ข้อความที่ไม่ปรากฏในเทอร์มินัลที่ 2 (Subscriber):**
   - `home/bedroom/balcony/light ON`

3. **คำอธิบายเหตุผลทางเทคนิค:**
   - จากการทดลอง กำหนด Topic Filter ฝั่ง Subscriber เป็น `home/+/light` ซึ่งมีโครงสร้างความลึกของ Topic ทั้งหมด **3 ระดับ (3 Levels)** คั่นด้วยเครื่องหมาย `/` ได้แก่:
     - Level 1: `home`
     - Level 2: `+` (Single-level Wildcard)
     - Level 3: `light`
   - ตามมาตรฐานโปรโตคอล MQTT สัญลักษณ์ `+` เป็น **Single-level Wildcard** ที่สามารถแทนที่ข้อความในระดับชั้นนั้นได้เพียง **1 ลำดับขั้นเท่านั้น** (ในที่นี้คือแทนชื่อห้อง เช่น `livingroom` หรือ `kitchen` ได้อย่างสมบูรณ์)
   - แต่สำหรับหัวข้อ `home/bedroom/balcony/light` มีความลึกของ Topic ทั้งหมด **4 ระดับ** โดยในช่วงระหว่าง `home/` กับ `/light` มีระดับย่อยซ้อนกันอยู่ถึง 2 ระดับ คือ `bedroom` และ `balcony`
   - จึงทำให้โครงสร้างระดับของ Topic ไม่ตรงกับตัวกรอง (Pattern Mismatch) ส่งผลให้ Mosquitto Broker กรองทิ้งและไม่ส่งต่อ (Forward) ข้อความดังกล่าวมายัง Subscriber ตัวนี้ (หากต้องการให้ครอบคลุมลำดับชั้นย่อยหลายระดับ จะต้องเปลี่ยนไปใช้ Multi-level Wildcard คือสัญลักษณ์ `#` เช่น `home/#` แทน)

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

![alt text](Images/image-4.png)
![alt text](Images/image-5.png)

---

### กิจกรรมที่ 11-1.3 การทดสอบระดับคุณภาพการส่งข้อมูล (QoS 0, QoS 1, QoS 2)

ในกิจกรรมนี้ ให้สังเกตข้อความ Log ใน **เทอร์มินัลที่ 1 (Mosquitto Broker)** ควบคู่กับคำสั่งในเทอร์มินัลที่ 2 และ 3

#### 1. ทดสอบ QoS 0 (At Most Once)
* ในเทอร์มินัลที่ 2 (Subscriber)  `& "C:\Program Files\mosquitto\mosquitto_sub.exe" -h 127.0.0.1 -t "test/qos" -q 0 -v`
* ในเทอร์มินัลที่ 3 (Publisher)  `& "C:\Program Files\mosquitto\mosquitto_pub.exe" -h 127.0.0.1 -t "test/qos" -m "QoS 0 Message" -q 0`
* *หมายเหตุ* การรันโปรแกรมทั้ง subscriber และ publisher จะทำให้หน้าต่างค้างและรอแสดงข้อความที่ publisher ส่งมาให้ ดังนั้นการเปลี่ยนคำสั่ง ต้องกด  `Ctrl+C`  ทุกครั้งเพื่อออกจาก  โปรแกรม subscriber ก่อนเสมอ แล้วเปลี่ยนคำสั่ง subscriber ใหม่ จากนั้นจึงรันคำสั่ง publisher ใหม่
* *สังเกต Log ฝั่ง Broker*  มีเพียงแพ็กเก็ต `PUBLISH` ส่งผ่านไปโดยไม่มีแพ็กเก็ตตอบรับใดๆ



#### 2. ทดสอบ QoS 1 (At Least Once)
* ในเทอร์มินัลที่ 2 (Subscriber)  `& "C:\Program Files\mosquitto\mosquitto_sub.exe" -h 127.0.0.1 -t "test/qos" -q 1 -v`
* ในเทอร์มินัลที่ 3 (Publisher)  `& "C:\Program Files\mosquitto\mosquitto_pub.exe" -h 127.0.0.1 -t "test/qos" -m "QoS 1 Message" -q 1`
* *สังเกต Log ฝั่ง Broker*  จะปรากฏแพ็กเก็ต `PUBLISH (d0, q1, m1, ...)` ตามด้วยแพ็กเก็ต `PUBACK (Mid: 1)` เพื่อยืนยันการรับส่งข้อมูล

#### 3. ทดสอบ QoS 2 (Exactly Once)
* ในเทอร์มินัลที่ 2 (Subscriber)  `& mosquitto_sub.exe -h 127.0.0.1 -t "test/qos" -q 2 -v`
* ในเทอร์มินัลที่ 3 (Publisher)  `& mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "test/qos" -m "QoS 2 Message" -q 2`
* *สังเกต Log ฝั่ง Broker*  จะปรากฏกระบวนการ 4-way Handshake อย่างชัดเจน
  1. `Received PUBLISH ... q2`
  2. `Sending PUBREC`
  3. `Received PUBREL`
  4. `Sending PUBCOMP`

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

---

## 4. การบันทึกและวิเคราะห์ผลการทดลอง

ให้นักศึกษาบันทึกภาพหน้าจอ (Screenshots) และตอบคำถามต่อไปนี้ลงในรายงาน
1. ภาพหน้าจอการทำงานของ Mosquitto Broker Verbose Log ขณะที่มีการเชื่อมต่อและ Subscribe ข้อความ

![alt text](Images/image.png)
![alt text](Images/image-1.png)



2. ตารางเปรียบเทียบลำดับแพ็กเก็ตที่เกิดขึ้นระหว่าง QoS 0, QoS 1 และ QoS 2 ที่สังเกตได้จาก Broker Log

QoS 0

![alt text](Images/image-6.png)
![alt text](Images/image-7.png)
![alt text](Images/image-8.png)

QoS 1

![alt text](Images/image-9.png)
![alt text](Images/image-10.png)
![alt text](Images/image-11.png)

QoS 2

![alt text](Images/image-12.png)
![alt text](Images/image-13.png)
![alt text](Images/image-14.png)

#### **ตารางเปรียบเทียบลำดับแพ็กเก็ตที่เกิดขึ้นระหว่าง QoS 0, QoS 1 และ QoS 2 (สังเกตได้จาก Broker Log)**

| ระดับ QoS | ระดับการรับประกัน (Delivery Guarantee) | ลำดับแพ็กเก็ตฝั่ง Publisher $\leftrightarrow$ Broker | ลำดับแพ็กเก็ตฝั่ง Broker $\leftrightarrow$ Subscriber | Packet ID (Mid) | รูปแบบ Handshake / การตอบรับ | จุดเด่นและข้อจำกัด |
| :---: | :--- | :--- | :--- | :---: | :--- | :--- |
| **QoS 0** | **At most once**<br>(ส่งได้มากที่สุดครั้งเดียว / Fire-and-forget) | 1. `Received PUBLISH` (q0, m0) | 1. `Sending PUBLISH` (q0, m0) | ไม่มี<br>(Mid = 0) | **ไม่มีการตอบรับ (No ACK)**<br>Broker รับแล้วส่งต่อทันที ไม่มีการตรวจสอบว่าปลายทางได้รับหรือไม่ | • ประสิทธิภาพสูงสุด Overhead ต่ำสุด<br>• ข้อมูลอาจสูญหายได้หากเครือข่ายขัดข้อง |
| **QoS 1** | **At least once**<br>(ส่งถึงอย่างน้อยหนึ่งครั้ง) | 1. `Received PUBLISH` (q1, m1)<br>2. `Sending PUBACK` (Mid: 1) | 1. `Sending PUBLISH` (q1, m1)<br>2. `Received PUBACK` (Mid: 1) | มี<br>(ระบุเลข Mid) | **2-Way Handshake**<br>`PUBLISH` $\rightarrow$ `PUBACK`<br>มีการตอบรับเพื่อยืนยันว่าได้รับข้อความแล้ว | • การันตีว่าข้อความไม่สูญหาย<br>• อาจเกิดข้อความซ้ำ (Duplicate) ได้หาก ACK ส่งกลับไม่ทัน |
| **QoS 2** | **Exactly once**<br>(ส่งถึงครั้งเดียวอย่างแน่นอน) | 1. `Received PUBLISH` (q2, m1)<br>2. `Sending PUBREC` (Mid: 1)<br>3. `Received PUBREL` (Mid: 1)<br>4. `Sending PUBCOMP` (Mid: 1) | 1. `Sending PUBLISH` (q2, m1)<br>2. `Received PUBREC` (Mid: 1)<br>3. `Sending PUBREL` (Mid: 1)<br>4. `Received PUBCOMP` (Mid: 1) | มี<br>(ระบุเลข Mid) | **4-Way Handshake**<br>1. `PUBLISH` (ส่งข้อความ)<br>2. `PUBREC` (รับทราบข้อความ)<br>3. `PUBREL` (ปล่อย/เคลียร์สถานะ)<br>4. `PUBCOMP` (เสร็จสมบูรณ์) | • ปลอดภัยสูงสุด ข้อมูลไม่สูญหายและไม่ซ้ำซ้อนแน่นอน<br>• มี Network Overhead และ Latency สูงสุด |

#### **บทวิเคราะห์ผลการทดลองเชิงลึกจาก Broker Log:**
1. **การสังเกต Packet Identifier (Mid):**
   - ใน **QoS 0** จะพบพารามิเตอร์ `m0` (Mid = 0) เสมอ เนื่องจากไม่มีความจำเป็นต้องอ้างอิงเลขแพ็กเก็ตเพื่อตอบรับ
   - ใน **QoS 1 และ QoS 2** จะมีการสร้าง Packet Identifier เช่น `m1`, `Mid: 1` ขึ้นมา เพื่อใช้ในการจับคู่กับแพ็กเก็ตตอบรับ ป้องกันความสับสนเมื่อมีการส่งข้อมูลหลายชุดพร้อมกัน
2. **การทำงานของกลไกการรับประกัน (Handshake Flow):**
   - **QoS 0:** มีลักษณะเป็น *Unacknowledged Transmission* ส่งผ่านแล้วจบการทำงานทันที
   - **QoS 1:** เกิดกระบวนการยืนยันแบบ 2 ขั้นตอน เมื่อ Broker ได้รับ `PUBLISH` จะส่ง `PUBACK` กลับไปแจ้ง Publisher ทันที และทำแบบเดียวกันกับฝั่ง Subscriber
   - **QoS 2:** เกิดกระบวนการ 4-way Handshake อย่างชัดเจนใน Log โดยแบ่งเป็น 2 เฟส คือ เฟสการันตีการรับส่งข้อความ (`PUBLISH` $\rightarrow$ `PUBREC`) และเฟสการันตีการเคลียร์สถานะไม่ให้ประมวลผลซ้ำ (`PUBREL` $\rightarrow$ `PUBCOMP`) ทำให้เป็นระดับที่มีความน่าเชื่อถือสูงที่สุดตามมาตรฐาน MQTT Specification



3. ภาพการทดสอบ Retained Message เมื่อเปิด Subscriber ขึ้นมาใหม่

![alt text](Images/image-15.png)


---

## 5. คำถามท้ายการทดลอง (Post-Lab Questions)

1. **เพราะเหตุใดสัญลักษณ์ `#` จึงต้องวางไว้ที่ตำแหน่งท้ายสุดของ Topic เสมอ? หากกำหนด Topic Filter เป็น `home/#/light` จะเกิดข้อผิดพลาดอย่างไร?**
   - **คำตอบ:** 
     - ตามข้อกำหนดมาตรฐาน **MQTT Specification (OASIS Standard)** สัญลักษณ์ `#` ถูกนิยามให้เป็น **Multi-level Wildcard** ซึ่งมีหน้าที่จับคู่ระดับชั้นของหัวข้อ (Topic Levels) ทั้งหมดที่อยู่ถัดจากมันไปจนสุดโครงสร้าง (Match all remaining levels downwards) ดังนั้น เพื่อความเป็นเอกภาพและไม่ขัดแย้งในขั้นตอนการทำ Topic Matching โปรโตคอลจึงบังคับว่า `#` จะต้องอยู่ตำแหน่งสุดท้ายของ Topic Filter เท่านั้น (เช่น `home/#` หรือ `#`)
     - **ข้อผิดพลาดที่เกิดขึ้นหากใช้ `home/#/light`:** จะถือว่าเป็น **ไวยากรณ์ผิดพลาด (Malformed/Invalid Topic Filter)** ตามมาตรฐาน MQTT โดยตัว Mosquitto Broker จะปฏิเสธการ Subscribe ทันที และตอบกลับด้วยแพ็กเก็ต `SUBACK` ที่มี Return Code ความล้มเหลว (Failure Code เช่น `0x80`) หรืออาจตัดการเชื่อมต่อทันทีเนื่องจากเกิด Protocol Error ทำให้ Client ไม่สามารถสมัครรับข้อมูลในหัวข้อดังกล่าวได้

2. **ในระบบงานติดตามยานพาหนะ (GPS Tracking) ที่ส่งพิกัดรถยนต์ทุกๆ 1 วินาที ท่านคิดว่าควรเลือกระดับ QoS ใดระหว่าง QoS 0, 1 หรือ 2 เพราะเหตุใด?**
   - **คำตอบ:** 
     - **ควรเลือกใช้ QoS 0 (At most once)**
     - **เหตุผลประกอบ:**
       1. **ข้อมูลมีความถี่สูงและให้ความสำคัญกับค่าปัจจุบันมากที่สุด (High Frequency & Transient Data):** พิกัด GPS มีการอัปเดตทุก 1 วินาที หากมีแพ็กเก็ตใดสูญหายไปในบางวินาที ข้อมูลใหม่ในอีก 1 วินาทีถัดไปก็จะถูกส่งมาทดแทนทันที จึงไม่มีความจำเป็นต้องเสียเวลาและแบนด์วิดท์ในการส่งซ้ำ (Retransmission)
       2. **ลดภาระเครือข่ายและประหยัดพลังงาน (Network & Resource Efficiency):** ระบบติดตามยานพาหนะมักทำงานผ่านเครือข่าย Cellular (4G/5G/NB-IoT) ซึ่งมีข้อจำกัดเรื่องแบนด์วิดท์ การใช้ QoS 0 ไม่มี Packet Identifier และไม่มี Overhead จาก Handshake/ACK (`PUBACK`, `PUBREC`) ช่วยประหยัดค่าใช้จ่ายด้าน Data และลดภาระของไมโครคอนโทรลเลอร์
       3. **ป้องกันปัญหาคอขวดและความหน่วงสะสม (Avoid Latency Accumulation):** หากใช้ QoS 1 หรือ QoS 2 ในช่วงที่รถขับเข้าจุดอับสัญญาณ อุปกรณ์จะพยายามส่งแพ็กเก็ตเดิมซ้ำๆ และต้องรอ Handshake ซึ่งจะทำให้เกิดคิวข้อมูลสะสม (Head-of-Line Blocking) ส่งผลให้พิกัดที่แสดงบนหน้าจอดีเลย์ ไม่ตรงกับเวลาจริง (Non-Realtime)

3. **หาก Client ส่งแพ็กเก็ต `DISCONNECT` ไปยัง Broker อย่างถูกต้องก่อนปิดโปรแกรม ข้อความ Last Will and Testament (LWT) จะถูกเผยแพร่ออกไปหรือไม่ เพราะเหตุใด?**
   - **คำตอบ:** 
     - **ข้อความ LWT จะไม่ถูกเผยแพร่ออกไป (Will NOT be published)**
     - **เหตุผลประกอบ:**
       - กลไก **LWT (Last Will and Testament)** ถูกออกแบบมาเพื่อแจ้งเตือนสถานะเมื่อเกิด **"การขาดการเชื่อมต่ออย่างผิดปกติ وغيرคาดคิด" (Ungraceful / Abnormal Disconnection)** เช่น อุปกรณ์ไฟดับ กะทันหัน แบตเตอรี่หมด สัญญาณเครือข่ายขาดหาย หรือแอปพลิเคชัน Crash จนไม่สามารถส่ง PINGREQ ตามรอบเวลา Keep Alive ได้
       - แต่กรณีที่ Client ส่งแพ็กเก็ต `DISCONNECT` จะถือว่าเป็น **"การตัดการเชื่อมต่ออย่างปกติและตั้งใจ (Graceful Disconnection)"** เมื่อ Broker ได้รับแพ็กเก็ตนี้ จะรับรู้ว่า Client จบการทำงานตามปกติ จึงทำการเคลียร์ Session และ **ยกเลิก/ลบทิ้ง (Discard) ข้อความ LWT ทันที** โดยไม่เผยแพร่ไปยัง Subscriber อื่นๆ

4. **Retained Message มีประโยชน์อย่างไรในการออกแบบระบบแดชบอร์ด Smart Home สำหรับผู้ใช้งานที่เพิ่งเปิดแอปพลิเคชันขึ้นมาเป็นครั้งแรก?**
   - **คำตอบ:** 
     - **ประโยชน์สำคัญ:** ช่วยแก้ปัญหา **"หน้าจอแดชบอร์ดว่างเปล่า (Blank Dashboard Problem)"** และทำให้ผู้ใช้สามารถทราบ **"สถานะล่าสุดของอุปกรณ์ได้ในทันที (Immediate State Synchronization)"**
     - **เหตุผลและหลักการทำงาน:**
       - โดยปกติอุปกรณ์ IoT ในบ้าน เช่น สวิตช์ไฟ ประตู หรือแอร์ จะส่งข้อมูลแบบตามเหตุการณ์ (Event-driven) คือส่งเฉพาะตอนที่มีการเปิด/ปิด หรือส่งตามรอบเวลาที่ค่อนข้างห่าง (เช่น ทุก 5-15 นาที เพื่อประหยัดพลังงาน)
       - หากไม่ใช้ Retained Message เมื่อผู้ใช้เปิดแอป Smart Home ขึ้นมาเป็นครั้งแรก แอปจะไม่มีข้อมูลสถานะใดๆ แสดงเลย จนกว่าจะรอให้อุปกรณ์นั้นๆ มีการกดสวิตช์หรือส่งข้อมูลรอบถัดไป
       - การกำหนดให้ส่งข้อความสถานะด้วยแฟล็ก `retain = true` จะทำให้ Broker บันทึกข้อความล่าสุดนั้นเก็บไว้บนเซิร์ฟเวอร์ ทันทีที่แอปเปิดขึ้นมาและทำ Subscribe (เช่น `home/+/status`) ตัว Broker จะส่งข้อมูลสถานะล่าสุดให้อัตโนมัติทันที ทำให้แดชบอร์ดแสดงสถานะเปิด/ปิดของไฟทุกดวงได้สมบูรณ์โดยไม่ต้องรอให้อุปกรณ์ส่งข้อมูลใหม่

