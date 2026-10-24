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
 ```
PS C:\Users\HP\Documents\Week-11-MQTT-Cloud-Communication-and-control> & mosquitto.exe -c "C:\mqtt_lab\mosquitto.conf" -v
1792837553: mosquitto version 2.1.2 starting
1792837553: Config loaded from C:\mqtt_lab\mosquitto.conf.
1792837553: Bridge support available.
1792837553: Persistence support available.
1792837553: TLS support available.
1792837553: TLS-PSK support available.
1792837553: Websockets support available.
1792837553: Opening ipv4 listen socket on port 1883.
1792837553: mosquitto version 2.1.2 running
1792837564: New connection from 127.0.0.1:62994 on port 1883.
1792837564: New client connected from 127.0.0.1:62994 as auto-3F45AB65-A2CB-C043-4AE5-59633C435798 (p4, c1, k60).
1792837564: No will message specified.
1792837564: Sending CONNACK to auto-3F45AB65-A2CB-C043-4AE5-59633C435798 (0, 0)
1792837564: Received SUBSCRIBE from auto-3F45AB65-A2CB-C043-4AE5-59633C435798
1792837564:     home/livingroom/light (QoS 0)
1792837564: auto-3F45AB65-A2CB-C043-4AE5-59633C435798 0 home/livingroom/light
1792837564: Sending SUBACK to auto-3F45AB65-A2CB-C043-4AE5-59633C435798
1792837624: Received PINGREQ from auto-3F45AB65-A2CB-C043-4AE5-59633C435798
1792837624: Sending PINGRESP to auto-3F45AB65-A2CB-C043-4AE5-59633C435798
1792837630: New connection from 127.0.0.1:63016 on port 1883.
1792837630: New client connected from 127.0.0.1:63016 as auto-B81709EB-7074-C593-8A2A-83340E5DBFB2 (p4, c1, k60).
1792837630: No will message specified.
1792837630: Sending CONNACK to auto-B81709EB-7074-C593-8A2A-83340E5DBFB2 (0, 0)
1792837630: Received PUBLISH from auto-B81709EB-7074-C593-8A2A-83340E5DBFB2 (d0, q0, r0, m0, 'home/livingroom/light', ... (2 bytes))
1792837630: Sending PUBLISH to auto-3F45AB65-A2CB-C043-4AE5-59633C435798 (d0, q0, r0, m0, 'home/livingroom/light', ... (2 bytes))
1792837630: Received DISCONNECT from auto-B81709EB-7074-C593-8A2A-83340E5DBFB2
1792837630: Client auto-B81709EB-7074-C593-8A2A-83340E5DBFB2 [127.0.0.1:63016] disconnected.
1792837684: Received PINGREQ from auto-3F45AB65-A2CB-C043-4AE5-59633C435798
1792837684: Sending PINGRESP to auto-3F45AB65-A2CB-C043-4AE5-59633C435798
1792837744: Received PINGREQ from auto-3F45AB65-A2CB-C043-4AE5-59633C435798
1792837744: Sending PINGRESP to auto-3F45AB65-A2CB-C043-4AE5-59633C435798
1792837804: Received PINGREQ from auto-3F45AB65-A2CB-C043-4AE5-59633C435798
1792837804: Sending PINGRESP to auto-3F45AB65-A2CB-C043-4AE5-59633C435798
1792837864: Received PINGREQ from auto-3F45AB65-A2CB-C043-4AE5-59633C435798
1792837864: Sending PINGRESP to auto-3F45AB65-A2CB-C043-4AE5-59633C435798
1792837923: New connection from 127.0.0.1:61345 on port 1883.
1792837923: New client connected from 127.0.0.1:61345 as auto-B08A1D33-CC86-A393-8158-71DC9141D39B (p4, c1, k60).
1792837923: No will message specified.
1792837923: Sending CONNACK to auto-B08A1D33-CC86-A393-8158-71DC9141D39B (0, 0)
1792837923: Received PUBLISH from auto-B08A1D33-CC86-A393-8158-71DC9141D39B (d0, q0, r0, m0, 'home/livingroom/light', ... (2 bytes))
1792837923: Sending PUBLISH to auto-3F45AB65-A2CB-C043-4AE5-59633C435798 (d0, q0, r0, m0, 'home/livingroom/light', ... (2 bytes))
1792837923: Received DISCONNECT from auto-B08A1D33-CC86-A393-8158-71DC9141D39B
1792837923: Client auto-B08A1D33-CC86-A393-8158-71DC9141D39B [127.0.0.1:61345] disconnected.
1792837923: Received PINGREQ from auto-3F45AB65-A2CB-C043-4AE5-59633C435798
1792837923: Sending PINGRESP to auto-3F45AB65-A2CB-C043-4AE5-59633C435798
1792837924: New connection from 127.0.0.1:61348 on port 1883.
1792837924: New client connected from 127.0.0.1:61348 as auto-F1134427-D0C3-D3BB-6FAC-409A661A671D (p4, c1, k60).
1792837924: No will message specified.
1792837924: Sending CONNACK to auto-F1134427-D0C3-D3BB-6FAC-409A661A671D (0, 0)
1792837924: Received SUBSCRIBE from auto-F1134427-D0C3-D3BB-6FAC-409A661A671D
1792837924:     home/livingroom/light (QoS 0)
1792837924: auto-F1134427-D0C3-D3BB-6FAC-409A661A671D 0 home/livingroom/light
1792837924: Sending SUBACK to auto-F1134427-D0C3-D3BB-6FAC-409A661A671D
1792837931: Client auto-F1134427-D0C3-D3BB-6FAC-409A661A671D [127.0.0.1:61348] disconnected: connection closed by client.
1792837984: Received PINGREQ from auto-3F45AB65-A2CB-C043-4AE5-59633C435798
1792837984: Sending PINGRESP to auto-3F45AB65-A2CB-C043-4AE5-59633C435798
1792837984: New connection from 127.0.0.1:61371 on port 1883.
1792837984: New client connected from 127.0.0.1:61371 as auto-C005C648-0FF8-0EFF-3CBC-148252C69DFD (p4, c1, k60).
1792837984: No will message specified.
1792837984: Sending CONNACK to auto-C005C648-0FF8-0EFF-3CBC-148252C69DFD (0, 0)
1792837984: Received SUBSCRIBE from auto-C005C648-0FF8-0EFF-3CBC-148252C69DFD
1792837984:     home/livingroom/light (QoS 0)
1792837984: auto-C005C648-0FF8-0EFF-3CBC-148252C69DFD 0 home/livingroom/light
1792837984: Sending SUBACK to auto-C005C648-0FF8-0EFF-3CBC-148252C69DFD
1792837985: New connection from 127.0.0.1:61374 on port 1883.
1792837985: New client connected from 127.0.0.1:61374 as auto-1C66EE21-A529-1B20-32DC-D9F7DFF1165D (p4, c1, k60).
1792837985: No will message specified.
1792837985: Sending CONNACK to auto-1C66EE21-A529-1B20-32DC-D9F7DFF1165D (0, 0)
1792837985: Received PUBLISH from auto-1C66EE21-A529-1B20-32DC-D9F7DFF1165D (d0, q0, r0, m0, 'home/livingroom/light', ... (2 bytes))
1792837985: Sending PUBLISH to auto-3F45AB65-A2CB-C043-4AE5-59633C435798 (d0, q0, r0, m0, 'home/livingroom/light', ... (2 bytes))
1792837985: Sending PUBLISH to auto-C005C648-0FF8-0EFF-3CBC-148252C69DFD (d0, q0, r0, m0, 'home/livingroom/light', ... (2 bytes))
1792837985: Received DISCONNECT from auto-1C66EE21-A529-1B20-32DC-D9F7DFF1165D
1792837985: Client auto-1C66EE21-A529-1B20-32DC-D9F7DFF1165D [127.0.0.1:61374] disconnected.
1792837985: Client auto-C005C648-0FF8-0EFF-3CBC-148252C69DFD [127.0.0.1:61371] disconnected: connection closed by client.
1792837986: New connection from 127.0.0.1:61377 on port 1883.
1792837986: New client connected from 127.0.0.1:61377 as auto-FAB4D88F-9E05-3A58-6C31-B82E8AD2F732 (p4, c1, k60).
1792837986: No will message specified.
1792837986: Sending CONNACK to auto-FAB4D88F-9E05-3A58-6C31-B82E8AD2F732 (0, 0)
1792837986: Received SUBSCRIBE from auto-FAB4D88F-9E05-3A58-6C31-B82E8AD2F732
1792837986:     home/+/light (QoS 0)
1792837986: auto-FAB4D88F-9E05-3A58-6C31-B82E8AD2F732 0 home/+/light
1792837986: Sending SUBACK to auto-FAB4D88F-9E05-3A58-6C31-B82E8AD2F732
1792837986: New connection from 127.0.0.1:61380 on port 1883.
1792837986: New client connected from 127.0.0.1:61380 as auto-B4AE8CEF-9DA2-6DB5-8B5C-957D841CAD8A (p4, c1, k60).
1792837986: No will message specified.
1792837986: Sending CONNACK to auto-B4AE8CEF-9DA2-6DB5-8B5C-957D841CAD8A (0, 0)
1792837986: Received PUBLISH from auto-B4AE8CEF-9DA2-6DB5-8B5C-957D841CAD8A (d0, q0, r0, m0, 'home/livingroom/light', ... (2 bytes))
1792837986: Sending PUBLISH to auto-3F45AB65-A2CB-C043-4AE5-59633C435798 (d0, q0, r0, m0, 'home/livingroom/light', ... (2 bytes))
1792837986: Sending PUBLISH to auto-FAB4D88F-9E05-3A58-6C31-B82E8AD2F732 (d0, q0, r0, m0, 'home/livingroom/light', ... (2 bytes))
1792837986: Received DISCONNECT from auto-B4AE8CEF-9DA2-6DB5-8B5C-957D841CAD8A
1792837986: Client auto-B4AE8CEF-9DA2-6DB5-8B5C-957D841CAD8A [127.0.0.1:61380] disconnected.
1792837986: New connection from 127.0.0.1:61383 on port 1883.
1792837986: New client connected from 127.0.0.1:61383 as auto-7FED4079-3EA6-345F-417C-5514C82F712D (p4, c1, k60).
1792837986: No will message specified.
1792837986: Sending CONNACK to auto-7FED4079-3EA6-345F-417C-5514C82F712D (0, 0)
1792837986: Received PUBLISH from auto-7FED4079-3EA6-345F-417C-5514C82F712D (d0, q0, r0, m0, 'home/kitchen/light', ... (3 bytes))
1792837986: Sending PUBLISH to auto-FAB4D88F-9E05-3A58-6C31-B82E8AD2F732 (d0, q0, r0, m0, 'home/kitchen/light', ... (3 bytes))
1792837986: Received DISCONNECT from auto-7FED4079-3EA6-345F-417C-5514C82F712D
1792837986: Client auto-7FED4079-3EA6-345F-417C-5514C82F712D [127.0.0.1:61383] disconnected.
 ```
