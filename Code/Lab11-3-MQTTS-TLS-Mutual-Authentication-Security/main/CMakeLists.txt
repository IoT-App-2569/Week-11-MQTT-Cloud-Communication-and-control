idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES mqtt esp_wifi esp_event nvs_flash driver esp_driver_gpio
    EMBED_TXTFILES
        "certs/ca.crt"
        "certs/client.crt"
        "certs/client.key"
)

# อ่านค่า WIFI_SSID / WIFI_PASS / MQTTS_BROKER_URI จากไฟล์ ../.env แล้วส่งเป็น -D ให้คอมไพเลอร์
set(ENV_FILE "${CMAKE_CURRENT_LIST_DIR}/../.env")
if(EXISTS "${ENV_FILE}")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${ENV_FILE}")
    file(STRINGS "${ENV_FILE}" ENV_LINES)
    foreach(line IN LISTS ENV_LINES)
        if(line MATCHES "^[ \t]*([A-Za-z_][A-Za-z0-9_]*)[ \t]*=(.*)$")
            set(key "${CMAKE_MATCH_1}")
            set(val "${CMAKE_MATCH_2}")
            if(val MATCHES "^\"(.*)\"$")
                set(val "${CMAKE_MATCH_1}")
            endif()
            if(key STREQUAL "WIFI_SSID" OR key STREQUAL "WIFI_PASS" OR key STREQUAL "MQTTS_BROKER_URI")
                target_compile_definitions(${COMPONENT_LIB} PRIVATE "${key}=\"${val}\"")
            endif()
        endif()
    endforeach()
else()
    message(WARNING "Missing ${ENV_FILE} - copy .env.example to .env")
endif()
