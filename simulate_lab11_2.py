import time
import random
import sys

def plog(tag, level, msg):
    ms = int((time.time() - start_time) * 1000) + 1200
    print(f"{level} ({ms}) {tag}: {msg}")
    sys.stdout.flush()

start_time = time.time()

print("--- idf_monitor on COM3 115200 ---")
print("--- Quit: Ctrl+] | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H ---")
time.sleep(0.3)
print("ESP-ROM:esp32c3-api1-20210207")
print("Build:Feb  7 2021")
print("rst:0x1 (POWERON),boot:0xc (SPI_FAST_FLASH_BOOT)")
print("SPIWP:0xee")
print("mode:DIO, clock div:1")
print("load:0x3fcd5820,len:0x1128")
print("load:0x403cc710,len:0x934")
print("load:0x403ce710,len:0x2d18")
print("entry 0x403cc710")
time.sleep(0.4)

plog("cpu_start", "I", "App cpu up.")
plog("cpu_start", "I", "Pro cpu start user code")
plog("cpu_start", "I", "cpu freq: 160000000 Hz")
plog("cpu_start", "I", "Application information: project name: Lab11-2_MQTT_Client, version: 1")
plog("nvs", "I", "NVS partition 'nvs' is initialized")
plog("wifi", "I", "wifi driver task: 3fcdba40, prio:23, stack:6656, core=0")
plog("system_api", "I", "Base MAC Address: 84:f7:03:07:9b:18")
plog("wifi", "I", "wifi firmware version: 8a48ef6")
plog("wifi", "I", "wifi mode : STA")
plog("LAB11_MQTT", "I", "Wi-Fi Initialization finished.")
time.sleep(0.6)

plog("wifi", "I", 'Connected to "AIS 4G Hi-Speed Home WiFi_769475"')
plog("esp_netif_handlers", "I", "sta ip: 192.168.1.145, mask: 255.255.255.0, gw: 192.168.1.1")
plog("LAB11_MQTT", "I", "Got IP Address: 192.168.1.145")
time.sleep(0.4)

plog("LAB11_MQTT", "I", "MQTT_EVENT_CONNECTED: Connected to Broker!")
plog("LAB11_MQTT", "I", "Subscribed to: iot/node01/cmd/led")
time.sleep(0.4)

adc_val = 1750
led_state = False

for loop in range(1, 8):
    time.sleep(2.0)
    adc_val += random.randint(-35, 55)
    adc_val = max(500, min(3800, adc_val))
    volt = round((adc_val / 4095.0) * 3.3, 3)
    ms = int((time.time() - start_time) * 1000)
    
    if loop == 3:
        plog("LAB11_MQTT", "I", "MQTT_EVENT_DATA received:")
        plog("LAB11_MQTT", "I", "TOPIC=iot/node01/cmd/led")
        plog("LAB11_MQTT", "I", "DATA=ON")
        plog("LAB11_MQTT", "I", "LED State set to -> ON")
        led_state = True
        time.sleep(0.2)
    elif loop == 6:
        plog("LAB11_MQTT", "I", "MQTT_EVENT_DATA received:")
        plog("LAB11_MQTT", "I", "TOPIC=iot/node01/cmd/led")
        plog("LAB11_MQTT", "I", "DATA=OFF")
        plog("LAB11_MQTT", "I", "LED State set to -> OFF")
        led_state = False
        time.sleep(0.2)

    json_payload = f'{{"node_id":"esp32-node01","adc_raw":{adc_val},"voltage_v":{volt:.3f},"led_state":{"true" if led_state else "false"},"uptime_ms":{ms}}}'
    plog("LAB11_MQTT", "I", f"Telemetry Published: {json_payload}")
