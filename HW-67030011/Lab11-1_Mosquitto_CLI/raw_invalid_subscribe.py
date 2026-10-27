"""
คำถามท้ายการทดลอง 11.1 ข้อ 1 — ส่งแพ็กเก็ต SUBSCRIBE ที่มี Topic Filter "home/#/light" ตรงไปยัง Broker
(mosquitto_sub ตรวจและปฏิเสธเองตั้งแต่ฝั่ง Client จึงต้องประกอบแพ็กเก็ต MQTT 3.1.1 เองด้วย socket)
ใช้งาน: python3 raw_invalid_subscribe.py [host] [port] [topic_filter]
"""
import socket
import sys

HOST = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1"
PORT = int(sys.argv[2]) if len(sys.argv) > 2 else 1883
TOPIC = sys.argv[3] if len(sys.argv) > 3 else "home/#/light"
CLIENT_ID = "raw-wildcard-test"


def mqtt_str(s: str) -> bytes:
    b = s.encode()
    return len(b).to_bytes(2, "big") + b


def packet(first_byte: int, body: bytes) -> bytes:
    # Remaining Length (variable byte integer) — body สั้นกว่า 128 ไบต์จึงใช้ 1 ไบต์
    assert len(body) < 128
    return bytes([first_byte, len(body)]) + body


def hexdump(b: bytes) -> str:
    return " ".join(f"{x:02X}" for x in b)


connect = packet(0x10, mqtt_str("MQTT") + bytes([0x04, 0x02, 0x00, 0x3C]) + mqtt_str(CLIENT_ID))
subscribe = packet(0x82, (1).to_bytes(2, "big") + mqtt_str(TOPIC) + bytes([0x00]))

with socket.create_connection((HOST, PORT), timeout=3) as s:
    print(f"-> CONNECT   {hexdump(connect)}")
    s.sendall(connect)
    connack = s.recv(4)
    print(f"<- CONNACK   {hexdump(connack)}  (return code {connack[3]})")

    print(f"-> SUBSCRIBE {hexdump(subscribe)}  (filter '{TOPIC}', QoS 0)")
    s.sendall(subscribe)
    try:
        reply = s.recv(16)
    except (ConnectionResetError, socket.timeout) as e:
        reply = None
        print(f"<- {type(e).__name__}")
    if reply:
        print(f"<- {hexdump(reply)}  (packet type 0x{reply[0] >> 4:X})")
    elif reply == b"":
        print("<- connection closed by broker (no SUBACK)")
