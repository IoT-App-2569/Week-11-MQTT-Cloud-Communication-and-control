<#
.SYNOPSIS
  Helper for Lab 11-3 (MQTTS / mTLS): runs one lab step in the current terminal.

.EXAMPLE
  .\lab11-3.ps1 certs                 # 11-3.1 create CA / server / client certificates
  .\lab11-3.ps1 broker                # 11-3.2 Terminal 1: Mosquitto on 8883 (mTLS)
  .\lab11-3.ps1 sub                   # 11-3.3 Terminal 2: valid subscriber
  .\lab11-3.ps1 pub                   # 11-3.3 Terminal 3: valid publisher
  .\lab11-3.ps1 attack                # 11-3.3 publisher WITHOUT client cert (must be refused)
  .\lab11-3.ps1 plain                 # 11-3.3 publisher on plain 1883 against the TLS broker (must fail)
  .\lab11-3.ps1 show                  # print CN / validity / issuer of every certificate

.NOTES
  -BrokerIp is the IP address of THIS computer. It goes into the server certificate
  (CN + SAN), so run `certs` again whenever the IP changes.
  Files are written to .\certs and copied to .\main\certs for the ESP32 firmware.
#>
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [ValidateSet('certs', 'broker', 'sub', 'pub', 'attack', 'plain', 'show')]
    [string]$Step,

    [string]$BrokerIp = '10.131.32.114',
    [int]$Port = 8883
)

$ErrorActionPreference = 'Stop'
$Certs = Join-Path $PSScriptRoot 'certs'
$EspCerts = Join-Path $PSScriptRoot 'main\certs'

function Find-Tool([string]$name, [string[]]$extra) {
    $cmd = Get-Command $name -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    foreach ($p in $extra) { if (Test-Path $p) { return $p } }
    throw "$name not found. Install it and/or add it to PATH."
}
function Mosq([string]$name) {
    Find-Tool "$name.exe" @("C:\Program Files\mosquitto\$name.exe")
}
function OpenSsl {
    Find-Tool 'openssl.exe' @('C:\Program Files\Git\usr\bin\openssl.exe', 'C:\Program Files\OpenSSL-Win64\bin\openssl.exe')
}
function Run([string]$exe, [string[]]$arguments) {
    # openssl prints progress ("... self-signature ok") on stderr; do not treat that as an error
    $old = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    & $exe @arguments 2>&1 | ForEach-Object { "$_" }
    $code = $LASTEXITCODE
    $ErrorActionPreference = $old
    if ($code -ne 0) { throw "$exe exited with code $code" }
}
function Need-Certs {
    if (-not (Test-Path (Join-Path $Certs 'ca.crt'))) {
        throw "No certificates yet. Run: .\lab11-3.ps1 certs -BrokerIp <your PC IP>"
    }
}
$TlsArgs = @('--cafile', (Join-Path $Certs 'ca.crt'),
             '--cert',   (Join-Path $Certs 'client.crt'),
             '--key',    (Join-Path $Certs 'client.key'))

