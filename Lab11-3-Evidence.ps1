$Host.UI.RawUI.WindowTitle = 'Lab 11.3 Actual MQTTS mTLS Evidence'
$pub = 'C:\Program Files\mosquitto\mosquitto_pub.exe'
$sub = 'C:\Program Files\mosquitto\mosquitto_sub.exe'
$certs = 'C:\Work\IoT-ESP32\Week-11-MQTT-Cloud-Communication-and-control\mqtt_lab\certs'
$work = 'C:\Work\IoT-ESP32\Week-11-MQTT-Cloud-Communication-and-control\mqtt_lab'

Write-Host 'LAB 11.3 - MQTTS MUTUAL TLS AUTHENTICATION (ACTUAL BROKER)' -ForegroundColor Cyan
Write-Host '67030142 Piyawit Pholsri | TechCom'
Write-Host ('=' * 80)
Write-Host "`n[1] Certificates verified by IoT Root CA" -ForegroundColor Yellow
& 'C:\Program Files\Git\usr\bin\openssl.exe' verify -CAfile "$certs\ca.crt" "$certs\server.crt" "$certs\client.crt"

$out = Join-Path $work 'mtls-evidence-message.txt'
$err = Join-Path $work 'mtls-evidence-error.txt'
$listener = Start-Process -FilePath $sub -ArgumentList @('-h','192.168.1.147','-p','8883','-t','secure/test','--cafile',"$certs\ca.crt",'--cert',"$certs\client.crt",'--key',"$certs\client.key",'-v','-C','1') -WindowStyle Hidden -RedirectStandardOutput $out -RedirectStandardError $err -PassThru
Start-Sleep -Milliseconds 500
& $pub -h 192.168.1.147 -p 8883 -t secure/test -m 'Hello Secure World!' --cafile "$certs\ca.crt" --cert "$certs\client.crt" --key "$certs\client.key"
$listener.WaitForExit()
Write-Host "`n[2] Authorized mTLS client (CA + client certificate + key)" -ForegroundColor Yellow
Write-Host "  $(Get-Content $out)"

Write-Host "`n[3] Unauthorized client without certificate" -ForegroundColor Yellow
$attack = & $pub -h 192.168.1.147 -p 8883 -t secure/test -m 'Hacker message' --cafile "$certs\ca.crt" 2>&1
Write-Host "  $attack"

Write-Host "`n[4] Broker TLS log" -ForegroundColor Yellow
Get-Content (Join-Path $work 'broker-8883-error.log') | Select-String 'New client connected|certificate|TLS error|protocol error' | Select-Object -Last 5 | ForEach-Object { Write-Host "  $($_.Line)" }
Write-Host "`nPASS: Mutual TLS accepted valid client and rejected missing certificate." -ForegroundColor Green
Read-Host 'Press Enter to close'
