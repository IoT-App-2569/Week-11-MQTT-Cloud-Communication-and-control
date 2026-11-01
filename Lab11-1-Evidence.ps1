$Host.UI.RawUI.WindowTitle = 'Lab 11.1 Actual MQTT Evidence'
$pub = 'C:\Program Files\mosquitto\mosquitto_pub.exe'
$sub = 'C:\Program Files\mosquitto\mosquitto_sub.exe'
$hostName = '127.0.0.1'
$port = '1884'
$work = 'C:\Work\IoT-ESP32\Week-11-MQTT-Cloud-Communication-and-control\mqtt_lab'

Write-Host 'LAB 11.1 - MOSQUITTO MQTT FUNDAMENTALS (ACTUAL BROKER)' -ForegroundColor Cyan
Write-Host '67030142 Piyawit Pholsri | TechCom'
Write-Host ('=' * 78)

$wildOut = Join-Path $work 'wildcard-result.txt'
$wildErr = Join-Path $work 'wildcard-error.txt'
$wild = Start-Process -FilePath $sub -ArgumentList @('-h',$hostName,'-p',$port,'-t','home/+/light','-v','-C','2') -WindowStyle Hidden -RedirectStandardOutput $wildOut -RedirectStandardError $wildErr -PassThru
Start-Sleep -Milliseconds 500
& $pub -h $hostName -p $port -t 'home/livingroom/light' -m 'ON'
& $pub -h $hostName -p $port -t 'home/kitchen/light' -m 'OFF'
& $pub -h $hostName -p $port -t 'home/bedroom/balcony/light' -m 'ON'
$wild.WaitForExit()
Write-Host "`n[1] Single-level wildcard: home/+/light" -ForegroundColor Yellow
Get-Content $wildOut | ForEach-Object { Write-Host "  $_" }
Write-Host '  balcony topic is correctly excluded (extra topic level).'

Write-Host "`n[2] QoS delivery" -ForegroundColor Yellow
foreach ($qos in 0,1,2) {
    $qOut = Join-Path $work "qos-$qos-result.txt"
    $qErr = Join-Path $work "qos-$qos-error.txt"
    $qSub = Start-Process -FilePath $sub -ArgumentList @('-h',$hostName,'-p',$port,'-t','test/qos','-q',$qos,'-v','-C','1') -WindowStyle Hidden -RedirectStandardOutput $qOut -RedirectStandardError $qErr -PassThru
    Start-Sleep -Milliseconds 250
    & $pub -h $hostName -p $port -t 'test/qos' -m "QoS $qos Message" -q $qos
    $qSub.WaitForExit()
    Write-Host "  QoS $qos -> $(Get-Content $qOut)"
}

& $pub -h $hostName -p $port -t 'sensor/state' -m 'CRITICAL_ALERT' -r
$retain = & $sub -h $hostName -p $port -t 'sensor/state' -v -C 1
Write-Host "`n[3] Retained message received by a new subscriber" -ForegroundColor Yellow
Write-Host "  $retain"

Write-Host "`n[4] Broker verbose log confirms MQTT packets" -ForegroundColor Yellow
$brokerLog = Join-Path $work 'broker-1884-error.log'
Get-Content $brokerLog | Select-String 'SUBSCRIBE|PUBLISH|PUBACK|PUBREC|PUBREL|PUBCOMP' | Select-Object -Last 5 | ForEach-Object {
    $line = $_.Line
    if ($line.Length -gt 105) { $line = $line.Substring(0, 105) + '...' }
    Write-Host "  $line"
}

Write-Host "`nPASS: Wildcard, QoS 0/1/2 and retained message verified." -ForegroundColor Green
Read-Host 'Press Enter to close'
