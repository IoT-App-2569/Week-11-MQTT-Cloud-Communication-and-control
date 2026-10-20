#!/usr/bin/env bash
# ใบงาน 11.1 — ทดสอบ Pub/Sub, Wildcards, QoS 0/1/2, Retained Message และ LWT ด้วย mosquitto_sub / mosquitto_pub
# ทำตามลำดับกิจกรรมในใบงาน แต่รันอัตโนมัติและเก็บผลของแต่ละ "หน้าต่าง" ไว้ใน logs/
#   logs/broker.log          = เทอร์มินัลที่ 1 (Mosquitto Broker, verbose)
#   logs/<กิจกรรม>_sub.txt   = เทอร์มินัลที่ 2 (Subscriber)
#   logs/<กิจกรรม>_pub.txt   = เทอร์มินัลที่ 3/4 (Publisher)
#   logs/<กิจกรรม>_broker.txt = Log ของ Broker เฉพาะช่วงกิจกรรมนั้น
# ใช้งาน: ./lab11_1_cli_tests.sh   (ต้องไม่มีโปรแกรมอื่นจองพอร์ต 1883 อยู่)
set -u
cd "$(dirname "$0")"

H=127.0.0.1
P=1883
LOG=logs
rm -rf "$LOG" && mkdir -p "$LOG"

# แสดงคำสั่งแบบที่พิมพ์ในเทอร์มินัล (ใส่ "..." ให้อาร์กิวเมนต์ที่มีอักขระพิเศษ)
show() {
    local out="" a
    for a in "$@"; do
        if [[ -z $a || $a =~ [^A-Za-z0-9_./:=-] ]]; then out+=" \"$a\""; else out+=" $a"; fi
    done
    echo "\$${out}"
}

# รัน Subscriber เบื้องหลัง เก็บ PID ไว้ใน SUB_PID
sub_bg() {
    local f=$1; shift
    show mosquitto_sub "$@" >> "$f"
    mosquitto_sub "$@" >> "$f" 2>&1 &
    SUB_PID=$!
    sleep 0.7
}

pub() {
    local f=$1; shift
    show mosquitto_pub "$@" >> "$f"
    mosquitto_pub "$@" >> "$f" 2>&1
    sleep 0.3
}

# ตัด Broker Log เฉพาะบรรทัดที่เกิดขึ้นระหว่างกิจกรรม
mark() { wc -l < "$LOG/broker.log" | tr -d ' '; }
slice() { sed -n "$(( $1 + 1 )),\$p" "$LOG/broker.log" > "$LOG/$2_broker.txt"; }

# ---------- กิจกรรม 11-1.1 เริ่ม Broker ----------
echo "\$ mosquitto -c mosquitto.conf -v" > "$LOG/broker.log"
mosquitto -c mosquitto.conf -v >> "$LOG/broker.log" 2>&1 &
BROKER_PID=$!
sleep 1
if ! kill -0 "$BROKER_PID" 2>/dev/null; then
    echo "Broker ไม่ทำงาน ดู $LOG/broker.log"; exit 1
fi

# ---------- กิจกรรม 11-1.2 Pub/Sub และ Wildcards ----------
m=$(mark); f=11-1.2.1
sub_bg "$LOG/${f}_sub.txt" -h $H -p $P -t "home/livingroom/light" -v -W 2
pub "$LOG/${f}_pub.txt" -h $H -p $P -t "home/livingroom/light" -m "ON"
wait $SUB_PID; slice "$m" $f

m=$(mark); f=11-1.2.2
sub_bg "$LOG/${f}_sub.txt" -h $H -p $P -t "home/+/light" -v -W 3
pub "$LOG/${f}_pub.txt" -h $H -p $P -t "home/livingroom/light" -m "ON"
pub "$LOG/${f}_pub.txt" -h $H -p $P -t "home/kitchen/light" -m "OFF"
pub "$LOG/${f}_pub.txt" -h $H -p $P -t "home/bedroom/balcony/light" -m "ON"
wait $SUB_PID; slice "$m" $f

m=$(mark); f=11-1.2.3
sub_bg "$LOG/${f}_sub.txt" -h $H -p $P -t "home/#" -v -W 2
pub "$LOG/${f}_pub.txt" -h $H -p $P -t "home/livingroom/temp" -m "26.4"
pub "$LOG/${f}_pub.txt" -h $H -p $P -t "home/bedroom/balcony/light" -m "ON"
wait $SUB_PID; slice "$m" $f

# คำถามข้อ 1: ทดลอง Topic Filter ที่วาง # ไว้กลาง Topic
f=11-1.2.4
show mosquitto_sub -h $H -p $P -t "home/#/light" -v > "$LOG/${f}_sub.txt"
mosquitto_sub -h $H -p $P -t "home/#/light" -v -W 2 >> "$LOG/${f}_sub.txt" 2>&1
echo "(exit code = $?)" >> "$LOG/${f}_sub.txt"

# ---------- กิจกรรม 11-1.3 QoS 0 / 1 / 2 ----------
for q in 0 1 2; do
    m=$(mark); f=11-1.3_qos$q
    sub_bg "$LOG/${f}_sub.txt" -h $H -p $P -i "sub-qos$q" -t "test/qos" -q $q -v -d -W 2
    pub "$LOG/${f}_pub.txt" -h $H -p $P -i "pub-qos$q" -t "test/qos" -m "QoS $q Message" -q $q -d
    wait $SUB_PID; slice "$m" $f
