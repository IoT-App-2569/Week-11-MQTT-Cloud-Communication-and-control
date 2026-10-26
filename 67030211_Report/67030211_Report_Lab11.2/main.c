#include <stdio.h>
#include "mqtt_client.h"
void app_main(void) {
    esp_mqtt_client_config_t cfg = { .broker.address.uri = "mqtt://192.168.1.100:1883" };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&cfg);
    esp_mqtt_client_start(client);
    printf("ESP32 MQTT Client Connected!\n");
}
