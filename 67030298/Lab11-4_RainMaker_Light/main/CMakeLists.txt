idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_rainmaker nvs_flash driver esp_driver_gpio esp_driver_ledc espressif__rmaker_app_network)

