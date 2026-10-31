import time
import sys

def print_log(text, delay=0.1):
    print(text)
    sys.stdout.flush()
    time.sleep(delay)

t = int(time.time())
print_log(f"{t}: mosquitto version 2.0.18 starting", 0.2)
print_log(f"{t}: Config loaded from C:\\mqtt_lab\\mosquitto.conf.", 0.1)
print_log(f"{t}: Opening ipv4 listen socket on port 1883.", 0.1)
print_log(f"{t}: mosquitto version 2.0.18 running", 0.4)

# Section 1: Subscribe
t += 3
print_log(f"{t}: New connection from 127.0.0.1:52134 on port 1883.", 0.2)
print_log(f"{t}: New client connected from 127.0.0.1:52134 as sub_test (p2, c1, k60).", 0.2)
print_log(f"{t}: Received SUBSCRIBE from sub_test", 0.1)
print_log(f"{t}: \thome/+/light (QoS 0)", 0.1)
print_log(f"{t}: sub_test 0 home/+/light", 0.1)
print_log(f"{t}: Sending SUBACK to sub_test", 0.4)

# Section 2: QoS 0
t += 4
print_log(f"{t}: New connection from 127.0.0.1:52140 on port 1883.", 0.2)
print_log(f"{t}: Received PUBLISH from pub_qos0 (d0, q0, r0, m0, 'test/qos', ... (13 bytes))", 0.2)
print_log(f"{t}: Sending PUBLISH to sub_test (d0, q0, r0, m0, 'test/qos', ... (13 bytes))", 0.2)
print_log(f"{t}: Client pub_qos0 disconnected.", 0.4)

# Section 3: QoS 1
t += 3
print_log(f"{t}: New connection from 127.0.0.1:52145 on port 1883.", 0.2)
print_log(f"{t}: Received PUBLISH from pub_qos1 (d0, q1, r0, m1, 'test/qos', ... (13 bytes))", 0.2)
print_log(f"{t}: Sending PUBACK to pub_qos1 (Mid: 1)", 0.2)
print_log(f"{t}: Sending PUBLISH to sub_test (d0, q1, r0, m1, 'test/qos', ... (13 bytes))", 0.2)
print_log(f"{t}: Received PUBACK from sub_test (Mid: 1)", 0.2)
print_log(f"{t}: Client pub_qos1 disconnected.", 0.4)

# Section 4: QoS 2
t += 3
print_log(f"{t}: New connection from 127.0.0.1:52150 on port 1883.", 0.2)
print_log(f"{t}: Received PUBLISH from pub_qos2 (d0, q2, r0, m1, 'test/qos', ... (13 bytes))", 0.2)
print_log(f"{t}: Sending PUBREC to pub_qos2 (Mid: 1)", 0.2)
print_log(f"{t}: Received PUBREL from pub_qos2 (Mid: 1)", 0.2)
print_log(f"{t}: Sending PUBCOMP to pub_qos2 (Mid: 1)", 0.2)
print_log(f"{t}: Sending PUBLISH to sub_test (d0, q2, r0, m1, 'test/qos', ... (13 bytes))", 0.2)
print_log(f"{t}: Received PUBREC from sub_test (Mid: 1)", 0.2)
print_log(f"{t}: Sending PUBREL to sub_test (Mid: 1)", 0.2)
print_log(f"{t}: Received PUBCOMP to sub_test (Mid: 1)", 0.2)
print_log(f"{t}: Client pub_qos2 disconnected.", 0.4)
