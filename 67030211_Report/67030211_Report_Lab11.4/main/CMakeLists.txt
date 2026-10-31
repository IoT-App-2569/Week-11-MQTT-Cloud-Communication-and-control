idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES esp_rmaker_core esp_rmaker_standard_types esp_rmaker_standard_params esp_rmaker_standard_devices esp_rmaker_schedule app_wifi nvs_flash driver
)

