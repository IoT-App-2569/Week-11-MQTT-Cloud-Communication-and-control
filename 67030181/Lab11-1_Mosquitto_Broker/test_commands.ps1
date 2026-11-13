# ==============================================================================
# PowerShell Test Script for MQTT Experiments (Lab 11.1)
# Student ID: 67030281
# ==============================================================================

Write-Host "==================================================================" -ForegroundColor Cyan
Write-Host " Lab 11.1: Mosquitto Broker Setup & MQTT Fundamentals Test Suite" -ForegroundColor Cyan
Write-Host " Student ID: 67030281" -ForegroundColor Cyan
Write-Host "==================================================================" -ForegroundColor Cyan

$BrokerIP = "127.0.0.1"
$BrokerPort = 1883

Write-Host "`n[Activity 11-1.2: Basic Pub/Sub & Wildcards]" -ForegroundColor Yellow
Write-Host "1. Exact Topic Subscription:"
Write-Host "   mosquitto_sub -h $BrokerIP -p $BrokerPort -t `"home/livingroom/light`" -v"
Write-Host "   mosquitto_pub -h $BrokerIP -p $BrokerPort -t `"home/livingroom/light`" -m `"ON`""

Write-Host "`n2. Single-Level Wildcard (+):"
Write-Host "   mosquitto_sub -h $BrokerIP -p $BrokerPort -t `"home/+/light`" -v"
Write-Host "   mosquitto_pub -h $BrokerIP -p $BrokerPort -t `"home/livingroom/light`" -m `"ON`""
Write-Host "   mosquitto_pub -h $BrokerIP -p $BrokerPort -t `"home/kitchen/light`" -m `"OFF`""
Write-Host "   mosquitto_pub -h $BrokerIP -p $BrokerPort -t `"home/bedroom/balcony/light`" -m `"ON`" (Will NOT match!)"

Write-Host "`n3. Multi-Level Wildcard (#):"
Write-Host "   mosquitto_sub -h $BrokerIP -p $BrokerPort -t `"home/#`" -v"
Write-Host "   mosquitto_pub -h $BrokerIP -p $BrokerPort -t `"home/livingroom/temp`" -m `"26.4`""
Write-Host "   mosquitto_pub -h $BrokerIP -p $BrokerPort -t `"home/bedroom/balcony/light`" -m `"ON`""

Write-Host "`n[Activity 11-1.3: Quality of Service (QoS 0, 1, 2)]" -ForegroundColor Yellow
Write-Host "QoS 0: mosquitto_pub -h $BrokerIP -p $BrokerPort -t `"test/qos`" -m `"QoS 0 Message`" -q 0"
Write-Host "QoS 1: mosquitto_pub -h $BrokerIP -p $BrokerPort -t `"test/qos`" -m `"QoS 1 Message`" -q 1"
Write-Host "QoS 2: mosquitto_pub -h $BrokerIP -p $BrokerPort -t `"test/qos`" -m `"QoS 2 Message`" -q 2"

Write-Host "`n[Activity 11-1.4: Retained Messages (-r)]" -ForegroundColor Yellow
Write-Host "Publish with retain: mosquitto_pub -h $BrokerIP -p $BrokerPort -t `"sensor/state`" -m `"CRITICAL_ALERT`" -r"
Write-Host "Verify subscribe:    mosquitto_sub -h $BrokerIP -p $BrokerPort -t `"sensor/state`" -v"
Write-Host "Clear retain:        mosquitto_pub -h $BrokerIP -p $BrokerPort -t `"sensor/state`" -n -r"

Write-Host "`n[Activity 11-1.5: Last Will and Testament (LWT)]" -ForegroundColor Yellow
Write-Host "Subscriber: mosquitto_sub -h $BrokerIP -p $BrokerPort -t `"node/esp32/status`" -v"
Write-Host "Node LWT:   mosquitto_sub -h $BrokerIP -p $BrokerPort -i `"Node_Client_01`" -t `"dummy/listen`" --will-topic `"node/esp32/status`" --will-payload `"LOST_CONNECTION_OFFLINE`" --will-qos 1"
Write-Host "Publish online: mosquitto_pub -h $BrokerIP -p $BrokerPort -t `"node/esp32/status`" -m `"ONLINE`""