2. ตารางเปรียบเทียบลำดับแพ็กเก็ตที่เกิดขึ้นระหว่าง QoS 0, QoS 1 และ QoS 2 ที่สังเกตได้จาก Broker Log

QoS 0 (At Most Once - ส่งเที่ยวเดียวจบ)
ลำดับแพ็กเก็ต: PUBLISH
คำอธิบาย: Broker ส่งต่อข้อความไปหา Subscriber ทันทีโดยไม่มีแพ็กเก็ตตอบรับยืนยันใดๆ (เร็วสุด แต่ไม่มีการันตี)
```
Received PUBLISH from ... (d0, q0, r0, m0, 'test/qos', ... (13 bytes))
Sending PUBLISH to ... (d0, q0, r0, m0, 'test/qos', ... (13 bytes))
```

QoS 1 (At Least Once - ยืนยัน 2 จังหวะ)
ลำดับแพ็กเก็ต: PUBLISH (q1) > PUBACK (Mid: 1)
คำอธิบาย: เมื่อ Publisher ส่งข้อความ Broker จะต้องส่งแพ็กเก็ต PUBACK กลับไปยืนยันเสมอ เพื่อรับประกันว่าข้อความส่งถึงอย่างแน่นอน
```
Received PUBLISH from ... (d0, q1, r0, m1, 'test/qos', ... (13 bytes))
Sending PUBACK to ... (m1, rc0)
Sending PUBLISH to ... (d0, q1, r0, m1, 'test/qos', ... (13 bytes))
Received PUBACK from ... (Mid: 1)
```

