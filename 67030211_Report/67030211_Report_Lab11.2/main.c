#include <stdio.h>
#include "mqtt_client.h"
#include "esp_log.h"

static esp_mqtt_client_handle_t client;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    if (event_id == MQTT_EVENT_CONNECTED) {
        printf("[MQTT] Connected to Broker! Subscribing to command topic...\n");
        esp_mqtt_client_subscribe(client, "iot/esp32/control", 1);
    } else if (event_id == MQTT_EVENT_DATA) {
        printf("[MQTT] Incoming Data -> Topic: %.*s | Payload: %.*s\n",
               event->topic_len, event->topic, event->data_len, event->data);
    }
}

void app_main(void) {
    esp_mqtt_client_config_t mqtt_cfg = { .broker.address.uri = "mqtt://192.168.1.100:1883" };
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}
