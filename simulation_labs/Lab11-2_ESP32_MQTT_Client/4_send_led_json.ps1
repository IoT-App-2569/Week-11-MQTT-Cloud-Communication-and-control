# 4_send_led_json.ps1 - ส่งคำสั่งเปิดไฟ LED ในรูปแบบ JSON
Write-Host "Sending Command JSON: {`"state`": 1} to iot/node01/cmd/led..." -ForegroundColor Yellow
& "C:\Program Files\mosquitto\mosquitto_pub.exe" -h 127.0.0.1 -p 1883 -t "iot/node01/cmd/led" -m "{\`"state\`": 1}" -q 1
Write-Host "Command Sent!" -ForegroundColor Cyan
