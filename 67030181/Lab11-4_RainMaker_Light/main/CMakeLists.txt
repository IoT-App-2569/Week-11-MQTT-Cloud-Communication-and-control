idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES esp_rainmaker nvs_flash esp_wifi esp_event esp_netif driver esp_driver_gpio
)

