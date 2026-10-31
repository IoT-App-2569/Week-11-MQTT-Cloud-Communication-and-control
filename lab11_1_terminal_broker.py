import time
import sys

def p(text, delay=0.05):
    print(text)
    sys.stdout.flush()
    time.sleep(delay)

p("mosquitto version 2.0.18 starting", 0.2)
p("Config loaded from C:\\mqtt_lab\\mosquitto.conf.", 0.1)
p("Opening ipv4 listen socket on port 1883.", 0.1)
p("mosquitto version 2.0.18 running", 0.4)
p("", 0.2)

t = 1730432100

# Activity 11-1.1 & 11-1.2
p(f"{t+5}: New connection from 127.0.0.1:54321 on port 1883.")
p(f"{t+5}: New client connected from 127.0.0.1:54321 as sub_client (p2, c1, k60).")
p(f"{t+5}: Received SUBSCRIBE from sub_client")
p(f"{t+5}: \thome/+/light (QoS 0)")
p(f"{t+5}: sub_client 0 home/+/light")
p(f"{t+5}: Sending SUBACK to sub_client")
p("", 0.3)

p(f"{t+8}: New connection from 127.0.0.1:54322 on port 1883.")
p(f"{t+8}: Received PUBLISH from pub_client (d0, q0, r0, m0, 'home/livingroom/light', ... (2 bytes))")
p(f"{t+8}: Sending PUBLISH to sub_client (d0, q0, r0, m0, 'home/livingroom/light', ... (2 bytes))")
p(f"{t+9}: Received PUBLISH from pub_client (d0, q0, r0, m0, 'home/kitchen/light', ... (3 bytes))")
p(f"{t+9}: Sending PUBLISH to sub_client (d0, q0, r0, m0, 'home/kitchen/light', ... (3 bytes))")
p(f"{t+10}: Received PUBLISH from pub_client (d0, q0, r0, m0, 'home/bedroom/balcony/light', ... (2 bytes))")
p(f"{t+10}: Client pub_client disconnected.")
p("", 0.4)

# Activity 11-1.3 QoS 0, 1, 2
p(f"{t+15}: --- Testing QoS 0, 1, 2 ---")
p(f"{t+16}: Received PUBLISH from pub_qos0 (d0, q0, r0, m0, 'test/qos', ... (14 bytes))")
p(f"{t+16}: Sending PUBLISH to sub_client (d0, q0, r0, m0, 'test/qos', ... (14 bytes))")
p("", 0.2)
p(f"{t+20}: Received PUBLISH from pub_qos1 (d0, q1, r0, m1, 'test/qos', ... (14 bytes))")
p(f"{t+20}: Sending PUBACK to pub_qos1 (Mid: 1)")
p(f"{t+20}: Sending PUBLISH to sub_client (d0, q1, r0, m1, 'test/qos', ... (14 bytes))")
p(f"{t+20}: Received PUBACK from sub_client (Mid: 1)")
p("", 0.2)
p(f"{t+25}: Received PUBLISH from pub_qos2 (d0, q2, r0, m1, 'test/qos', ... (14 bytes))")
p(f"{t+25}: Sending PUBREC to pub_qos2 (Mid: 1)")
p(f"{t+25}: Received PUBREL from pub_qos2 (Mid: 1)")
p(f"{t+25}: Sending PUBCOMP to pub_qos2 (Mid: 1)")
p(f"{t+25}: Sending PUBLISH to sub_client (d0, q2, r0, m1, 'test/qos', ... (14 bytes))")
p(f"{t+25}: Received PUBREC from sub_client (Mid: 1)")
p(f"{t+25}: Sending PUBREL to sub_client (Mid: 1)")
p(f"{t+25}: Received PUBCOMP to sub_client (Mid: 1)")
p("", 0.4)

# Activity 11-1.4 Retained message
p(f"{t+30}: --- Retained Message ---")
p(f"{t+31}: Received PUBLISH from pub_client (d0, q0, r1, m0, 'sensor/state', ... (14 bytes))")
p(f"{t+35}: New connection from 127.0.0.1:54330 on port 1883.")
p(f"{t+35}: Received SUBSCRIBE from new_sub")
p(f"{t+35}: \tsensor/state (QoS 0)")
p(f"{t+35}: Sending PUBLISH to new_sub (d0, q0, r1, m0, 'sensor/state', ... (14 bytes))")
p("", 0.4)

# Activity 11-1.5 LWT
p(f"{t+40}: --- Last Will and Testament (LWT) ---")
p(f"{t+41}: Client Node_Client_01 has will topic 'node/esp32/status', will payload 'LOST_CONNECTION_OFFLINE'")
p(f"{t+45}: Socket error on client Node_Client_01, disconnecting.")
p(f"{t+45}: Sending will message to topic 'node/esp32/status'")
p(f"{t+45}: Sending PUBLISH to sub_client (d0, q1, r0, m1, 'node/esp32/status', ... (23 bytes))")

