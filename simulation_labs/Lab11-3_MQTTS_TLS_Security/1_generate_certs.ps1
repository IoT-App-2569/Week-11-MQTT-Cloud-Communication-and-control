# 1_generate_certs.ps1 - สร้างคู่กุญแจและใบรับรอง X.509 ผ่าน OpenSSL
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Generating X.509 Certificates for mTLS  " -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan

# ตรวจหา OpenSSL
$openssl = "openssl"
if (-not (Get-Command openssl -ErrorAction SilentlyContinue)) {
    if (Test-Path "D:\Git\usr\bin\openssl.exe") {
        $openssl = "D:\Git\usr\bin\openssl.exe"
    } elseif (Test-Path "C:\Program Files\Git\usr\bin\openssl.exe") {
        $openssl = "C:\Program Files\Git\usr\bin\openssl.exe"
    } else {
        Write-Error "OpenSSL binary not found! Please check installation."
        exit 1
    }
}

$certDir = Join-Path $PSScriptRoot "certs"
if (-not (Test-Path $certDir)) {
    New-Item -ItemType Directory -Path $certDir | Out-Null
}
Set-Location $certDir

Write-Host "`n[Step 1] Creating Root CA (Authority)..." -ForegroundColor Yellow
& $openssl genrsa -out ca.key 2048
& $openssl req -out ca.csr -key ca.key -new -subj "/C=TH/ST=Bangkok/O=IoT Security Lab/CN=IoT Root CA"
& $openssl x509 -req -in ca.csr -out ca.crt -sha256 -days 3650 -signkey ca.key

Write-Host "`n[Step 2] Creating Server Certificate (Mosquitto Broker @ 127.0.0.1)..." -ForegroundColor Yellow
& $openssl genrsa -out server.key 2048
& $openssl req -out server.csr -key server.key -new -subj "/C=TH/ST=Bangkok/O=IoT Broker/CN=127.0.0.1"
& $openssl x509 -req -in server.csr -out server.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650

Write-Host "`n[Step 3] Creating Client Certificate (ESP32 Node)..." -ForegroundColor Yellow
& $openssl genrsa -out client.key 2048
& $openssl req -out client.csr -key client.key -new -subj "/C=TH/ST=Bangkok/O=IoT Node/CN=esp32-secure-node"
& $openssl x509 -req -in client.csr -out client.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650

Write-Host "`nCertificates generated successfully in: $certDir" -ForegroundColor Green
Get-ChildItem -Path $certDir | Select-Object Name, Length, LastWriteTime | Format-Table -AutoSize
