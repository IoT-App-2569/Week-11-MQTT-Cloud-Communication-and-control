@echo off
REM ==============================================================================
REM Batch Script to Launch Mosquitto Broker for Lab 11.1
REM Student ID: 67030281
REM ==============================================================================

echo [INFO] Checking and stopping background Mosquitto Windows Service (if active)...
powershell -Command "if (Get-Service mosquitto -ErrorAction SilentlyContinue) { Stop-Service mosquitto; Set-Service mosquitto -StartupType Manual; Write-Host '[OK] Mosquitto Windows Service stopped.' }"

echo [INFO] Starting Mosquitto Broker in Verbose Mode...
if exist "C:\Program Files\mosquitto\mosquitto.exe" (
    "C:\Program Files\mosquitto\mosquitto.exe" -c "%~dp0mosquitto.conf" -v
) else (
    mosquitto.exe -c "%~dp0mosquitto.conf" -v
)
pause

