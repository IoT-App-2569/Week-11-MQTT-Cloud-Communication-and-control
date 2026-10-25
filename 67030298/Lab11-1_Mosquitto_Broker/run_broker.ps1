# Script สำหรับเริ่มต้นรัน Mosquitto Broker สำหรับการทดลองที่ 11.1
# ข้อควรระวัง: ปิด Windows Service ก่อนเสมอเพื่อไม่ให้เกิด Split-Brain Broker

Write-Host "Checking Mosquitto Windows Service..." -ForegroundColor Cyan
try {
    $service = Get-Service -Name mosquitto -ErrorAction SilentlyContinue
    if ($service -and $service.Status -eq 'Running') {
        Write-Host "Stopping background mosquitto service to prevent conflict..." -ForegroundColor Yellow
        Stop-Service mosquitto -ErrorAction SilentlyContinue
        Set-Service mosquitto -StartupType Manual -ErrorAction SilentlyContinue
    }
} catch {
    Write-Warning "Cannot manage service (Run PowerShell as Administrator if port 1883 is busy)."
}

$confPath = Join-Path $PSScriptRoot "mosquitto.conf"
Write-Host "Starting Mosquitto Broker using config: $confPath" -ForegroundColor Green

if (Get-Command mosquitto.exe -ErrorAction SilentlyContinue) {
    & mosquitto.exe -c "$confPath" -v
} elseif (Test-Path "C:\Program Files\mosquitto\mosquitto.exe") {
    & "C:\Program Files\mosquitto\mosquitto.exe" -c "$confPath" -v
} else {
    Write-Error "mosquitto.exe not found! Please check installation or PATH variable."
}
