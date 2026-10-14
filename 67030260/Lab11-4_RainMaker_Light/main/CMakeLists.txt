idf_component_register(
    SRCS "Lab11-4_RainMaker_Light.c"
    INCLUDE_DIRS "."
    REQUIRES esp_rainmaker rmaker_app_network nvs_flash esp_driver_gpio driver
)
