#!/usr/bin/env bash
# กิจกรรม 11-3.1 — สร้าง Root CA, Server Certificate (Mosquitto) และ Client Certificate (ESP32) ด้วย OpenSSL
# ใช้งาน: ./gen_certs.sh <IP ของเครื่องที่รัน Broker>   (ค่าเริ่มต้น 172.20.10.2)
# ไฟล์ *.key เป็นกุญแจส่วนตัว อยู่ใน .gitignore — ต้องรันสคริปต์นี้ก่อน build โปรเจกต์ ESP32
set -e
cd "$(dirname "$0")"
BROKER_IP=${1:-172.20.10.2}

# 1. Root CA (ผู้รับรอง)
openssl genrsa -out ca.key 2048
openssl req -out ca.csr -key ca.key -new -subj "/C=TH/ST=Bangkok/O=IoT Security Lab/CN=IoT Root CA"
openssl x509 -req -in ca.csr -out ca.crt -sha256 -days 3650 -signkey ca.key -extfile ca_ext.cnf -extensions v3_ca

# 2. Server (Mosquitto Broker) — CN ต้องตรงกับ IP ที่ Client ใช้เชื่อมต่อ
openssl genrsa -out server.key 2048
openssl req -out server.csr -key server.key -new -subj "/C=TH/ST=Bangkok/O=IoT Broker/CN=${BROKER_IP}"
openssl x509 -req -in server.csr -out server.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650

# 3. Client (ESP32)
openssl genrsa -out client.key 2048
openssl req -out client.csr -key client.key -new -subj "/C=TH/ST=Bangkok/O=IoT Node/CN=esp32-secure-node"
openssl x509 -req -in client.csr -out client.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650

# คัดลอกไฟล์ที่ ESP32 ต้องใช้ไปไว้ใน main/certs/ (EMBED_TXTFILES)
mkdir -p ../main/certs
cp ca.crt client.crt client.key ../main/certs/
echo "สร้างใบรับรองเสร็จ (Server CN=${BROKER_IP})"
