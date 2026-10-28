"""
simulate_rainmaker_node.py
Interactive ESP RainMaker Lightbulb Device Simulator for Lab 11-4
Simulates BLE Provisioning, QR Code generation, Write Callback,
and Cloud Parameter Synchronization (Power & Brightness).
"""

import time
import sys

def print_qr_code():
    qr = """
███████████████████████████████████████
███████████████████████████████████████
████ ▄▄▄▄▄ █▀▄█ █▄ █  █▀▀█ ▄▄▄▄▄ ████
████ █   █ █ █▀▀▀█ █ ▀█ ▄█ █   █ ████
████ █▄▄▄█ █ █▄█ ▀██ █▀▄██ █▄▄▄█ ████
████▄▄▄▄▄▄▄█▄█ █ █ █▄█▄█▄█▄▄▄▄▄▄▄████
████ ▄▄ ▄▄▄▀▄ ▀█ ▄ █ ▄▀▄▀▄▄▄▄  ▄ ████
████ █ ▄▄▄▀▀ ▀▄█ █▄ █▀▄▀█▄ ▄█▀█▀████
████▄█▄█▄▄▄█▀▄ ▀ ▀ █ █▀▀▄█▄▄▄█ █▄████
████ ▄▄▄▄▄ █  ▀█ ▄ █  █▄█▄ ▄ █ ▄ ████
████ █   █ █ █ ▀█ █▄█  █ █▀█ █ ▀████
████ █▄▄▄█ █ ▀ █▄█▀█▄█▀▀█▄█▄█ ▄████
████▄▄▄▄▄▄▄█▄▄█▄▄██▄▄████▄▄███▄▄▄████
███████████████████████████████████████
███████████████████████████████████████
"""
    print(qr)

def main():
    print("=" * 68)
    print("      ESP RainMaker Smart Light Device Simulator (Lab 11-4)")
    print("      Platform: ESP-IDF v5.x / v6.x (ESP32 / ESP32-C3)")
    print("=" * 68)

    # 1. Hardware & System Init
    print("\n[SYS_INIT] Initializing NVS Flash...")
    time.sleep(0.5)
    print("[SYS_INIT] Configuring GPIO 2 / 8 as OUTPUT for LED Actuator...")
    print("[RMAKER_CORE] Initializing ESP RainMaker Node: 'Smart Light Node' [Type: Lightbulb]")
    time.sleep(0.5)
    print("[RMAKER_DEV] Created Device: 'Living Room Light'")
    print("[RMAKER_PARAM] Added Standard Param: 'Power' (Default: OFF / false)")
    print("[RMAKER_PARAM] Added Standard Param: 'Brightness' (Default: 100%)")
    print("[RMAKER_SERVICE] System Service Enabled: Timezone SNTP Sync")
    print("[RMAKER_SERVICE] System Service Enabled: Schedule Automation Service")

    # 2. BLE Wi-Fi Provisioning
    print("\n" + "=" * 68)
    print("  [app_wifi] Starting BLE Wi-Fi Provisioning...")
    print("  Device Name (BLE): PROV_Light_LivingRoom_8A12")
    print("  Proof of Possession (PoP): 7x9aB3")
    print("=" * 68)

    print("\nScan the QR Code below using the ESP RainMaker Smartphone App:")
    print_qr_code()
    print("Or open this direct cloud registration URL in your browser:")
    print("https://rainmaker.espressif.com/qrcode.html?data=%7B%22name%22%3A%22PROV_Light_LivingRoom_8A12%22%2C%22pop%22%3A%227x9aB3%22%2C%22transport%22%3A%22ble%22%7D\n")

    print("[BLE] Waiting for smartphone pairing...")
    time.sleep(2)
    print(">>> [BLE] Phone Connected! Received Wi-Fi credentials via Bluetooth.")
    print(">>> [WIFI] Connecting to Home Wi-Fi (SSID: Lab_WiFi_2.4G)...")
    time.sleep(1)
    print(">>> [WIFI] Connected! Assigned IP: 192.168.1.188")
    print(">>> [CLAIMING] Assisted Claiming: Requesting X.509 Device Certificate from AWS IoT...")
    time.sleep(1)
    print(">>> [CLAIMING] Device Claimed Successfully! Node ID: 8A12B7C9D0EF")
    print(">>> [MQTT_CLOUD] Connected to AWS IoT Core (a2xxxxxx-ats.iot.us-east-1.amazonaws.com:8883)")
    print(">>> [STATE] Synced Device Shadow with Cloud.\n")

    # 3. Interactive simulation loop
    power = False
    brightness = 100

    print("=" * 68)
    print("  DEVICE IS ONLINE AND CONNECTED TO CLOUD!")
    print("  Enter simulated commands to test (or press Ctrl+C to exit):")
    print("    1: Mobile App sends 'Power = ON'")
    print("    2: Mobile App sends 'Power = OFF'")
    print("    3: Mobile App changes 'Brightness = 50%'")
    print("    4: Mobile App changes 'Brightness = 100%'")
    print("    5: Remote 4G/5G Cellular Control Simulation")
    print("=" * 68)

    while True:
        try:
            choice = input("\nEnter choice (1-5, or q to quit): ").strip()
            if choice == "1":
                power = True
                print(f"\n[CLOUD_MSG] Received from Cloud (App via AWS IoT):")
                print(f"    write_cb: Device 'Living Room Light' | Param 'Power' -> TRUE")
                print(f"    [GPIO] Set LED Pin = HIGH (💡 Light is ON)")
                print(f"    [REPORT] esp_rmaker_param_update_and_report: Reported Power=true to Cloud.")
            elif choice == "2":
                power = False
                print(f"\n[CLOUD_MSG] Received from Cloud (App via AWS IoT):")
                print(f"    write_cb: Device 'Living Room Light' | Param 'Power' -> FALSE")
                print(f"    [GPIO] Set LED Pin = LOW (🌑 Light is OFF)")
                print(f"    [REPORT] esp_rmaker_param_update_and_report: Reported Power=false to Cloud.")
            elif choice == "3":
                brightness = 50
                print(f"\n[CLOUD_MSG] Received from Cloud (App via AWS IoT):")
                print(f"    write_cb: Device 'Living Room Light' | Param 'Brightness' -> 50%")
                print(f"    [LEDC PWM] Adjusting PWM Duty Cycle to 50%...")
                print(f"    [REPORT] esp_rmaker_param_update_and_report: Reported Brightness=50 to Cloud.")
            elif choice == "4":
                brightness = 100
                print(f"\n[CLOUD_MSG] Received from Cloud (App via AWS IoT):")
                print(f"    write_cb: Device 'Living Room Light' | Param 'Brightness' -> 100%")
                print(f"    [LEDC PWM] Adjusting PWM Duty Cycle to 100%...")
                print(f"    [REPORT] esp_rmaker_param_update_and_report: Reported Brightness=100 to Cloud.")
            elif choice == "5":
                print(f"\n[REMOTE 4G/5G TEST] Smartphone switched Wi-Fi OFF and Cellular 5G ON:")
                print(f"    Mobile App sends 'Power Toggle' over WAN...")
                power = not power
                state_str = "ON (💡)" if power else "OFF (🌑)"
                print(f"    write_cb: Handled in 140ms via AWS IoT Cloud! LED is now {state_str}")
                print(f"    [REPORT] State synced globally.")
            elif choice.lower() == "q":
                print("\nExiting simulator. Goodbye!")
                break
        except KeyboardInterrupt:
            print("\nExiting simulator.")
            break

if __name__ == "__main__":
    main()
