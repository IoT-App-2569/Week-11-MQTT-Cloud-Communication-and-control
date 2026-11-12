#!/usr/bin/env python3
"""
Certificate and Key Pair Generator for Lab 11.3 (MQTTS & mTLS)
Student ID: 67030181
Generates valid standard 2048-bit RSA keys and X.509 v3 certificates in PEM format.
"""

import os
import sys
import secrets
import hashlib
import base64
import time

def is_prime(n, k=8):
    if n < 2: return False
    if n in (2, 3): return True
    if n % 2 == 0: return False
    for p in [3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97]:
        if n % p == 0: return n == p
    r, d = 0, n - 1
    while d % 2 == 0:
        r += 1
        d //= 2
    for _ in range(k):
        a = secrets.randbelow(n - 4) + 2
        x = pow(a, d, n)
        if x == 1 or x == n - 1:
            continue
        for _ in range(r - 1):
            x = pow(x, 2, n)
            if x == n - 1:
                break
        else:
            return False
    return True

def get_prime(bits):
    while True:
        p = secrets.randbits(bits) | (1 << (bits - 1)) | 1
        if is_prime(p):
            return p

def gen_rsa_key(bits=2048, e=65537):
    p_bits = bits // 2
    while True:
        p = get_prime(p_bits)
        q = get_prime(bits - p_bits)
        if p == q: continue
        if p < q: p, q = q, p
        n = p * q
        phi = (p - 1) * (q - 1)
        if math_gcd(e, phi) == 1:
            break
    d = pow(e, -1, phi)
    dp = d % (p - 1)
    dq = d % (q - 1)
    qi = pow(q, -1, p)
    return {
        'n': n, 'e': e, 'd': d, 'p': p, 'q': q,
        'dp': dp, 'dq': dq, 'qi': qi, 'bits': bits
    }

def math_gcd(a, b):
    while b:
        a, b = b, a % b
    return a

def der_len(n):
    if n < 128:
        return bytes([n])
    b = []
    while n > 0:
        b.append(n & 0xFF)
        n >>= 8
    b.reverse()
    return bytes([0x80 | len(b)]) + bytes(b)

def der_tlv(tag, val):
    return bytes([tag]) + der_len(len(val)) + val

def der_int(n):
    if n == 0:
        return der_tlv(0x02, b'\x00')
    b = []
    while n > 0:
        b.append(n & 0xFF)
        n >>= 8
    if b[-1] & 0x80:
        b.append(0x00)
    b.reverse()
    return der_tlv(0x02, bytes(b))

def der_seq(items):
    return der_tlv(0x30, b''.join(items))

def der_set(items):
    return der_tlv(0x31, b''.join(items))

def der_bitstring(data):
    return der_tlv(0x03, bytes([0x00]) + data)

def rsa_to_pkcs1_pem(key):
    seq = der_seq([
        der_int(0),
        der_int(key['n']),
        der_int(key['e']),
        der_int(key['d']),
        der_int(key['p']),
        der_int(key['q']),
        der_int(key['dp']),
        der_int(key['dq']),
        der_int(key['qi'])
    ])
    b64 = base64.b64encode(seq).decode('ascii')
    lines = [b64[i:i+64] for i in range(0, len(b64), 64)]
    return "-----BEGIN RSA PRIVATE KEY-----\n" + "\n".join(lines) + "\n-----END RSA PRIVATE KEY-----\n"

def rsa_public_key_info(key):
    alg_id = der_seq([
        der_tlv(0x06, bytes.fromhex('2a864886f70d010101')),
        der_tlv(0x05, b'')
    ])
    pub_key_seq = der_seq([
        der_int(key['n']),
        der_int(key['e'])
    ])
    return der_seq([alg_id, der_bitstring(pub_key_seq)])

def rsa_sign_sha256(key, data):
    k = key['bits'] // 8
    h = hashlib.sha256(data).digest()
    prefix = bytes.fromhex('3031300d060960864801650304020105000420')
    t = prefix + h
    ps_len = k - len(t) - 3
    em = b'\x00\x01' + (b'\xff' * ps_len) + b'\x00' + t
    m = int.from_bytes(em, 'big')
    s = pow(m, key['d'], key['n'])
    return s.to_bytes(k, 'big')

