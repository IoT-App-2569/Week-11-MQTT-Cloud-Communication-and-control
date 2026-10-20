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

![alt text](image.png)
![alt text](image-1.png)

## ผลที่สังเกตได้

| Topic ที่ส่ง | แสดงใน Subscriber? |
|---|---|
| `home/livingroom/light` | ✅ แสดง (`ON`) |
| `home/kitchen/light` | ✅ แสดง (`OFF`) |
| `home/bedroom/balcony/light` | ❌ ไม่แสดง |

ผลที่เห็นในเทอร์มินัลที่ 2:

```text
home/livingroom/light ON
home/kitchen/light OFF
```

## เหตุผลที่ `home/bedroom/balcony/light` ไม่แสดง

เครื่องหมาย `+` ใช้แทนได้ **เพียง 1 ระดับ** ของ topic เท่านั้น

| | ระดับ 1 | ระดับ 2 | ระดับ 3 | ระดับ 4 |
|---|---|---|---|---|
| Filter `home/+/light` | home | `+` | light | - |
| `home/kitchen/light` | home | kitchen | light | - |
| `home/bedroom/balcony/light` | home | bedroom | balcony | light |

- Filter มี **3 ระดับ** แต่ topic `home/bedroom/balcony/light` มี **4 ระดับ**
- `+` แทนได้แค่ `bedroom` ทำให้ระดับที่ 3 คือ `balcony` ต้องไปเทียบกับ `light` ซึ่งไม่ตรงกัน
- จึงไม่ตรงกับ filter และ Broker ไม่ส่งข้อความนี้ให้ Subscriber

## สรุป

`+` ตรงกับได้ **1 ระดับพอดี** ถ้าต้องการรับทุกระดับที่ลึกกว่านั้น ให้ใช้ `#` (เช่น `home/#`)




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

![alt text](image-2.png)
![alt text](image-3.png)

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

![alt text](image-14.png)
![alt text](image-15.png)
![alt text](image-16.png)

2. ตารางเปรียบเทียบลำดับแพ็กเก็ตที่เกิดขึ้นระหว่าง QoS 0, QoS 1 และ QoS 2 ที่สังเกตได้จาก Broker Log

- QoS 0
![alt text](image-4.png)
![alt text](image-5.png)
![alt text](image-6.png)

- QoS 1
![alt text](image-7.png)
![alt text](image-8.png)
![alt text](image-9.png)

- QoS 2
![alt text](image-10.png)
![alt text](image-11.png)
![alt text](image-12.png)



# ตารางเปรียบเทียบลำดับแพ็กเก็ต QoS 0, QoS 1 และ QoS 2 จาก Broker Log

> Topic ที่ทดสอบ: `test/qos` | Publisher และ Subscriber ใช้ QoS ระดับเดียวกันในแต่ละการทดลอง
> (ชื่อ client ในรูปจริงเป็นชื่อสุ่ม `auto-xxxx` ในตารางนี้ย่อเป็น Pub / Sub)

1. ตารางสรุป

| QoS | ชื่อระดับ | แพ็กเก็ตฝั่ง Pub ↔ Broker | แพ็กเก็ตฝั่ง Broker ↔ Sub | รวมแพ็กเก็ตใน Log |
|---|---|---|---|---|
| 0 | At Most Once | `PUBLISH` | `PUBLISH` | 2 |
| 1 | At Least Once | `PUBLISH` → `PUBACK` | `PUBLISH` → `PUBACK` | 4 |
| 2 | Exactly Once | `PUBLISH` → `PUBREC` → `PUBREL` → `PUBCOMP` | `PUBLISH` → `PUBREC` → `PUBREL` → `PUBCOMP` | 8 |

2. ลำดับแพ็กเก็ตที่เกิดขึ้นจริงใน Broker Log

QoS 0 (At Most Once)

| ลำดับ | Log ที่เห็น | ทิศทาง |
|---|---|---|
| 1 | `Received PUBLISH from Pub (d0, q0, r0, m0, 'test/qos')` | Pub → Broker |
| 2 | `Sending PUBLISH to Sub (d0, q0, r0, m0, 'test/qos')` | Broker → Sub |

ไม่มีแพ็กเก็ตตอบรับใดๆ และไม่มี Packet ID (`m0`)

QoS 1 (At Least Once)

| ลำดับ | Log ที่เห็น | ทิศทาง |
|---|---|---|
| 1 | `Received PUBLISH from Pub (d0, q1, r0, m1, 'test/qos')` | Pub → Broker |
| 2 | `Sending PUBLISH to Sub (d0, q1, r0, m1, 'test/qos')` | Broker → Sub |
| 3 | `Sending PUBACK to Pub (m1, rc0)` | Broker → Pub |
| 4 | `Received PUBACK from Sub (Mid: 1, RC:0)` | Sub → Broker |

