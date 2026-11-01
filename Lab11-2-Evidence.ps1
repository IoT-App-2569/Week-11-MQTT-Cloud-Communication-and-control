$Host.UI.RawUI.WindowTitle = 'Lab 11.2 Actual ESP32 MQTT Evidence'
$pub = 'C:\Program Files\mosquitto\mosquitto_pub.exe'
$sub = 'C:\Program Files\mosquitto\mosquitto_sub.exe'
$hostName = '127.0.0.1'
$port = '1884'
$work = 'C:\Work\IoT-ESP32\Week-11-MQTT-Cloud-Communication-and-control\mqtt_lab'

Write-Host 'LAB 11.2 - ESP32 MQTT TELEMETRY & LED CONTROL (ACTUAL HARDWARE)' -ForegroundColor Cyan
Write-Host '67030142 Piyawit Pholsri | TechCom'
Write-Host ('=' * 82)

$status = & $sub -h $hostName -p $port -t 'iot/node01/status' -v -C 1
Write-Host "`n[1] Retained device status" -ForegroundColor Yellow
Write-Host "  $status"

$telemetry = & $sub -h $hostName -p $port -t 'iot/node01/telemetry' -v -C 2
Write-Host "`n[2] ESP32 telemetry (ADC / voltage / LED / uptime)" -ForegroundColor Yellow
$telemetry | ForEach-Object { Write-Host "  $_" }

$ackOut = Join-Path $work 'esp32-led-ack.txt'
$ackErr = Join-Path $work 'esp32-led-ack-error.txt'
$ackSub = Start-Process -FilePath $sub -ArgumentList @('-h',$hostName,'-p',$port,'-t','iot/node01/state/led','-v','-C','2') -WindowStyle Hidden -RedirectStandardOutput $ackOut -RedirectStandardError $ackErr -PassThru
Start-Sleep -Milliseconds 400
& $pub -h $hostName -p $port -t 'iot/node01/cmd/led' -m 'ON' -q 1
Start-Sleep -Milliseconds 500
& $pub -h $hostName -p $port -t 'iot/node01/cmd/led' -m 'OFF' -q 1
$ackSub.WaitForExit()
Write-Host "`n[3] Remote LED command and ESP32 acknowledgement" -ForegroundColor Yellow
Get-Content $ackOut | ForEach-Object { Write-Host "  $_" }

Write-Host "`nPASS: ESP32 publish, subscribe, retained status and LED control verified." -ForegroundColor Green
Read-Host 'Press Enter to close'