switch ($Step) {
    'certs' {
        $ssl = OpenSsl
        $env:MSYS2_ARG_CONV_EXCL = '*'          # stop Git-for-Windows openssl from rewriting "/C=TH/..."
        New-Item -ItemType Directory -Force $Certs | Out-Null
        Push-Location $Certs
        try {
            # X.509 v3 extensions (mbedTLS in ESP-IDF 6 and OpenSSL 3 expect v3 certificates)
            Set-Content ca.ext     "basicConstraints=critical,CA:TRUE`nkeyUsage=critical,keyCertSign,cRLSign`nsubjectKeyIdentifier=hash" -Encoding ascii
            Set-Content server.ext "basicConstraints=CA:FALSE`nkeyUsage=digitalSignature,keyEncipherment`nextendedKeyUsage=serverAuth`nsubjectAltName=IP:$BrokerIp" -Encoding ascii
            Set-Content client.ext "basicConstraints=CA:FALSE`nkeyUsage=digitalSignature,keyEncipherment`nextendedKeyUsage=clientAuth" -Encoding ascii

            Write-Host '== 1. Root CA ==' -ForegroundColor Cyan
            Run $ssl @('genrsa', '-out', 'ca.key', '2048')
            Run $ssl @('req', '-out', 'ca.csr', '-key', 'ca.key', '-new', '-subj', '/C=TH/ST=Bangkok/O=IoT Security Lab/CN=IoT Root CA')
            Run $ssl @('x509', '-req', '-in', 'ca.csr', '-out', 'ca.crt', '-sha256', '-days', '3650', '-signkey', 'ca.key', '-extfile', 'ca.ext')

            Write-Host "== 2. Server (CN=$BrokerIp) ==" -ForegroundColor Cyan
            Run $ssl @('genrsa', '-out', 'server.key', '2048')
            Run $ssl @('req', '-out', 'server.csr', '-key', 'server.key', '-new', '-subj', "/C=TH/ST=Bangkok/O=IoT Broker/CN=$BrokerIp")
            Run $ssl @('x509', '-req', '-in', 'server.csr', '-out', 'server.crt', '-sha256', '-CA', 'ca.crt', '-CAkey', 'ca.key', '-CAcreateserial', '-days', '3650', '-extfile', 'server.ext')

            Write-Host '== 3. Client (CN=esp32-secure-node) ==' -ForegroundColor Cyan
            Run $ssl @('genrsa', '-out', 'client.key', '2048')
            Run $ssl @('req', '-out', 'client.csr', '-key', 'client.key', '-new', '-subj', '/C=TH/ST=Bangkok/O=IoT Node/CN=esp32-secure-node')
            Run $ssl @('x509', '-req', '-in', 'client.csr', '-out', 'client.crt', '-sha256', '-CA', 'ca.crt', '-CAkey', 'ca.key', '-CAcreateserial', '-days', '3650', '-extfile', 'client.ext')
        }
        finally { Pop-Location }

        # Broker config (absolute paths with forward slashes, as required by Mosquitto on Windows)
        $fwd = ($Certs -replace '\\', '/')
        $conf = @"
# MQTTS on 8883 with mutual TLS (Lab 11-3)
listener $Port 0.0.0.0

cafile   $fwd/ca.crt
certfile $fwd/server.crt
keyfile  $fwd/server.key

# client must present a certificate signed by ca.crt
require_certificate true

# Mosquitto 2.x refuses clients without username/password ("not authorised") unless told otherwise.
# The certificate is the credential here, so accept it and use its CN as the username.
use_identity_as_username true
allow_anonymous true

tls_version tlsv1.2

log_type all
"@
        Set-Content (Join-Path $Certs 'mosquitto_ssl.conf') $conf -Encoding ascii

        # files embedded into the ESP32 firmware
        New-Item -ItemType Directory -Force $EspCerts | Out-Null
        foreach ($f in 'ca.crt', 'client.crt', 'client.key') {
            Copy-Item (Join-Path $Certs $f) $EspCerts -Force
        }
        Write-Host "`nDone. Files in $Certs and copied ca.crt/client.crt/client.key to $EspCerts" -ForegroundColor Green
        Write-Host "Server certificate CN/SAN = $BrokerIp  (must equal MQTTS_BROKER_URI in .env)"
    }

    'show' {
        Need-Certs
        $ssl = OpenSsl
        foreach ($f in 'ca.crt', 'server.crt', 'client.crt') {
            Write-Host "--- $f ---" -ForegroundColor Cyan
            Run $ssl @('x509', '-in', (Join-Path $Certs $f), '-noout', '-subject', '-issuer', '-dates')
            if ($f -eq 'server.crt') {
                Run $ssl @('x509', '-in', (Join-Path $Certs $f), '-noout', '-ext', 'subjectAltName')
            }
        }
        Write-Host '--- verify chain ---' -ForegroundColor Cyan
        Run $ssl @('verify', '-CAfile', (Join-Path $Certs 'ca.crt'), (Join-Path $Certs 'server.crt'), (Join-Path $Certs 'client.crt'))
    }

    'broker' {
        Need-Certs
        $svc = Get-Service mosquitto -ErrorAction SilentlyContinue
        if ($svc -and $svc.Status -eq 'Running') {
            Write-Warning "Windows service 'mosquitto' is running (port 1883). Stop it from an Admin PowerShell: Stop-Service mosquitto"
        }
        Write-Host "Starting Mosquitto (mTLS) on port $Port ... Ctrl+C to stop" -ForegroundColor Cyan
        & (Mosq 'mosquitto') -c (Join-Path $Certs 'mosquitto_ssl.conf') -v
    }

    'sub' {
        Need-Certs
        & (Mosq 'mosquitto_sub') -h $BrokerIp -p $Port -t 'secure/test' @TlsArgs -v
    }

    'pub' {
        Need-Certs
        & (Mosq 'mosquitto_pub') -h $BrokerIp -p $Port -t 'secure/test' -m 'Hello Secure World!' @TlsArgs
    }

    'attack' {
        Need-Certs
        # only --cafile: no client certificate -> broker must refuse
        & (Mosq 'mosquitto_pub') -h $BrokerIp -p $Port -t 'secure/test' -m 'Hacker message' --cafile (Join-Path $Certs 'ca.crt')
    }

    'plain' {
        # no TLS at all, to port 1883 (the TLS broker does not listen there)
        & (Mosq 'mosquitto_pub') -h $BrokerIp -p 1883 -t 'secure/test' -m 'plain text'
    }
}
