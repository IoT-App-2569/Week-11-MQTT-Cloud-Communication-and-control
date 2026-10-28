# 2_send_led_on.ps1 - ส่งคำสั่งเปิดไฟ LED
Write-Host "Sending Command: ON to iot/node01/cmd/led..." -ForegroundColor Green
& "C:\Program Files\mosquitto\mosquitto_pub.exe" -h 127.0.0.1 -p 1883 -t "iot/node01/cmd/led" -m "ON" -q 1
Write-Host "Command Sent!" -ForegroundColor Cyan
