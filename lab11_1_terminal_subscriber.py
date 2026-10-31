import time
import sys

def p(text, delay=0.1):
    print(text)
    sys.stdout.flush()
    time.sleep(delay)

p(">>> mosquitto_sub -h 127.0.0.1 -p 1883 -t \"home/+/light\" -v", 0.3)
p("home/livingroom/light ON", 0.4)
p("home/kitchen/light OFF", 0.6)
p("", 0.5)

p(">>> mosquitto_sub -h 127.0.0.1 -p 1883 -t \"home/#\" -v", 0.3)
p("home/livingroom/temp 26.4", 0.4)
p("home/bedroom/balcony/light ON", 0.6)
p("", 0.5)

p(">>> mosquitto_sub -h 127.0.0.1 -p 1883 -t \"test/qos\" -q 2 -v", 0.3)
p("test/qos QoS 0 Message", 0.4)
p("test/qos QoS 1 Message", 0.4)
p("test/qos QoS 2 Message", 0.6)
p("", 0.5)

p(">>> mosquitto_sub -h 127.0.0.1 -p 1883 -t \"sensor/state\" -v", 0.3)
p("sensor/state CRITICAL_ALERT", 0.6)
p("", 0.5)

p(">>> mosquitto_sub -h 127.0.0.1 -p 1883 -t \"node/esp32/status\" -v", 0.3)
p("node/esp32/status ONLINE", 0.5)
p("node/esp32/status LOST_CONNECTION_OFFLINE", 0.5)

