# 4. การบันทึกและวิเคราะห์ผลการทดลอง
ให้นักศึกษาบันทึกภาพหน้าจอ (Screenshots) และตอบคำถามต่อไปนี้ลงในรายงาน

# ภาพหน้าจอการทำงานของ Mosquitto Broker Verbose Log ขณะที่มีการเชื่อมต่อและ Subscribe ข้อความ
<img width="1049" height="272" alt="image" src="https://github.com/user-attachments/assets/ccb1aa3d-007a-45ad-a5cd-99c55480a20c" />
<img width="1186" height="266" alt="image" src="https://github.com/user-attachments/assets/7a5c19b2-eb2e-49f7-8d5f-80ad28432c0e" />
<img width="1187" height="206" alt="image" src="https://github.com/user-attachments/assets/032135c2-ed40-4af9-9ab5-69ad26c9cbd0" />

# ตารางเปรียบเทียบลำดับแพ็กเก็ตที่เกิดขึ้นระหว่าง QoS 0, QoS 1 และ QoS 2 ที่สังเกตได้จาก Broker Log
<img width="1196" height="27" alt="image" src="https://github.com/user-attachments/assets/82295544-15e0-4e76-ab86-d61dbf12cbae" />
<img width="1199" height="28" alt="image" src="https://github.com/user-attachments/assets/de805756-1e8e-47d6-bd5f-41d5a6e27d6d" />
<img width="1201" height="44" alt="image" src="https://github.com/user-attachments/assets/c2ad1fda-08c2-4d81-8559-f1f70132682d" />
<img width="1049" height="162" alt="image" src="https://github.com/user-attachments/assets/0ab36c92-3b44-45d9-b365-d19fe5a0e015" />
ตารางเปรียบเทียบลำดับแพ็กเก็ตที่เกิดขึ้นระหว่าง QoS 0, QoS 1 และ QoS 2 ที่สังเกตได้จาก Broker Log

| ระดับ QoS | ชื่อเรียก | ลำดับแพ็กเก็ตที่สังเกตได้จาก Broker Log | จำนวนแพ็กเก็ตที่ใช้ยืนยัน | ลักษณะการรับประกัน |
|:---:|:---:|---|:---:|---|
| **QoS 0** | At Most Once | `PUBLISH` | 0 (ไม่มีแพ็กเก็ตตอบรับ) | ส่งแบบ "ส่งแล้วจบ" ไม่มีการยืนยัน ข้อความอาจสูญหายได้ |
| **QoS 1** | At Least Once | `PUBLISH` → `PUBACK` | 1 | รับประกันว่าถึงอย่างน้อย 1 ครั้ง แต่อาจได้รับซ้ำ |
| **QoS 2** | Exactly Once | `PUBLISH` → `PUBREC` → `PUBREL` → `PUBCOMP` | 3 (4-way handshake) | รับประกันว่าถึงเพียง 1 ครั้งพอดี ไม่ซ้ำ ไม่สูญหาย |

**สรุป:** ยิ่งระดับ QoS สูง ความน่าเชื่อถือยิ่งมาก แต่ใช้แพ็กเก็ตและเวลามากขึ้นตามไปด้วย


# ภาพการทดสอบ Retained Message เมื่อเปิด Subscriber ขึ้นมาใหม่
<img width="1111" height="98" alt="image" src="https://github.com/user-attachments/assets/2840fd6e-8d95-4587-98ef-56f6c2ab7d39" />
<img width="1188" height="87" alt="image" src="https://github.com/user-attachments/assets/9d9fef7a-e7e8-4e00-b4f2-ec23c4d6e151" />
```
ผลที่เห็นขึ้น sensor/state CRITICAL_ALERT ทันที ทั้งที่ไม่มีส่งอะไรใหม่
```
