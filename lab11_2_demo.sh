#!/bin/bash
# Script สาธิตการทดลอง Lab 11.2 สำหรับอัดคลิปวิดีโอ
BROKER_IP="127.0.0.1"
BROKER_PORT="1883"

echo "=========================================================="
echo "   LAB 11.2: ESP32 MQTT Telemetry & LED Control Demo"
echo "=========================================================="
echo "กำลังเชื่อมต่อกับ Broker และดักรับ Telemetry จาก ESP32..."
echo "----------------------------------------------------------"

# รัน mosquitto_sub ใน background เพื่อแสดงผลข้อมูล JSON สดๆ
mosquitto_sub -h $BROKER_IP -p $BROKER_PORT -t "iot/node01/telemetry" -v &
SUB_PID=$!

# เมื่อกด Ctrl+C ให้ปิด mosquitto_sub อัตโนมัติ
trap "kill $SUB_PID 2>/dev/null; exit 0" INT TERM EXIT

echo ""
echo "คำแนะนำในการอัดคลิป:"
echo "1. หมุน Potentiometer เพื่อให้ค่า adc_raw และ voltage_v เปลี่ยน"
echo "2. พิมพ์ '1' แล้วกด Enter -> เพื่อเปิดไฟ LED (ON)"
echo "3. พิมพ์ '2' แล้วกด Enter -> เพื่อปิดไฟ LED (OFF)"
echo "4. พิมพ์ 'q' แล้วกด Enter -> เพื่อจบการทำงาน"
echo "----------------------------------------------------------"

while true; do
    read -p ">> คำสั่ง (1=เปิดไฟ, 2=ปิดไฟ, q=ออก): " CMD
    case "$CMD" in
        1)
            mosquitto_pub -h $BROKER_IP -p $BROKER_PORT -t "iot/node01/cmd/led" -m "ON" -q 1
            echo "==> [สั่งเปิดไฟ LED: ON]"
            ;;
        2)
            mosquitto_pub -h $BROKER_IP -p $BROKER_PORT -t "iot/node01/cmd/led" -m "OFF" -q 1
            echo "==> [สั่งปิดไฟ LED: OFF]"
            ;;
        q|Q)
            echo "จบการสาธิต..."
            break
            ;;
        *)
            echo "กด 1 เพื่อเปิดไฟ, 2 เพื่อปิดไฟ, หรือ q เพื่อออก"
            ;;
    esac
done
