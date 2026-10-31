import time
import random
import sys

adc_val = 1750
volt = 1.41
uptime = 4108

try:
    for i in range(10):
        time.sleep(2.0)
        adc_val += random.randint(-30, 45)
        volt = round((adc_val / 4095.0) * 3.3, 3)
        uptime += 2000
        led = "true" if i >= 2 and i < 6 else "false"
        print(f'iot/node01/telemetry {{"node_id":"esp32-node01","adc_raw":{adc_val},"voltage_v":{volt:.3f},"led_state":{led},"uptime_ms":{uptime}}}')
        sys.stdout.flush()
except KeyboardInterrupt:
    pass

