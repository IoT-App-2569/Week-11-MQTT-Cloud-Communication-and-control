idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES mqtt esp_wifi esp_event nvs_flash driver
    EMBED_TXTFILES 
        "certs/ca.crt"
        "certs/client.crt"
        "certs/client.key"
)
