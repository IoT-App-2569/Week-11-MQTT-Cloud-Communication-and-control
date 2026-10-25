# Certificates Folder for ESP32 mTLS

โฟลเดอร์นี้ใช้เก็บไฟล์ใบรับรองสำหรับคอมไพล์ฝังลงในเฟิร์มแวร์ ESP32 ด้วยคำสั่ง `EMBED_TXTFILES` ใน `CMakeLists.txt`:
1. `ca.crt` - ใบรับรอง Root CA (สำหรับตรวจความถูกต้องของ Broker)
2. `client.crt` - ใบรับรองของ ESP32 Client (ลงนามโดย Root CA)
3. `client.key` - กุญแจส่วนตัว RSA ของ ESP32 Client

> **คำแนะนำ:** 
> รันสคริปต์ `..\scripts\generate_certs.ps1 -BrokerIP <YOUR_PC_IP>` เพื่อสร้างไฟล์ใบรับรองทั้งหมดด้วย OpenSSL โดยสคริปต์จะคัดลอกไฟล์มาไว้ในโฟลเดอร์นี้ให้อัตโนมัติ
