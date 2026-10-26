#!/bin/bash
openssl req -new -x509 -days 365 -nodes -out ca.crt -keyout ca.key -subj "/CN=IoT-CA"
openssl req -new -nodes -out client.csr -keyout client.key -subj "/CN=ESP32-Client"
openssl x509 -req -in client.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out client.crt -days 365
