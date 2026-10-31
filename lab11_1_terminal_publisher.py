import time
import sys

def p(text, delay=0.15):
    print(text)
    sys.stdout.flush()
    time.sleep(delay)

p(">>> mosquitto_pub -h 127.0.0.1 -p 1883 -t \"home/livingroom/light\" -m \"ON\"", 0.3)
p(">>> mosquitto_pub -h 127.0.0.1 -p 1883 -t \"home/kitchen/light\" -m \"OFF\"", 0.3)
p(">>> mosquitto_pub -h 127.0.0.1 -p 1883 -t \"home/bedroom/balcony/light\" -m \"ON\"", 0.5)
p("", 0.4)

p(">>> mosquitto_pub -h 127.0.0.1 -p 1883 -t \"home/livingroom/temp\" -m \"26.4\"", 0.3)
p(">>> mosquitto_pub -h 127.0.0.1 -p 1883 -t \"home/bedroom/balcony/light\" -m \"ON\"", 0.5)
p("", 0.4)

p(">>> mosquitto_pub -h 127.0.0.1 -p 1883 -t \"test/qos\" -m \"QoS 0 Message\" -q 0", 0.3)
p(">>> mosquitto_pub -h 127.0.0.1 -p 1883 -t \"test/qos\" -m \"QoS 1 Message\" -q 1", 0.3)
p(">>> mosquitto_pub -h 127.0.0.1 -p 1883 -t \"test/qos\" -m \"QoS 2 Message\" -q 2", 0.5)
p("", 0.4)

p(">>> mosquitto_pub -h 127.0.0.1 -p 1883 -t \"sensor/state\" -m \"CRITICAL_ALERT\" -r", 0.5)
p("", 0.4)

p(">>> mosquitto_pub -h 127.0.0.1 -p 1883 -t \"node/esp32/status\" -m \"ONLINE\"", 0.5)

