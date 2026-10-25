param (
    [string]$BrokerIP = "192.168.1.100"
)

if (-not (Get-Command openssl -ErrorAction SilentlyContinue)) {
    if (Test-Path "C:\Program Files\Git\usr\bin\openssl.exe") {
        $env:Path = "C:\Program Files\Git\usr\bin;" + $env:Path
    }
}

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host " Generating MQTTS Certificates with OpenSSL (Lab 11.3)" -ForegroundColor Cyan
Write-Host " Broker IP: $BrokerIP" -ForegroundColor Yellow
Write-Host "==========================================================" -ForegroundColor Cyan

$OutDir = Join-Path $PSScriptRoot "certs_out"
if (-not (Test-Path $OutDir)) { New-Item -ItemType Directory -Path $OutDir | Out-Null }

Push-Location $OutDir

# 1. Root CA
Write-Host "`n[1/3] Generating Root CA..." -ForegroundColor Green
openssl genrsa -out ca.key 2048
openssl req -out ca.csr -key ca.key -new -subj "/C=TH/ST=Bangkok/O=IoT Security Lab/CN=IoT Root CA"
openssl x509 -req -in ca.csr -out ca.crt -sha256 -days 3650 -signkey ca.key -extfile $PSScriptRoot/ca_ext.cnf

# 2. Server Certificate (Mosquitto)
Write-Host "`n[2/3] Generating Server Certificate for $BrokerIP..." -ForegroundColor Green
openssl genrsa -out server.key 2048
openssl req -out server.csr -key server.key -new -subj "/C=TH/ST=Bangkok/O=IoT Broker/CN=$BrokerIP"
openssl x509 -req -in server.csr -out server.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650 -extfile $PSScriptRoot/server_ext.cnf

# 3. Client Certificate (ESP32)
Write-Host "`n[3/3] Generating Client Certificate for ESP32..." -ForegroundColor Green
openssl genrsa -out client.key 2048
openssl req -out client.csr -key client.key -new -subj "/C=TH/ST=Bangkok/O=IoT Node/CN=esp32-secure-node"
openssl x509 -req -in client.csr -out client.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650 -extfile $PSScriptRoot/client_ext.cnf

Pop-Location

# Copy to main/certs for ESP-IDF EMBED_TXTFILES
$MainCertsDir = Join-Path $PSScriptRoot "..\main\certs"
if (-not (Test-Path $MainCertsDir)) { New-Item -ItemType Directory -Path $MainCertsDir | Out-Null }

Copy-Item (Join-Path $OutDir "ca.crt") -Destination $MainCertsDir -Force
Copy-Item (Join-Path $OutDir "client.crt") -Destination $MainCertsDir -Force
Copy-Item (Join-Path $OutDir "client.key") -Destination $MainCertsDir -Force

Write-Host "`nSuccessfully copied ca.crt, client.crt, client.key to: $MainCertsDir" -ForegroundColor Cyan
Write-Host "Certificates generated in: $OutDir" -ForegroundColor Cyan
Write-Host "Done!" -ForegroundColor Green
