idf_component_register(SRCS "Lab11-2_MQTT_Client.c"
                    INCLUDE_DIRS "."
                    REQUIRES mqtt cjson esp_wifi esp_event nvs_flash cjson esp_adc driver esp_driver_gpio)
