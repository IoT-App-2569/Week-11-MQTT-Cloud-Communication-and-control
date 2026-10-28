"""
simulate_esp32_device.py
ESP32 Bi-directional MQTT Client Simulator for Lab 11-2
Simulates FreeRTOS Telemetry Task (ADC / Potentiometer reading)
and MQTT Event Handler (Actuator LED Command).
Uses only Python Standard Library + Mosquitto CLI binaries.
"""

import subprocess
import time
import json
import random
import threading
import sys
import os

BROKER_IP = "127.0.0.1"
BROKER_PORT = "1883"
MOSQUITTO_PUB = r"C:\Program Files\mosquitto\mosquitto_pub.exe"
MOSQUITTO_SUB = r"C:\Program Files\mosquitto\mosquitto_sub.exe"

TELEMETRY_TOPIC = "iot/node01/telemetry"
CMD_TOPIC = "iot/node01/cmd/led"
STATUS_TOPIC = "iot/node01/status"

led_state = False
running = True
pot_angle_raw = 2048  # Default middle point (approx 1.65V)

def send_mqtt_publish(topic, payload, qos=1, retain=False):
    """Helper to send MQTT message using mosquitto_pub"""
    cmd = [MOSQUITTO_PUB, "-h", BROKER_IP, "-p", BROKER_PORT, "-t", topic, "-m", payload, "-q", str(qos)]
    if retain:
        cmd.append("-r")
    try:
        subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
    except Exception as e:
        print(f"[ERROR] Failed to publish: {e}")

def command_listener_thread():
    """Background thread simulating MQTT_EVENT_DATA on iot/node01/cmd/led"""
    global led_state, running
    cmd = [MOSQUITTO_SUB, "-h", BROKER_IP, "-p", BROKER_PORT, "-t", CMD_TOPIC, "-v"]
    try:
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
        while running:
            line = proc.stdout.readline()
            if not line:
                break
            line = line.strip()
            if line:
                parts = line.split(" ", 1)
                payload = parts[1] if len(parts) > 1 else ""
                print("\n=======================================================")
                print(f"📥 [MQTT_EVENT_DATA] Topic: {CMD_TOPIC}")
                print(f"    Raw Payload Received: {payload}")
                
                # Parse command
                if payload.upper() == "ON" or '"state": 1' in payload or '"state":1' in payload:
                    led_state = True
                    print("    💡 [ACTUATOR LOGIC] Command matched: LED Turned ON (GPIO 2 / 8 = HIGH)")
                elif payload.upper() == "OFF" or '"state": 0' in payload or '"state":0' in payload:
                    led_state = False
                    print("    💡 [ACTUATOR LOGIC] Command matched: LED Turned OFF (GPIO 2 / 8 = LOW)")
                else:
                    print(f"    ⚠️ [ACTUATOR LOGIC] Unknown command format: {payload}")
                print("=======================================================\n")
    except Exception as e:
        if running:
            print(f"[ERROR] Listener thread error: {e}")

def main():
    global led_state, running, pot_angle_raw
    print("=" * 65)
    print("  ESP32-C3 / ESP32 MQTT Client Simulation (Lab 11-2)")
    print("  Connecting to Broker at mqtt://" + BROKER_IP + ":" + BROKER_PORT)
    print("=" * 65)

    # 1. Simulate Wi-Fi connect
    print("[ESP-IDF] wifi_init_sta: Connecting to Wi-Fi...")
    time.sleep(1)
    print("[ESP-IDF] Got IP Address: 192.168.1.150")
    print("[ESP-IDF] Starting esp-mqtt client...")
    time.sleep(0.5)

    # 2. Simulate MQTT Connect & LWT/Status
    print("[MQTT_EVENT] MQTT_EVENT_CONNECTED: Connected to Broker!")
    print(f"[MQTT] Publishing initial status: {STATUS_TOPIC} = 'online' (retain=1)")
    send_mqtt_publish(STATUS_TOPIC, "online", qos=1, retain=True)

    # 3. Start Command Listener
    print(f"[MQTT] Subscribing to command topic: {CMD_TOPIC}")
    sub_thread = threading.Thread(target=command_listener_thread, daemon=True)
    sub_thread.start()

    # 4. Telemetry Loop
    start_time = time.time()
    print("\n[FreeRTOS Task] telemetry_task started! Publishing telemetry every 4s...\n")
    print("(Press Ctrl+C to terminate simulation)\n")

    try:
        while running:
            # Simulate slight drift / turn of potentiometer
            drift = random.randint(-40, 40)
            pot_angle_raw = max(0, min(4095, pot_angle_raw + drift))
            voltage_v = round((pot_angle_raw / 4095.0) * 3.3, 3)
            uptime_ms = int((time.time() - start_time) * 1000)

            telemetry_data = {
                "node_id": "esp32-node01",
                "adc_raw": pot_angle_raw,
                "voltage_v": voltage_v,
                "led_state": led_state,
                "uptime_ms": uptime_ms
            }
            json_payload = json.dumps(telemetry_data)

            # Publish
            send_mqtt_publish(TELEMETRY_TOPIC, json_payload, qos=1)
            print(f"📤 [TELEMETRY SENT] {json_payload}")

            time.sleep(4)
    except KeyboardInterrupt:
        print("\n\n[ESP-IDF] Disconnecting client...")
        running = False
        send_mqtt_publish(STATUS_TOPIC, "offline", qos=1, retain=True)
        print("[ESP-IDF] Sent status = 'offline'. Simulation finished gracefully.")

if __name__ == "__main__":
    main()
