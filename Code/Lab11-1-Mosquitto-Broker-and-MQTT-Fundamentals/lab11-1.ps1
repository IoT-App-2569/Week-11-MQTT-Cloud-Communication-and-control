<#
.SYNOPSIS
  Helper for Lab 11-1: runs one lab step in the current terminal.

.EXAMPLE
  .\lab11-1.ps1 broker        # Terminal 1
  .\lab11-1.ps1 sub-basic     # Terminal 2
  .\lab11-1.ps1 pub-basic     # Terminal 3

.NOTES
  Each step maps to one command in the labsheet (06-Labsheet-11-1).
  Override the broker address with -BrokerHost / -Port if needed.
#>
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [ValidateSet(
        'broker',
        # 11-1.2 wildcards
        'sub-basic', 'pub-basic',
        'sub-plus',  'pub-plus',
        'sub-hash',  'pub-hash',
        # 11-1.3 QoS (use 'sub-qos0|1|2' in terminal 2, then 'pub-qos0|1|2' in terminal 3)
        'sub-qos0', 'pub-qos0',
        'sub-qos1', 'pub-qos1',
        'sub-qos2', 'pub-qos2',
        # 11-1.4 retained
        'pub-normal', 'sub-retained', 'pub-retained', 'clear-retained',
        # 11-1.5 LWT
        'sub-status', 'lwt-client', 'pub-online'
    )]
    [string]$Step,

    [string]$BrokerHost = '127.0.0.1',
    [int]$Port = 1883
)

$ErrorActionPreference = 'Stop'

function Find-Mosq([string]$name) {
    $cmd = Get-Command $name -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    $p = Join-Path 'C:\Program Files\mosquitto' $name
    if (Test-Path $p) { return $p }
    throw "$name not found. Install Mosquitto or add 'C:\Program Files\mosquitto' to PATH."
}

$mosq = Find-Mosq 'mosquitto.exe'
$sub  = Find-Mosq 'mosquitto_sub.exe'
$pub  = Find-Mosq 'mosquitto_pub.exe'
$conf = Join-Path $PSScriptRoot 'mosquitto.conf'
$conn = @('-h', $BrokerHost, '-p', $Port)

switch ($Step) {
    'broker' {
        $svc = Get-Service mosquitto -ErrorAction SilentlyContinue
        if ($svc -and $svc.Status -eq 'Running') {
            Write-Warning "Windows service 'mosquitto' is running -> two brokers would conflict."
            Write-Warning "Run as Administrator:  Stop-Service mosquitto ; Set-Service mosquitto -StartupType Manual"
        }
        & $mosq -c $conf -v
    }

    # --- 11-1.2 ---------------------------------------------------------
    'sub-basic' { & $sub @conn -t 'home/livingroom/light' -v }
    'pub-basic' { & $pub @conn -t 'home/livingroom/light' -m 'ON' }

    'sub-plus'  { & $sub @conn -t 'home/+/light' -v }
    'pub-plus' {
        & $pub @conn -t 'home/livingroom/light'       -m 'ON'
        & $pub @conn -t 'home/kitchen/light'          -m 'OFF'
        & $pub @conn -t 'home/bedroom/balcony/light'  -m 'ON'   # NOT matched by +
    }

    'sub-hash'  { & $sub @conn -t 'home/#' -v }
    'pub-hash' {
        & $pub @conn -t 'home/livingroom/temp'        -m '26.4'
        & $pub @conn -t 'home/bedroom/balcony/light'  -m 'ON'
    }

    # --- 11-1.3 ---------------------------------------------------------
    'sub-qos0' { & $sub @conn -t 'test/qos' -q 0 -v }
    'pub-qos0' { & $pub @conn -t 'test/qos' -m 'QoS 0 Message' -q 0 }
    'sub-qos1' { & $sub @conn -t 'test/qos' -q 1 -v }
    'pub-qos1' { & $pub @conn -t 'test/qos' -m 'QoS 1 Message' -q 1 }
    'sub-qos2' { & $sub @conn -t 'test/qos' -q 2 -v }
    'pub-qos2' { & $pub @conn -t 'test/qos' -m 'QoS 2 Message' -q 2 }

    # --- 11-1.4 ---------------------------------------------------------
    'pub-normal'     { & $pub @conn -t 'sensor/state' -m 'Normal' }
    'sub-retained'   { & $sub @conn -t 'sensor/state' -v }
    'pub-retained'   { & $pub @conn -t 'sensor/state' -m 'CRITICAL_ALERT' -r }
    'clear-retained' { & $pub @conn -t 'sensor/state' -n -r }

    # --- 11-1.5 ---------------------------------------------------------
    'sub-status' { & $sub @conn -t 'node/esp32/status' -v }
    'lwt-client' {
        Write-Host 'Simulated ESP32 connected with LWT. Close this window abruptly (do NOT press Ctrl+C) to trigger the will.'
        & $sub @conn -i 'Node_Client_01' -t 'dummy/listen' `
            --will-topic 'node/esp32/status' --will-payload 'LOST_CONNECTION_OFFLINE' --will-qos 1
    }
    'pub-online' { & $pub @conn -t 'node/esp32/status' -m 'ONLINE' }
}
