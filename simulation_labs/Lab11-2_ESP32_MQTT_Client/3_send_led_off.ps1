# 3_send_led_off.ps1 - ส่งคำสั่งปิดไฟ LED
Write-Host "Sending Command: OFF to iot/node01/cmd/led..." -ForegroundColor Red
& "C:\Program Files\mosquitto\mosquitto_pub.exe" -h 127.0.0.1 -p 1883 -t "iot/node01/cmd/led" -m "OFF" -q 1
Write-Host "Command Sent!" -ForegroundColor Cyan