done

# ---------- กิจกรรม 11-1.4 Retained Message ----------
m=$(mark); f=11-1.4
pub "$LOG/${f}_pub.txt" -h $H -p $P -t "sensor/state" -m "Normal"
echo "# --- (1) เปิด Subscriber หลังส่งข้อความธรรมดา: ไม่ควรได้รับอะไร ---" >> "$LOG/${f}_sub.txt"
sub_bg "$LOG/${f}_sub.txt" -h $H -p $P -t "sensor/state" -v
echo "# --- (2) Subscriber ยังเปิดอยู่ แล้ว Publish พร้อม -r ---" >> "$LOG/${f}_sub.txt"
pub "$LOG/${f}_pub.txt" -h $H -p $P -t "sensor/state" -m "CRITICAL_ALERT" -r
kill -INT $SUB_PID; wait $SUB_PID 2>/dev/null
echo "^C" >> "$LOG/${f}_sub.txt"
echo "# --- (3) เปิด Subscriber ใหม่ (ไม่มีการ Publish ใหม่) ---" >> "$LOG/${f}_sub.txt"
sub_bg "$LOG/${f}_sub.txt" -h $H -p $P -t "sensor/state" -v -F "%t %p   [retain flag=%r]"
kill -INT $SUB_PID; wait $SUB_PID 2>/dev/null
echo "^C" >> "$LOG/${f}_sub.txt"
echo "# --- (4) ลบ Retained Message ด้วย Empty Payload + -r แล้วเปิด Subscriber อีกครั้ง ---" >> "$LOG/${f}_sub.txt"
pub "$LOG/${f}_pub.txt" -h $H -p $P -t "sensor/state" -n -r
sub_bg "$LOG/${f}_sub.txt" -h $H -p $P -t "sensor/state" -v
kill -INT $SUB_PID; wait $SUB_PID 2>/dev/null
echo "^C" >> "$LOG/${f}_sub.txt"
slice "$m" $f

# ---------- กิจกรรม 11-1.5 Last Will and Testament ----------
m=$(mark); f=11-1.5
sub_bg "$LOG/${f}_sub.txt" -h $H -p $P -t "node/esp32/status" -v
WATCH_PID=$SUB_PID
# เทอร์มินัลที่ 3: Client จำลอง ESP32 ที่ตั้ง LWT ไว้
show mosquitto_sub -h $H -p $P -i "Node_Client_01" -t "dummy/listen" --will-topic "node/esp32/status" --will-payload "LOST_CONNECTION_OFFLINE" --will-qos 1 > "$LOG/${f}_node.txt"
mosquitto_sub -h $H -p $P -i "Node_Client_01" -t "dummy/listen" --will-topic "node/esp32/status" --will-payload "LOST_CONNECTION_OFFLINE" --will-qos 1 >> "$LOG/${f}_node.txt" 2>&1 &
NODE_PID=$!
sleep 0.7
# เทอร์มินัลที่ 4: ส่งสถานะ ONLINE
pub "$LOG/${f}_pub.txt" -h $H -p $P -t "node/esp32/status" -m "ONLINE"
# จำลองไฟดับ: ฆ่าโปรเซสทันที (SIGKILL) Client ไม่มีโอกาสส่ง DISCONNECT
kill -KILL $NODE_PID; wait $NODE_PID 2>/dev/null
echo "[process killed with SIGKILL — ไม่มีการส่งแพ็กเก็ต DISCONNECT]" >> "$LOG/${f}_node.txt"
sleep 1
slice "$m" $f

# คำถามข้อ 3: Client ที่ตั้ง LWT เหมือนกัน แต่ปิดอย่างถูกต้อง (Ctrl+C -> ส่ง DISCONNECT)
m=$(mark); f=11-1.5_graceful
show mosquitto_sub -h $H -p $P -i "Node_Client_02" -t "dummy/listen" --will-topic "node/esp32/status" --will-payload "WILL_FROM_NODE_02" --will-qos 1 > "$LOG/${f}_node.txt"
mosquitto_sub -h $H -p $P -i "Node_Client_02" -t "dummy/listen" --will-topic "node/esp32/status" --will-payload "WILL_FROM_NODE_02" --will-qos 1 >> "$LOG/${f}_node.txt" 2>&1 &
NODE_PID=$!
sleep 0.7
kill -INT $NODE_PID; wait $NODE_PID 2>/dev/null
echo "^C   [SIGINT — mosquitto_sub ส่ง DISCONNECT ก่อนปิด]" >> "$LOG/${f}_node.txt"
sleep 1
slice "$m" $f
kill -INT $WATCH_PID; wait $WATCH_PID 2>/dev/null
echo "^C" >> "$LOG/11-1.5_sub.txt"

# ---------- ปิด Broker (Ctrl+C) ----------
kill -INT $BROKER_PID; wait $BROKER_PID 2>/dev/null
echo "^C" >> "$LOG/broker.log"
echo "เสร็จสิ้น ผลลัพธ์อยู่ใน $LOG/"
