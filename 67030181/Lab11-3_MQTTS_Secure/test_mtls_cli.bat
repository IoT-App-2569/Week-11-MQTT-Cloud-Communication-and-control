@echo off
REM ==============================================================================
REM Test CLI Scripts for MQTTS & mTLS (Lab 11.3)
REM Student ID: 67030281
REM ==============================================================================

set BROKER_IP=192.168.1.111
set BROKER_PORT=8883

echo ================================================================
echo  Lab 11.3 MQTTS & Mutual TLS (mTLS) Security Verification Menu
echo  Broker Target: %BROKER_IP%:%BROKER_PORT%
echo ================================================================
echo [1] Start Mosquitto MQTTS Broker (Port 8883)
echo [2] Authorized Subscriber (with ca.crt, client.crt, client.key)
echo [3] Authorized Publisher (Send "Hello Secure World!")
echo [4] UNAUTHORIZED ATTACK TEST (Publish without client cert -> Must Fail!)
echo [5] Exit
echo ================================================================
choice /c 12345 /m "Select option:"

if errorlevel 5 goto end
if errorlevel 4 goto attack_test
if errorlevel 3 goto auth_pub
if errorlevel 2 goto auth_sub
if errorlevel 1 goto run_broker

:run_broker
echo Starting Mosquitto MQTTS Broker on port 8883...
mosquitto -c "%~dp0mosquitto_ssl.conf" -v
pause
goto end

:auth_sub
echo Subscribing securely with valid mTLS certificate...
cd "%~dp0certs"
mosquitto_sub -h %BROKER_IP% -p %BROKER_PORT% -t "secure/test" --cafile ca.crt --cert client.crt --key client.key -v
pause
goto end

:auth_pub
echo Publishing securely with valid mTLS certificate...
cd "%~dp0certs"
mosquitto_pub -h %BROKER_IP% -p %BROKER_PORT% -t "secure/test" -m "Hello Secure World!" --cafile ca.crt --cert client.crt --key client.key
echo Message published!
pause
goto end

:attack_test
echo ====================================================================
echo Simulating Unauthorized Client Attack (Missing Client Certificate)...
echo ====================================================================
cd "%~dp0certs"
mosquitto_pub -h %BROKER_IP% -p %BROKER_PORT% -t "secure/test" -m "Hacker Infiltration Attempt" --cafile ca.crt
echo.
echo Notice: Broker should reject this connection due to missing client certificate!
pause
goto end

:end

