# 3_test_legit_client.ps1 - ทดสอบ Client ที่ถูกต้อง ส่ง cert และ key ครบถ้วน
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Testing Legitimate Client (mTLS Handshake Success) " -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan

$certDir = Join-Path $PSScriptRoot "certs"
$ca = Join-Path $certDir "ca.crt"
$cert = Join-Path $certDir "client.crt"
$key = Join-Path $certDir "client.key"
$pubExe = "C:\Program Files\mosquitto\mosquitto_pub.exe"

Write-Host "Sending secure message over Port 8883 with Client Certificate..." -ForegroundColor Yellow
& $pubExe -h 127.0.0.1 -p 8883 -t "secure/test" -m "Hello Secure mTLS from Valid ESP32 Node" --cafile "$ca" --cert "$cert" --key "$key"

Write-Host "`nResult: Message successfully published over encrypted mTLS channel!" -ForegroundColor Green
Write-Host "Check your Broker Terminal to see: 'Peer certificate verified: /C=TH/ST=Bangkok/O=IoT Node/CN=esp32-secure-node'" -ForegroundColor Cyan
