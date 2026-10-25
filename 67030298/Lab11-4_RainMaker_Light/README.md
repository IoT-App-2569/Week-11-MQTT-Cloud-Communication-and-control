# Lab 11.4: ESP RainMaker Smart Light & Cloud IoT

โปรเจกต์อุปกรณ์ Smart Light สู่ระบบคลาวด์ด้วย ESP RainMaker SDK รองรับ BLE Wi-Fi Provisioning, Assisted Claiming และการควบคุมผ่านสมาร์ตโฟนข้ามอินเทอร์เน็ต

## โครงสร้างโปรเจกต์
```
Lab11-4_RainMaker_Light/
├── CMakeLists.txt
├── README.md
└── main/
    ├── CMakeLists.txt
    ├── idf_component.yml
    └── main.c
```

## คอมไพล์และแฟลชโปรแกรม
```powershell
idf.py set-target esp32c3   # หรือ esp32
idf.py build
idf.py -p COMx flash monitor
```

## ขั้นตอนการเชื่อมต่อและทดสอบ
1. เปิดแอปพลิเคชัน **ESP RainMaker** บนมือถือ
2. สแกน QR Code ที่แสดงผลใน Serial Monitor
3. กดยืนยันการขอใบรับรองผ่าน **Assisted Claiming**
4. เลือก Wi-Fi SSID 2.4 GHz และใส่รหัสผ่านผ่าน Bluetooth BLE
5. ทดสอบเปิด-ปิดสวิตช์ Power และปรับความสว่าง Brightness บนสมาร์ตโฟน
6. ทดสอบควบคุมข้ามเครือข่ายอินเทอร์เน็ตด้วย 4G/5G
