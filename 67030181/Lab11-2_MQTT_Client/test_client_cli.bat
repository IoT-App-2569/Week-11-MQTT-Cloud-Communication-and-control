@echo off
REM ==============================================================================
REM Test Commands for ESP32 MQTT Client (Lab 11.2)
REM Student ID: 67030281
REM ==============================================================================

set BROKER_IP=192.168.1.111
set BROKER_PORT=1883

echo ================================================================
echo  Lab 11.2 ESP32 MQTT Client Verification Menu
echo  Broker Target: %BROKER_IP%:%BROKER_PORT%
echo ================================================================
echo [1] Subscribe Telemetry Data (iot/node01/telemetry)
echo [2] Subscribe Node Status (iot/node01/status)
echo [3] Send LED Command: ON
echo [4] Send LED Command: OFF
echo [5] Send LED Command: JSON {"state": 1}
echo [6] Exit
echo ================================================================
choice /c 123456 /m "Select option:"

if errorlevel 6 goto end
if errorlevel 5 goto led_json
if errorlevel 4 goto led_off
if errorlevel 3 goto led_on
if errorlevel 2 goto sub_status
if errorlevel 1 goto sub_telemetry

:sub_telemetry
echo Subscribing to telemetry (Press Ctrl+C to stop)...
mosquitto_sub -h %BROKER_IP% -p %BROKER_PORT% -t "iot/node01/telemetry" -v
pause
goto end

:sub_status
echo Subscribing to node status...
mosquitto_sub -h %BROKER_IP% -p %BROKER_PORT% -t "iot/node01/status" -v
pause
goto end

:led_on
echo Publishing LED ON command...
mosquitto_pub -h %BROKER_IP% -p %BROKER_PORT% -t "iot/node01/cmd/led" -m "ON" -q 1
goto end

:led_off
echo Publishing LED OFF command...
mosquitto_pub -h %BROKER_IP% -p %BROKER_PORT% -t "iot/node01/cmd/led" -m "OFF" -q 1
goto end

:led_json
echo Publishing LED JSON command {"state": 1}...
mosquitto_pub -h %BROKER_IP% -p %BROKER_PORT% -t "iot/node01/cmd/led" -m "{\"state\": 1}" -q 1
goto end

:end