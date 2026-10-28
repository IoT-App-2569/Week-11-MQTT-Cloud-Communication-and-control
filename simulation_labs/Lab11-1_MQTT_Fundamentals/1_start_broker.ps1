# 1_start_broker.ps1 - สั่งเริ่มการทำงานของ Broker แบบ Verbose
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Starting Mosquitto MQTT Broker (Verbose) " -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan
$configPath = Join-Path $PSScriptRoot "mosquitto.conf"
& "C:\Program Files\mosquitto\mosquitto.exe" -c "$configPath" -v
