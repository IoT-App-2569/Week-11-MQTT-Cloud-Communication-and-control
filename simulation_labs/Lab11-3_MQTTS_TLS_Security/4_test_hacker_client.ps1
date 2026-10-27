# 4_test_hacker_client.ps1 - จำลองแฮกเกอร์ที่ไม่มีใบรับรอง Client
Write-Host "========================================" -ForegroundColor Red
Write-Host " Testing Hacker Client (No Client Certificate - Should Be Rejected) " -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Red

$certDir = Join-Path $PSScriptRoot "certs"
$ca = Join-Path $certDir "ca.crt"
$pubExe = "C:\Program Files\mosquitto\mosquitto_pub.exe"

Write-Host "Attempting unauthorized connection without client certificate..." -ForegroundColor Yellow
& $pubExe -h 127.0.0.1 -p 8883 -t "secure/test" -m "Hacker Infiltration Message" --cafile "$ca"

Write-Host "`nResult: Connection was forcefully rejected by Broker!" -ForegroundColor Red
Write-Host "Check your Broker Terminal to see: 'Client disconnected due to TLS error: peer did not return a certificate'" -ForegroundColor Cyan
