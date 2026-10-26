#!/bin/bash
# Generate CA
openssl req -new -x509 -days 365 -nodes -out ca.crt -keyout ca.key -subj "/CN=IoT-CA"
# Server Cert
openssl req -new -nodes -out server.csr -keyout server.key -subj "/CN=192.168.1.100"
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 365
# Client Cert
openssl req -new -nodes -out client.csr -keyout client.key -subj "/CN=ESP32-Client"
openssl x509 -req -in client.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out client.crt -days 365
