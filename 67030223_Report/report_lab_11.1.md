# Labsheet 11.1: Mosquitto Broker Setup & MQTT Fundamentals (Pub/Sub, QoS, LWT)


#### 1. บันทึกผลการทดลอง / โค้ดที่พัฒนา

ไฟล์คอนฟิกูเรชันสำหรับ Mosquitto Broker (`mosquitto.conf`):

```text
# กำหนดให้รับการเชื่อมต่อที่พอร์ต 1883 จากทุก Network Interface
listener 1883 0.0.0.0

# อนุญาตการเชื่อมต่อแบบ Anonymous สำหรับการทดสอบในห้องปฏิบัติการ
allow_anonymous true

# บันทึกรายละเอียดการทำงานทุกชนิด
log_type all
```

คำสั่งการรัน Broker และคำสั่งทดสอบผ่าน CLI:

```powershell
# หน้าต่างที่ 1: รัน Broker ในโหมด Verbose
& mosquitto.exe -c "mosquitto.conf" -v

# หน้าต่างที่ 2: Subscriber หัวข้อ Wildcard ระดับเดียว (+)
& mosquitto_sub.exe -h 127.0.0.1 -p 1883 -t "home/+/light" -v

# หน้าต่างที่ 3: Publisher ส่งข้อความ
& mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "home/livingroom/light" -m "ON"

# การทดสอบ Retained Message (-r)
& mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "sensor/state" -m "CRITICAL_ALERT" -r

# การทดสอบ Last Will and Testament (LWT)
& mosquitto_sub.exe -h 127.0.0.1 -p 1883 -i "Node_Client_01" -t "dummy/listen" --will-topic "node/esp32/status" --will-payload "LOST_CONNECTION_OFFLINE" --will-qos 1
```

#### 2. ผลลัพธ์จาก Serial Monitor / หน้าจอแสดงผล

บันทึกเหตุการณ์จากหน้าต่าง Mosquitto Broker Verbose Log:
```text
mosquitto version 2.0.18 starting
Config loaded from mosquitto.conf.
Opening ipv4 listen socket on port 1883.
mosquitto version 2.0.18 running

New connection from 127.0.0.1:52134 on port 1883.
New client connected from 127.0.0.1:52134 as sub_client_1 (p2, c1, k60).
Received SUBSCRIBE from sub_client_1
    home/+/light (QoS 0)
sub_client_1 0 home/+/light

New connection from 127.0.0.1:52135 on port 1883.
New client connected from 127.0.0.1:52135 as pub_client_1 (p2, c1, k60).
Received PUBLISH from pub_client_1 (d0, q0, r0, m0, 'home/livingroom/light', ... (2 bytes))
Sending PUBLISH to sub_client_1 (d0, q0, r0, m0, 'home/livingroom/light', ... (2 bytes))
```

ตารางเปรียบเทียบลำดับแพ็กเก็ตที่สังเกตได้จาก Broker Log ในแต่ละระดับ QoS:

| ระดับ QoS | นิยาม | ลำดับแพ็กเก็ตที่เกิดขึ้นระหว่าง Publisher, Broker และ Subscriber | ข้อสังเกต |
| :---: | :---: | :---: | :---: |
| **QoS 0** | At Most Once | `PUBLISH` -> Broker -> `PUBLISH` | ส่งครั้งเดียว ไม่มีการยืนยัน ไม่มีการส่งซ้ำ |
| **QoS 1** | At Least Once | `PUBLISH (q1, m1)` -> Broker -> `PUBACK (Mid: 1)` | มีการยืนยันด้วย PUBACK รับประกันว่าได้รับอย่างน้อย 1 ครั้ง |
| **QoS 2** | Exactly Once | 1. `PUBLISH (q2)` -> Broker<br>2. `PUBREC` -> Publisher<br>3. `PUBREL` -> Broker<br>4. `PUBCOMP` -> Publisher | จับมือ 4 ขั้นตอน รับประกันว่าได้รับถูกต้องเพียงครั้งเดียวแน่นอน |

ผลการทดสอบ Retained Message:
- เมื่อสั่ง Publish ด้วยพารามิเตอร์ `-r` ข้อความ `"CRITICAL_ALERT"` ไปยัง `sensor/state`
- เมื่อเปิด Subscriber ตัวใหม่ขึ้นมาภายหลัง Subscriber จะได้รับข้อความ `sensor/state CRITICAL_ALERT` ทันทีที่เชื่อมต่อสำเร็จ แม้ไม่มีการส่งข้อความใหม่

ผลการทดสอบ Last Will and Testament (LWT):
- เมื่อปิดเทอร์มินัลของ Client `Node_Client_01` กะทันหัน โบรกเกอร์ตรวจพบว่า TCP Connection ขาดหาย และส่งข้อความ `node/esp32/status LOST_CONNECTION_OFFLINE` ไปยัง Subscriber ทันที