มี Packet ID (`m1` / `Mid: 1`) และ `PUBACK` ยืนยันการรับ 1 ครั้งในแต่ละฝั่ง

QoS 2 (Exactly Once)

| ลำดับ | Log ที่เห็น | ทิศทาง |
|---|---|---|
| 1 | `Received PUBLISH from Pub (d0, q2, r0, m1, 'test/qos')` | Pub → Broker |
| 2 | `Sending PUBREC to Pub (m1, rc0)` | Broker → Pub |
| 3 | `Received PUBREL from Pub (Mid: 1)` | Pub → Broker |
| 4 | `Sending PUBLISH to Sub (d0, q2, r0, m1, 'test/qos')` | Broker → Sub |
| 5 | `Sending PUBCOMP to Pub (m1)` | Broker → Pub |
| 6 | `Received PUBREC from Sub (Mid: 1)` | Sub → Broker |
| 7 | `Sending PUBREL to Sub (m1)` | Broker → Sub |
| 8 | `Received PUBCOMP from Sub (Mid: 1, RC:0)` | Sub → Broker |

ฝั่ง Publisher เป็น 4-way handshake ตามใบงาน (ลำดับ 1, 2, 3, 5) และฝั่ง Subscriber ทำ handshake แบบเดียวกันอีกชุด (ลำดับ 4, 6, 7, 8)

3. วิเคราะห์ผล

| QoS | ข้อสังเกตจาก Log | ผลที่ตามมา |
|---|---|---|
| 0 | ไม่มี ACK ไม่มี Packet ID | เร็วที่สุด แต่ถ้าข้อความหายระหว่างทางจะไม่มีใครรู้ |
| 1 | มี `PUBACK` ตอบรับ | ผู้ส่งรู้ว่าข้อความถึงแล้ว ถ้าไม่ได้ ACK จะส่งซ้ำ จึงอาจได้ข้อความซ้ำ |
| 2 | มี `PUBREC` → `PUBREL` → `PUBCOMP` | ข้อความถึงครั้งเดียวพอดี ไม่ซ้ำ ไม่หาย แต่มีแพ็กเก็ตมากที่สุดและช้าที่สุด |

4. สรุป

ยิ่งระดับ QoS สูง จำนวนแพ็กเก็ตยืนยันยิ่งมาก (2 → 4 → 8 แพ็กเก็ต) ทำให้ข้อความน่าเชื่อถือขึ้นแต่ใช้เวลาและแบนด์วิดท์มากขึ้น
- **QoS 0** เหมาะกับข้อมูลที่ส่งถี่และหายได้ เช่น ค่าเซนเซอร์
- **QoS 1** เหมาะกับคำสั่งทั่วไป ที่รับซ้ำได้
- **QoS 2** เหมาะกับงานที่ห้ามซ้ำห้ามหาย เช่น การชำระเงิน


3. ภาพการทดสอบ Retained Message เมื่อเปิด Subscriber ขึ้นมาใหม่

![alt text](image-13.png)
---

## 5. คำถามท้ายการทดลอง (Post-Lab Questions)
1. เพราะเหตุใดสัญลักษณ์ `#` จึงต้องวางไว้ที่ตำแหน่งท้ายสุดของ Topic เสมอ? หากกำหนด Topic Filter เป็น `home/#/light` จะเกิดข้อผิดพลาดอย่างไร?

**คำตอบ:** `#` แปลว่า "ทุกระดับที่อยู่ถัดลงไปทั้งหมด" ไม่ว่าจะลึกกี่ชั้นก็ตาม เมื่อมันครอบคลุมทุกอย่างที่อยู่หลังจุดนั้นไปแล้ว จึงไม่มีความหมายที่จะมีอะไรต่อท้ายได้อีก มาตรฐาน MQTT จึงกำหนดว่า `#` ต้องเป็นตัวสุดท้ายของ Topic Filter เสมอ

**ถ้าใช้ `home/#/light`:**
- เป็น Topic Filter ที่ **ไม่ถูกต้อง (Invalid)**
- Broker/Client จะ **ปฏิเสธการ Subscribe** ทำให้ไม่ได้รับข้อความใดๆ
- ถ้าต้องการรับ topic ที่ลงท้ายด้วย `light` จากหลายระดับ ต้องใช้ `home/+/light` (ได้แค่ 1 ระดับ) หรือ Subscribe `home/#` แล้วกรองเองที่ฝั่งโปรแกรม

---


2. ในระบบงานติดตามยานพาหนะ (GPS Tracking) ที่ส่งพิกัดรถยนต์ทุกๆ 1 วินาที ท่านคิดว่าควรเลือกระดับ QoS ใดระหว่าง QoS 0, 1 หรือ 2 เพราะเหตุใด?

