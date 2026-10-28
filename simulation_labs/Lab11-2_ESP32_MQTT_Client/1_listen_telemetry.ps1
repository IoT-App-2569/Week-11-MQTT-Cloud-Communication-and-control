# 1_listen_telemetry.ps1 - รับข้อมูล Telemetry จาก ESP32
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Listening to ESP32 Telemetry Data " -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan

& "C:\Program Files\mosquitto\mosquitto_sub.exe" -h 127.0.0.1 -p 1883 -t "iot/node01/telemetry" -v
