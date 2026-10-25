idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    EMBED_TXTFILES 
        "certs/ca.crt"
        "certs/client.crt"
        "certs/client.key"
    REQUIRES mqtt esp_wifi esp_event nvs_flash driver esp_driver_gpio
)