**คำตอบ: QoS 0**

เหตุผล:
1. **ข้อมูลเก่าหมดค่าเร็ว** พิกัดใหม่มาทุก 1 วินาที ถ้าพิกัดหนึ่งหายไป อีก 1 วินาทีก็มีพิกัดใหม่มาแทนทันที
2. **ประหยัดทรัพยากร** จาก Broker Log ของเรา QoS 0 ใช้แค่ 2 แพ็กเก็ต ส่วน QoS 2 ใช้ถึง 8 แพ็กเก็ต เมื่อส่งทุกวินาทีตลอดเวลา ความต่างนี้สะสมเป็นปริมาณมาก (ทั้งแบตเตอรี่ อินเทอร์เน็ต และภาระ Broker)
3. **QoS 1/2 อาจทำให้เกิดความล่าช้า** เพราะต้องรอ ACK และส่งซ้ำเมื่อไม่ได้รับ ทำให้แผนที่แสดงพิกัดเก่าที่มาช้ากว่าพิกัดล่าสุด ซึ่งไม่มีประโยชน์

> หมายเหตุ: ถ้ามีเหตุการณ์สำคัญที่หายไม่ได้ เช่น แจ้งเตือนอุบัติเหตุ ให้ส่ง topic แยกต่างหากด้วย QoS 1

---

3. หาก Client ส่งแพ็กเก็ต `DISCONNECT` ไปยัง Broker อย่างถูกต้องก่อนปิดโปรแกรม ข้อความ Last Will and Testament (LWT) จะถูกเผยแพร่ออกไปหรือไม่ เพราะเหตุใด?

**คำตอบ: ไม่ถูกเผยแพร่**

เหตุผล:
- LWT ออกแบบมาไว้สำหรับกรณีที่ Client **หลุดโดยไม่ได้ตั้งใจ** (ไฟดับ เน็ตหลุด โปรแกรมค้างหรือถูกฆ่า) ซึ่ง Broker จะเป็นฝ่ายส่งข้อความแทน
- การส่ง `DISCONNECT` คือการ "ลาก่อนอย่างเป็นทางการ" Broker รู้ว่า Client ตั้งใจออก จึง **ทิ้งข้อความ Will โดยไม่ส่ง**

**หลักฐานจากการทดลอง:**
- Client `Node_Client_01` ถูกฆ่าด้วย `pkill -9` (ไม่มี `Received DISCONNECT` ใน log) → Broker ส่ง `LOST_CONNECTION_OFFLINE` (23 bytes) ออกไป
- ถ้ากด `Ctrl + C` จะมีการส่ง `DISCONNECT` และ LWT จะไม่ถูกส่ง

---


4. Retained Message มีประโยชน์อย่างไรในการออกแบบระบบแดชบอร์ด Smart Home สำหรับผู้ใช้งานที่เพิ่งเปิดแอปพลิเคชันขึ้นมาเป็นครั้งแรก?

**คำตอบ:** Retained Message ทำให้แอปได้ **สถานะล่าสุดทันทีที่เชื่อมต่อ** ไม่ต้องรอให้อุปกรณ์ส่งข้อมูลใหม่

**ถ้าไม่มี Retain:**
- เปิดแอปมาแล้วหน้าจอ **ว่างเปล่า** (เหมือนที่เราเห็นในการทดลอง ตอน Subscriber มาทีหลังไม่ได้อะไรเลย)
- ต้องรอจนกว่าอุปกรณ์จะส่งข้อมูลรอบถัดไป ซึ่งอาจเป็นนาทีหรือชั่วโมง ถ้าเป็นอุปกรณ์ที่ส่งเฉพาะตอนสถานะเปลี่ยน เช่น สวิตช์ไฟ ก็อาจไม่ส่งอีกเลย

**ถ้ามี Retain:**
- Broker เก็บค่าล่าสุดของแต่ละ topic ไว้ เช่น `home/livingroom/light = ON`, `home/livingroom/temp = 26.4`
- เมื่อแอปเปิดและ Subscribe ก็จะได้ค่าล่าสุดทันที (เหมือนที่ Subscriber ได้ `CRITICAL_ALERT` ทันทีในการทดลอง)
- แดชบอร์ดแสดงสถานะบ้านได้ครบตั้งแต่วินาทีแรก

**ตัวอย่าง:** เปิดแอปตอนเย็น เห็นทันทีว่าไฟห้องนั่งเล่นเปิดอยู่ อุณหภูมิ 26.4°C ประตูล็อกแล้ว โดยไม่ต้องรอข้อมูลใหม่
