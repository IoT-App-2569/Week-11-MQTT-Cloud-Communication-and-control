idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    EMBED_TXTFILES
        "certs/ca.crt"
        "certs/client.crt"
        "certs/client.key"
    PRIV_REQUIRES espressif__mqtt esp_wifi esp_event esp_netif nvs_flash
)