#### 3. สรุปผลและการตอบคำถามท้ายใบงาน

- **สรุปผลการทดลอง:**
ติดตั้งและทดสอบการทำงานของ Mosquitto MQTT Broker ได้สมบูรณ์ โดยระบบรองรับสถาปัตยกรรม Publish/Subscribe, การกรอง Topic ด้วย Single-level Wildcard (+), กลไกการส่งข้อมูล 3 ระดับ (QoS 0, 1, 2) ซึ่งมีลำดับแพ็กเก็ตยืนยันต่างกันอย่างชัดเจน ตลอดจนฟีเจอร์ Retained Message ที่ส่งสถานะล่าสุดให้ Subscriber ใหม่ทันที และ Last Will and Testament (LWT) ที่แจ้งสถานะออฟไลน์อัตโนมัติเมื่ออุปกรณ์ขาดการเชื่อมต่ออย่างกะทันหัน

**คำถามท้ายการทดลอง (Post-Lab Questions):**
1. **เพราะเหตุใดสัญลักษณ์ `#` จึงต้องวางไว้ที่ตำแหน่งท้ายสุดของ Topic เสมอ? หากกำหนด Topic Filter เป็น `home/#/light` จะเกิดข้อผิดพลาดอย่างไร?**
     - **คำตอบ:** เพราะ # คือ Multi-level Wildcard ที่ใช้แทนลำดับชั้นของ Topic ทั้งหมดที่อยู่ถัดไปจนสุดสาย หากกำหนดเป็น home/#/light จะผิดกฎมาตรฐาน MQTT โบรกเกอร์จะมองเป็น Topic ที่ไม่ถูกต้อง (Invalid Topic) และปฏิเสธคำสั่ง Subscribe ทันที (หากต้องการดักจับเฉพาะชั้นตรงกลางต้องใช้ + เช่น home/+/light)

2. **ในระบบงานติดตามยานพาหนะ (GPS Tracking) ที่ส่งพิกัดรถยนต์ทุกๆ 1 วินาที ท่านคิดว่าควรเลือกระดับ QoS ใดระหว่าง QoS 0, 1 หรือ 2 เพราะเหตุใด?**
     - **คำตอบ:** ควรเลือก QoS 0 เพราะข้อมูลส่งถี่มาก (1 Hz) ระบบเน้นพิกัดล่าสุดเป็นหลัก หากมีข้อมูลหลุดหายไป 1 วินาที พิกัดใหม่ในวินาทีถัดไปจะเข้ามาแทนที่ทันที การใช้ QoS 1 หรือ 2 จะสร้างภาระ Handshake และเสี่ยงต่อการเกิดปัญหาข้อมูลค้างส่งสะสม (Queue Latency) บนเครือข่ายมือถือ

3. **หาก Client ส่งแพ็กเก็ต `DISCONNECT` ไปยัง Broker อย่างถูกต้องก่อนปิดโปรแกรม ข้อความ Last Will and Testament (LWT) จะถูกเผยแพร่ออกไปหรือไม่ เพราะเหตุใด?**
     - **คำตอบ:** ไม่ถูกเผยแพร่ เพราะการส่งแพ็กเก็ต DISCONNECT ถือเป็นการตัดการเชื่อมต่อแบบปกติที่วางแผนไว้ (Graceful Disconnect) โบรกเกอร์จะยกเลิกและทิ้งข้อความ LWT ทันที โดยข้อความ LWT จะทำงานก็ต่อเมื่อเกิดการหลุดแบบผิดปกติ (Ungraceful Disconnect) เช่น เน็ตตัด ไฟดับ หรือโปรแกรมแครชเท่านั้น

4. **Retained Message มีประโยชน์อย่างไรในการออกแบบระบบแดชบอร์ด Smart Home สำหรับผู้ใช้งานที่เพิ่งเปิดแอปพลิเคชันขึ้นมาเป็นครั้งแรก?**
     - **คำตอบ:** ช่วยให้แอปพลิเคชันได้รับค่าสถานะปัจจุบันของอุปกรณ์ (เช่น สถานะไฟ ON/OFF หรืออุณหภูมิล่าสุด) ขึ้นมาแสดงบนแดชบอร์ดได้ทันทีที่กดเชื่อมต่อสำเร็จ โดยไม่ต้องรอให้อุปกรณ์ฝั่งฮาร์ดแวร์ส่งข้อมูลรอบถัดไป ช่วยแก้ปัญหาหน้าแดชบอร์ดว่างเปล่าหรือไม่แสดงสถานะขณะเปิดแอป


---