# 4_test_retained.ps1 - ทดสอบข้อความคงค้าง (Retained Messages)
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Testing Retained Messages (-r flag) " -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan

$pubExe = "C:\Program Files\mosquitto\mosquitto_pub.exe"

Write-Host "`n1. Publishing Retained Message to sensor/state..." -ForegroundColor Yellow
& $pubExe -h 127.0.0.1 -p 1883 -t "sensor/state" -m "CRITICAL_ALERT_RETAINED" -r

Write-Host "Published with -r! Now any subscriber that connects will receive this immediately." -ForegroundColor Green
Write-Host "`nTo clear the retained message later, run: & `"$pubExe`" -h 127.0.0.1 -p 1883 -t `"sensor/state`" -n -r" -ForegroundColor Gray
