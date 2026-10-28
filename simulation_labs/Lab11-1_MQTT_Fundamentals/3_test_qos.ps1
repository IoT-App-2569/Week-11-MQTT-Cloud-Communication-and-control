# 3_test_qos.ps1 - ทดสอบส่งข้อความเปรียบเทียบ QoS 0, QoS 1, QoS 2
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Testing MQTT Quality of Service (QoS 0, 1, 2) " -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan

$pubExe = "C:\Program Files\mosquitto\mosquitto_pub.exe"

Write-Host "`n[Testing QoS 0: At most once]" -ForegroundColor Yellow
& $pubExe -h 127.0.0.1 -p 1883 -t "test/qos" -m "Message QoS 0 (Fire and forget)" -q 0
Start-Sleep -Milliseconds 500

Write-Host "`n[Testing QoS 1: At least once (with PUBACK)]" -ForegroundColor Yellow
& $pubExe -h 127.0.0.1 -p 1883 -t "test/qos" -m "Message QoS 1 (Guaranteed delivery)" -q 1
Start-Sleep -Milliseconds 500

Write-Host "`n[Testing QoS 2: Exactly once (4-Way Handshake)]" -ForegroundColor Yellow
& $pubExe -h 127.0.0.1 -p 1883 -t "test/qos" -m "Message QoS 2 (Exact delivery without duplicate)" -q 2

Write-Host "`nCheck your Mosquitto Broker Terminal to observe packet exchanges!" -ForegroundColor Green
