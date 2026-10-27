# 2_start_ssl_broker.ps1 - สั่งรัน Mosquitto MQTTS Broker (Port 8883)
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Starting Mosquitto MQTTS Broker (Port 8883, mTLS) " -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan

$confPath = Join-Path $PSScriptRoot "mosquitto_ssl.conf"
& "C:\Program Files\mosquitto\mosquitto.exe" -c "$confPath" -v