QoS 2 (Exactly Once - ยืนยัน 4 จังหวะ / 4-Way Handshake)
ลำดับแพ็กเก็ต: PUBLISH (q2) 
PUBREC > PUBREL > PUBCOMP
คำอธิบาย: มีการคุยโต้ตอบครบ 4 ขั้นตอนเพื่อล็อกและปล่อยข้อความ รับประกันว่าข้อความจะถึงปลายทางถูกต้องแน่นอนและไม่ได้รับซ้ำ 100%
```
Received PUBLISH from ... (d0, q2, r0, m1, 'test/qos', ... (13 bytes))
Sending PUBREC to ... (m1, rc0)
Received PUBREL from ... (Mid: 1)
Sending PUBCOMP to ... (m1)
Sending PUBLISH to ... (d0, q2, r0, m1, 'test/qos', ... (13 bytes))
Received PUBREC from ... (Mid: 1)
Sending PUBREL to ... (m1)
Received PUBCOMP from ... (Mid: 1)
```
3. ภาพการทดสอบ Retained Message เมื่อเปิด Subscriber ขึ้นมาใหม่

<img width="1156" height="102" alt="image" src="https://github.com/user-attachments/assets/a1024904-a6f9-4982-9aac-aebaabe08874" />

---

## 5. คำถามท้ายการทดลอง (Post-Lab Questions)
1. เพราะเหตุใดสัญลักษณ์ `#` จึงต้องวางไว้ที่ตำแหน่งท้ายสุดของ Topic เสมอ? หากกำหนด Topic Filter เป็น `home/#/light` จะเกิดข้อผิดพลาดอย่างไร?

