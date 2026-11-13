@echo off
REM ==============================================================================
REM OpenSSL Certificate Generation Script for Lab 11.3 (Windows)
REM Student ID: 67030281
REM ==============================================================================

set BROKER_IP=192.168.1.111

echo [1/3] Generating Root CA (ca.key, ca.crt)...
openssl genrsa -out ca.key 2048
openssl req -out ca.csr -key ca.key -new -subj "/C=TH/ST=Bangkok/O=IoT Security Lab/CN=IoT Root CA"
openssl x509 -req -in ca.csr -out ca.crt -sha256 -days 3650 -signkey ca.key

echo [2/3] Generating Server Certificate for Mosquitto (server.key, server.crt)...
openssl genrsa -out server.key 2048
openssl req -out server.csr -key server.key -new -subj "/C=TH/ST=Bangkok/O=IoT Broker/CN=%BROKER_IP%"
openssl x509 -req -in server.csr -out server.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650

echo [3/3] Generating Client Certificate for ESP32 (client.key, client.crt)...
openssl genrsa -out client.key 2048
openssl req -out client.csr -key client.key -new -subj "/C=TH/ST=Bangkok/O=IoT Node/CN=esp32-secure-node"
openssl x509 -req -in client.csr -out client.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650

echo Copying certificates to main/certs for ESP-IDF embedding...
copy /Y ca.crt "..\main\certs\ca.crt"
copy /Y client.crt "..\main\certs\client.crt"
copy /Y client.key "..\main\certs\client.key"

echo [SUCCESS] All certificates created and copied successfully!
pause