def encode_name(c, st, o, cn):
    rdns = []
    for oid_hex, val in [
        ('550406', c),
        ('550408', st),
        ('55040a', o),
        ('550403', cn)
    ]:
        if val:
            attr = der_seq([
                der_tlv(0x06, bytes.fromhex(oid_hex)),
                der_tlv(0x0c, val.encode('utf-8'))
            ])
            rdns.append(der_set([attr]))
    return der_seq(rdns)

def encode_validity(days=3650):
    t_now = int(time.time())
    t_end = t_now + days * 86400
    s_now = time.strftime('%y%m%d%H%M%SZ', time.gmtime(t_now))
    s_end = time.strftime('%y%m%d%H%M%SZ', time.gmtime(t_end))
    return der_seq([
        der_tlv(0x17, s_now.encode('ascii')),
        der_tlv(0x17, s_end.encode('ascii'))
    ])

def make_cert(subject_info, issuer_info, pub_key, signing_key, serial, is_ca=False, days=3650):
    sig_alg = der_seq([
        der_tlv(0x06, bytes.fromhex('2a864886f70d01010b')),
        der_tlv(0x05, b'')
    ])
    extensions = []
    if is_ca:
        bc_val = der_seq([der_tlv(0x01, b'\xff')])
        ext_ca = der_seq([
            der_tlv(0x06, bytes.fromhex('551d13')),
            der_tlv(0x01, b'\xff'),
            der_tlv(0x04, bc_val)
        ])
        extensions.append(ext_ca)
        
    tbs_items = [
        der_tlv(0xa0, der_int(2)),
        der_int(serial),
        sig_alg,
        encode_name(**issuer_info),
        encode_validity(days),
        encode_name(**subject_info),
        rsa_public_key_info(pub_key)
    ]
    if extensions:
        tbs_items.append(der_tlv(0xa3, der_seq(extensions)))
        
    tbs = der_seq(tbs_items)
    sig = rsa_sign_sha256(signing_key, tbs)
    cert_der = der_seq([
        tbs,
        sig_alg,
        der_bitstring(sig)
    ])
    b64 = base64.b64encode(cert_der).decode('ascii')
    lines = [b64[i:i+64] for i in range(0, len(b64), 64)]
    return "-----BEGIN CERTIFICATE-----\n" + "\n".join(lines) + "\n-----END CERTIFICATE-----\n"

def main():
    out_dir = os.path.dirname(os.path.abspath(__file__))
    print(f"[INFO] Generating 2048-bit RSA keys and certificates in: {out_dir}")

    # 1. Root CA
    print("[1/3] Generating Root CA (ca.key, ca.crt)...")
    ca_key = gen_rsa_key(2048)
    ca_info = {'c': 'TH', 'st': 'Bangkok', 'o': 'IoT Security Lab', 'cn': 'IoT Root CA'}
    ca_crt = make_cert(ca_info, ca_info, ca_key, ca_key, serial=1, is_ca=True, days=3650)
    with open(os.path.join(out_dir, 'ca.key'), 'w') as f:
        f.write(rsa_to_pkcs1_pem(ca_key))
    with open(os.path.join(out_dir, 'ca.crt'), 'w') as f:
        f.write(ca_crt)

    # 2. Server Certificate
    print("[2/3] Generating Broker Server Certificate (server.key, server.crt)...")
    server_key = gen_rsa_key(2048)
    server_info = {'c': 'TH', 'st': 'Bangkok', 'o': 'IoT Broker', 'cn': '192.168.1.111'}
    server_crt = make_cert(server_info, ca_info, server_key, ca_key, serial=2, is_ca=False, days=3650)
    with open(os.path.join(out_dir, 'server.key'), 'w') as f:
        f.write(rsa_to_pkcs1_pem(server_key))
    with open(os.path.join(out_dir, 'server.crt'), 'w') as f:
        f.write(server_crt)

    # 3. Client Certificate
    print("[3/3] Generating ESP32 Client Certificate (client.key, client.crt)...")
    client_key = gen_rsa_key(2048)
    client_info = {'c': 'TH', 'st': 'Bangkok', 'o': 'IoT Node', 'cn': 'esp32-secure-node'}
    client_crt = make_cert(client_info, ca_info, client_key, ca_key, serial=3, is_ca=False, days=3650)
    with open(os.path.join(out_dir, 'client.key'), 'w') as f:
        f.write(rsa_to_pkcs1_pem(client_key))
    with open(os.path.join(out_dir, 'client.crt'), 'w') as f:
        f.write(client_crt)

    print("[SUCCESS] All certificate and key files generated successfully!")

if __name__ == '__main__':
    main()

