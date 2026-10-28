# 2_test_wildcards.ps1 - ทดสอบการส่งข้อมูลแบบ Wildcards (+) และ (#)
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Testing Single-Level (+) and Multi-Level (#) Wildcards " -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan

$pubExe = "C:\Program Files\mosquitto\mosquitto_pub.exe"

Write-Host "`n[Step 1] Publishing to test Single-Level Wildcard (home/+/light)..." -ForegroundColor Green
& $pubExe -h 127.0.0.1 -p 1883 -t "home/livingroom/light" -m "ON (Livingroom)"
& $pubExe -h 127.0.0.1 -p 1883 -t "home/kitchen/light" -m "OFF (Kitchen)"
& $pubExe -h 127.0.0.1 -p 1883 -t "home/bedroom/balcony/light" -m "ON (Balcony - Should be filtered out by +)"

Write-Host "`n[Step 2] Publishing to test Multi-Level Wildcard (home/#)..." -ForegroundColor Green
& $pubExe -h 127.0.0.1 -p 1883 -t "home/livingroom/temp" -m "26.4 C"
& $pubExe -h 127.0.0.1 -p 1883 -t "home/bedroom/balcony/light" -m "ON (Multi-level matches all)"

Write-Host "`nTest messages sent successfully!" -ForegroundColor Cyan
