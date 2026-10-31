import time

print("--- idf_monitor on COM3 115200 ---")
print("--- Quit: Ctrl+] | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H ---")
time.sleep(0.4)

print("I (540) esp_rmaker_core: RainMaker Core Initialized")
print("I (550) esp_rmaker_core: Node ID: 84F703079B18")
print("I (560) esp_rmaker_node: Device 'Living Room Light' added to Node")
print("I (570) esp_rmaker_param: Parameter 'Power' added to Device")
print("I (580) esp_rmaker_param: Parameter 'Brightness' added to Device")
print("I (610) app_wifi: Starting Wi-Fi Provisioning via BLE")
print("I (620) app_wifi: Provisioning Service name : PROV_9B18")
print("I (630) app_wifi: Scan this QR code from the ESP RainMaker phone app:\n")

qr_ascii = """
+-------------------------------------------------+
|                                                 |
|   ##### ####### ##   # #  ##   ### ##### #####  |
|   #   # #     # #   #   #  #     # #   # #   #  |
|   # # # # ### # # ### ###  ##### # # # # # # #  |
|   #   # #     # #   #   #  #     # #   # #   #  |
|   ##### ####### # # # # # ## # # # ##### #####  |
|                                                 |
|   #####  ###  #  #  ###  ##   #### ####### ###  |
|   # ### #   # # ## #  # #   # #  #   #   #   #  |
|   #  #  ##### #  # ##### #### #  #   #   #####  |
|                                                 |
|   ##### ####### ##   # #  ##   ### #   # #####  |
|   #   # #     # #   #   #  #     # ## ## #   #  |
|   # # # # ### # # ### ###  ##### # # # # # # #  |
|   #   # #     # #   #   #  #     # #   # #   #  |
|   ##### ####### # # # # # ## # # # #   # #####  |
|                                                 |
+-------------------------------------------------+
"""
print(qr_ascii)
print("If QR code is not visible, copy paste this URL in browser:")
print("https://rainmaker.espressif.com/qrcode.html?data=%7B%22name%22%3A%22PROV_9B18%22%2C%22pop%22%3A%229475%22%2C%22transport%22%3A%22ble%22%7D\n")
time.sleep(0.8)

print("I (3240) app_wifi: Provisioning started with BLE")
print("I (8450) app_wifi: BLE connection established with smartphone")
print("I (12500) esp_claim: Assisted claiming started...")
print("I (15200) esp_claim: Claim certificate received from cloud")
print("I (17100) app_wifi: Wi-Fi credentials received. Connecting to SSID: AIS 4G Hi-Speed Home WiFi_769475...")
print("I (19400) esp_netif_handlers: sta ip: 192.168.1.145, mask: 255.255.255.0, gw: 192.168.1.1")
print("I (21200) esp_rmaker_mqtt: Connecting to AWS IoT endpoint...")
print("I (22500) esp_rmaker_mqtt: Connected to MQTT broker (MQTTS Port 8883)")
print("I (23100) esp_rmaker_core: Node is now online and registered to user account!")
