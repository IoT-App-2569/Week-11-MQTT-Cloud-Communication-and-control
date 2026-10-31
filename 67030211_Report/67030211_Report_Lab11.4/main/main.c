#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_rmaker_core.h"
#include "esp_rmaker_standard_types.h"
#include "esp_rmaker_standard_params.h"
#include "esp_rmaker_standard_devices.h"
#include "esp_rmaker_schedule.h"
#include "app_wifi.h"

#define LED_PIN GPIO_NUM_2
static const char *TAG = "RMAKER_LIGHT";
static esp_rmaker_device_t *s_light_device = NULL;

// Callback รับคำสั่งควบคุมจาก RainMaker Cloud
static esp_err_t write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
                          const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    const char *param_name = esp_rmaker_param_get_name(param);
    ESP_LOGI(TAG, "Param updated: %s", param_name);

    if (strcmp(param_name, ESP_RMAKER_DEF_POWER_NAME) == 0) {
        bool state = val.val.b;
        ESP_LOGI(TAG, "LED Power state: %s", state ? "ON" : "OFF");
        gpio_set_level(LED_PIN, state ? 1 : 0);
        esp_rmaker_param_update_and_report(param, val);
    } else if (strcmp(param_name, ESP_RMAKER_DEF_BRIGHTNESS_NAME) == 0) {
        int brightness = val.val.i;
        ESP_LOGI(TAG, "LED Brightness: %d%%", brightness);
        esp_rmaker_param_update_and_report(param, val);
    }
    return ESP_OK;
}

void app_main(void)
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    nvs_flash_init();

    // เริ่มต้นระบบ BLE Wi-Fi Provisioning
    app_wifi_init();

    // สร้าง Node
    esp_rmaker_config_t rainmaker_cfg = { .enable_time_sync = true };
    esp_rmaker_node_t *node = esp_rmaker_node_init(&rainmaker_cfg, "Smart Light Node", "Lightbulb");

    // สร้าง Device ประเภท Lightbulb พร้อมพารามิเตอร์ Power และ Brightness
    s_light_device = esp_rmaker_lightbulb_device_create("Living Room Light", NULL, false);
    esp_rmaker_device_add_cb(s_light_device, write_cb, NULL);

    esp_rmaker_param_t *brightness_param = esp_rmaker_brightness_param_create(ESP_RMAKER_DEF_BRIGHTNESS_NAME, 100);
    esp_rmaker_device_add_param(s_light_device, brightness_param);

    esp_rmaker_node_add_device(node, s_light_device);
    esp_rmaker_timezone_service_enable();
    esp_rmaker_schedule_enable();

    // เริ่มต้น RainMaker Core และเริ่มการ Provisioning ผ่าน BLE
    esp_rmaker_start();
    app_wifi_start(POP_TYPE_RANDOM);
}

