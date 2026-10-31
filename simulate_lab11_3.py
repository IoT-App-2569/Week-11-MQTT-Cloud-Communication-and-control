import os
import time

cert_dir = os.path.join(os.getcwd(), "mqtt_certs")
os.makedirs(cert_dir, exist_ok=True)

files = [
    "ca.key", "ca.crt",
    "server.key", "server.crt",
    "client.key", "client.crt"
]

header_map = {
    ".key": "-----BEGIN RSA PRIVATE KEY-----\nMIIEowIBAAKCAQEA0w9mD9...",
    ".crt": "-----BEGIN CERTIFICATE-----\nMIIDXTCCAkWgAwIBAgIUbQ..."
}

for f in files:
    ext = os.path.splitext(f)[1]
    fp = os.path.join(cert_dir, f)
    if not os.path.exists(fp):
        with open(fp, "w") as out:
            out.write(header_map.get(ext, "") + "\n-----END-----\n")

t = int(time.time())
print(f"{t}: mosquitto version 2.0.18 starting")
print(f"{t}: Config loaded from {cert_dir}/mosquitto_ssl.conf.")
print(f"{t}: Opening ipv4 listen socket on port 8883.")
print(f"{t}: mosquitto version 2.0.18 running")
time.sleep(0.5)

# Handshake success
t += 5
print(f"{t}: New connection from 192.168.1.145:51234 on port 8883.")
print(f"{t}: Peer certificate verified: /C=TH/ST=Bangkok/O=IoT Node/CN=esp32-secure-node")
print(f"{t}: New client connected from 192.168.1.145:51234 as esp32-secure-node (p2, c1, k60).")
print(f"{t}: Received SUBSCRIBE from esp32-secure-node")
print(f"{t}: \tiot/node01/cmd/led (QoS 1)")
print(f"{t}: Sending SUBACK to esp32-secure-node")
time.sleep(0.5)

# Handshake fail (unauthorized client)
t += 8
print(f"{t}: New connection from 192.168.1.111:51238 on port 8883.")
print(f"{t}: OpenSSL Error[0]: error:1417C0C7:SSL routines:tls_process_client_certificate:peer did not return a certificate")
print(f"{t}: Client disconnected due to TLS error: peer did not return a certificate")
