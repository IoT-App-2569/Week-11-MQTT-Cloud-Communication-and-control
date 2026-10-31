import time
import json
import random
import paho.mqtt.client as mqtt

# 1. Callback เมื่อเชื่อมต่อ Mosquitto Broker สำเร็จ
def on_connect(client, userdata, flags, rc):
    print("==========================================")
    print("[ESP32 SIMULATOR] Connected to Broker!")
    client.subscribe("iot/node01/cmd/led")
    print("[ESP32 SIMULATOR] Subscribed to: iot/node01/cmd/led")
    print("==========================================")

# 2. Callback เมื่อได้รับคำสั่งเปิด-ปิดไฟ LED
def on_message(client, userdata, msg):
    payload = msg.payload.decode()
    print(f"\n[RECEIVED DATA] Topic: {msg.topic} | Payload: {payload}")
    
    if payload.upper() == "ON" or '"state":1' in payload or '"state": 1' in payload:
        print(">>> [VIRTUAL LED] STATUS: ON (ไฟสว่าง) <<<")
        client.publish("iot/node01/state/led", '{"led_state": true}')
    elif payload.upper() == "OFF" or '"state":0' in payload or '"state": 0' in payload:
        print(">>> [VIRTUAL LED] STATUS: OFF (ไฟดับ) <<<")
        client.publish("iot/node01/state/led", '{"led_state": false}')

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# เชื่อมต่อไปยัง Local Mosquitto Broker
client.connect("127.0.0.1", 1883, 60)
client.loop_start()

# 3. วนลูปส่ง Telemetry ทุกๆ 4 วินาที
try:
    while True:
        adc_raw = random.randint(0, 4095)
        voltage = round((adc_raw / 4095.0) * 3.3, 2)
        telemetry = {
            "node_id": "esp32-simulated-node",
            "adc_raw": adc_raw,
            "voltage_v": voltage,
            "uptime_ms": int(time.time() * 1000)
        }
        client.publish("iot/node01/telemetry", json.dumps(telemetry))
        print(f"[Telemetry Published]: {json.dumps(telemetry)}")
        time.sleep(4)
except KeyboardInterrupt:
    print("\nStopping simulation...")
    client.loop_stop()
    client.disconnect()