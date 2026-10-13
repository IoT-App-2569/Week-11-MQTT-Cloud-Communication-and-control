# Lab 11-1 – Mosquitto Broker and MQTT Fundamentals

## Files

| File | Purpose |
|---|---|
| `mosquitto.conf` | Broker config (port 1883, anonymous, verbose log) |
| `lab11-1.ps1` | Runs each lab step with one short command |

## 0. Prerequisites (once)

1. Install Mosquitto from <https://mosquitto.org/download/> (Windows x64 installer).
2. Open PowerShell **as Administrator** and stop the background Windows service, otherwise two brokers will run side by side:

```powershell
Stop-Service mosquitto
Set-Service mosquitto -StartupType Manual
```

3. In each terminal, allow the script to run for this session and `cd` to this folder:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
cd C:\IOT\Week-11-MQTT-Cloud-Communication-and-control\Code\Lab11-1-Mosquitto-Broker-and-MQTT-Fundamentals
```

4. Open **4 terminals**: Broker, Subscriber, Publisher, Publisher 2.

## 1. Start the broker (Terminal 1)

```powershell
.\lab11-1.ps1 broker
```

Expected: `mosquitto version 2.x.x running`. Leave it open (Ctrl+C to stop).

## 2. Activity 11-1.2 – Pub/Sub and wildcards

| Terminal 2 (Subscriber) | Terminal 3 (Publisher) | Expected on Terminal 2 |
|---|---|---|
| `.\lab11-1.ps1 sub-basic` | `.\lab11-1.ps1 pub-basic` | `home/livingroom/light ON` |
| `.\lab11-1.ps1 sub-plus`  | `.\lab11-1.ps1 pub-plus`  | livingroom + kitchen only (balcony **not** shown) |
| `.\lab11-1.ps1 sub-hash`  | `.\lab11-1.ps1 pub-hash`  | both messages |

Press **Ctrl+C** in Terminal 2 before starting the next subscriber.

## 3. Activity 11-1.3 – QoS 0 / 1 / 2

Run each pair, watching the **broker log** (Terminal 1):

```powershell
# Terminal 2                      # Terminal 3
.\lab11-1.ps1 sub-qos0            .\lab11-1.ps1 pub-qos0   # PUBLISH only
.\lab11-1.ps1 sub-qos1            .\lab11-1.ps1 pub-qos1   # PUBLISH -> PUBACK
.\lab11-1.ps1 sub-qos2            .\lab11-1.ps1 pub-qos2   # PUBLISH -> PUBREC -> PUBREL -> PUBCOMP
```

## 4. Activity 11-1.4 – Retained message

```powershell
# Terminal 3:  send a normal message while nobody is subscribed
.\lab11-1.ps1 pub-normal
# Terminal 2:  subscribe -> nothing appears
.\lab11-1.ps1 sub-retained
# Terminal 3:  send with retain flag -> Terminal 2 receives CRITICAL_ALERT
.\lab11-1.ps1 pub-retained
# Terminal 2:  Ctrl+C, run sub-retained again -> CRITICAL_ALERT arrives immediately
.\lab11-1.ps1 sub-retained
# Cleanup: delete the retained message
.\lab11-1.ps1 clear-retained
```

## 5. Activity 11-1.5 – Last Will and Testament

```powershell
# Terminal 2: watch the status topic
.\lab11-1.ps1 sub-status
# Terminal 3: simulated ESP32 with a will message
.\lab11-1.ps1 lwt-client
# Terminal 4: announce ONLINE  (Terminal 2 shows ONLINE)
.\lab11-1.ps1 pub-online
```

Now **close the Terminal 3 window with the X button** (do not press Ctrl+C, so no `DISCONNECT` is sent).
Terminal 2 then shows `node/esp32/status LOST_CONNECTION_OFFLINE`.

## Running without the helper script

All commands are plain Mosquitto CLI calls, e.g.:

```powershell
mosquitto.exe -c .\mosquitto.conf -v
mosquitto_sub.exe -h 127.0.0.1 -p 1883 -t "home/+/light" -v
mosquitto_pub.exe -h 127.0.0.1 -p 1883 -t "home/kitchen/light" -m "OFF"
```

## Troubleshooting

- **Messages don't arrive** – the Windows `mosquitto` service is still running (split-brain). Stop it as in step 0.
- **`mosquitto.exe` not found** – add `C:\Program Files\mosquitto` to `PATH` and reopen PowerShell.
- **Script blocked** – run `Set-ExecutionPolicy -Scope Process Bypass` in that terminal.
- **Port 1883 in use** – `Get-NetTCPConnection -LocalPort 1883` shows which process holds it.
