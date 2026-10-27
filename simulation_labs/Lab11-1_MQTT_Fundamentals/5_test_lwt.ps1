# 5_test_lwt.ps1 - จำลองการตั้งค่า Last Will and Testament (LWT)
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Testing Last Will and Testament (LWT) " -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan

$subExe = "C:\Program Files\mosquitto\mosquitto_sub.exe"

Write-Host "Connecting simulated client with LWT configuration..." -ForegroundColor Green
Write-Host "Will Topic  : node/esp32/status" -ForegroundColor Yellow
Write-Host "Will Payload: LOST_CONNECTION_OFFLINE" -ForegroundColor Yellow
Write-Host "`n[INSTRUCTION]:" -ForegroundColor Red
Write-Host "To trigger LWT, close this terminal window violently (click X or Kill Terminal)." -ForegroundColor White
Write-Host "Broker will detect abrupt disconnection and publish the LWT payload automatically.`n" -ForegroundColor White

& $subExe -h 127.0.0.1 -p 1883 -i "Simulated_ESP32_LWT_Node" -t "dummy/keepalive" --will-topic "node/esp32/status" --will-payload "LOST_CONNECTION_OFFLINE" --will-qos 1