- # แทนทุกระดับชั้นที่เหลือจนสุดสาย จึงมีหัวข้อย่อยตามหลังอีกไม่ได้ ไวยากรณ์ผิดทำให้ Broker ปฏิเสธการ Subscribe 

2. ในระบบงานติดตามยานพาหนะ (GPS Tracking) ที่ส่งพิกัดรถยนต์ทุกๆ 1 วินาที ท่านคิดว่าควรเลือกระดับ QoS ใดระหว่าง QoS 0, 1 หรือ 2 เพราะเหตุใด?

- QoS 0 ข้อมูลส่งถี่มากถ้าพิกัดหลุดไป 1 วินาที ข้อมูลใหม่ก็เข้ามาแทนที่ QoS 0 ส่งเร็วสุด ไม่กินเน็ต และไม่มีขั้นตอนส่งซ้ำให้ระบบหน่วง

3. หาก Client ส่งแพ็กเก็ต `DISCONNECT` ไปยัง Broker อย่างถูกต้องก่อนปิดโปรแกรม ข้อความ Last Will and Testament (LWT) จะถูกเผยแพร่ออกไปหรือไม่ เพราะเหตุใด?

- DISCONNECT เป็นการปิดตามปกติ Broker จะยกเลิกพินัยกรรมทิ้งทันที โดย LWT จะทำงานเฉพาะตอนที่อุปกรณ์ "หลุดแบบผิดปกติ" (ไฟดับ, เน็ตหลุด, ค้าง) เท่านั้น

4. Retained Message มีประโยชน์อย่างไรในการออกแบบระบบแดชบอร์ด Smart Home สำหรับผู้ใช้งานที่เพิ่งเปิดแอปพลิเคชันขึ้นมาเป็นครั้งแรก?

- ช่วยให้แอป แสดงสถานะล่าสุดของอุปกรณ์ได้ทันทีที่เปิดใช้งาน (เช่น ไฟเปิดอยู่กี่ดวง, อุณหภูมิล่าสุด) โดยที่ผู้ใช้ไม่ต้องรอให้อุปกรณ์ส่งข้อมูลรอบถัดไปขึ้นมา
