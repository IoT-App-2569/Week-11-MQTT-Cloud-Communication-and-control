#!/bin/bash
echo "Starting Local Mosquitto Broker with Plain (1883) and mTLS (8883)..."
mosquitto -c mosquitto_local.conf -v
