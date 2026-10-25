#!/bin/bash
set -e

BROKER_IP="${1:-192.168.1.100}"

echo "=========================================================="
echo " Generating MQTTS Certificates with OpenSSL (Lab 11.3)"
echo " Broker IP: ${BROKER_IP}"
echo "=========================================================="

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUT_DIR="${SCRIPT_DIR}/certs_out"
mkdir -p "${OUT_DIR}"
cd "${OUT_DIR}"

# 1. Root CA
echo "[1/3] Generating Root CA..."
openssl genrsa -out ca.key 2048
openssl req -out ca.csr -key ca.key -new -subj "/C=TH/ST=Bangkok/O=IoT Security Lab/CN=IoT Root CA"
openssl x509 -req -in ca.csr -out ca.crt -sha256 -days 3650 -signkey ca.key

# 2. Server Certificate
echo "[2/3] Generating Server Certificate for ${BROKER_IP}..."
openssl genrsa -out server.key 2048
openssl req -out server.csr -key server.key -new -subj "/C=TH/ST=Bangkok/O=IoT Broker/CN=${BROKER_IP}"
openssl x509 -req -in server.csr -out server.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650

# 3. Client Certificate
echo "[3/3] Generating Client Certificate for ESP32..."
openssl genrsa -out client.key 2048
openssl req -out client.csr -key client.key -new -subj "/C=TH/ST=Bangkok/O=IoT Node/CN=esp32-secure-node"
openssl x509 -req -in client.csr -out client.crt -sha256 -CA ca.crt -CAkey ca.key -CAcreateserial -days 3650

# Copy to main/certs
MAIN_CERTS_DIR="${SCRIPT_DIR}/../main/certs"
mkdir -p "${MAIN_CERTS_DIR}"
cp ca.crt client.crt client.key "${MAIN_CERTS_DIR}/"

echo "Certificates successfully generated and copied to ${MAIN_CERTS_DIR}"
