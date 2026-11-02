# Lab 11.4 — ESP RainMaker Smart Light

ESP-IDF project for the RainMaker lab. The firmware controls LED power and brightness using LEDC PWM, supports RainMaker BLE provisioning, and enables timezone and schedule services.

## Build and flash

Run these commands in an ESP-IDF PowerShell environment (the workspace machine has ESP-IDF 6.0.2 installed):

```powershell
cd Lab11-4_RainMaker_Light
idf.py set-target esp32c3 # or esp32
idf.py build
idf.py -p COMx flash monitor
```

Replace `COMx` with the board's actual serial port from Device Manager. On the first build, the IDF Component Manager downloads the RainMaker dependencies; network access is required.

Use the GPIO and board target matching the actual board. A bare LED with a series resistor is suitable for basic on/off; visible PWM dimming can vary with the LED, board circuit, and GPIO. Verify wiring and board pinout before connecting hardware.

Provision with the ESP RainMaker mobile app by scanning the QR code printed in the monitor, completing claiming if prompted, then entering a 2.4 GHz Wi-Fi network. Cloud, phone app, BLE claiming, and 4G/5G checks require actual hardware, account, and network access. The firmware starts the RainMaker agent after `app_network_start()` returns, following Espressif's sample flow.
