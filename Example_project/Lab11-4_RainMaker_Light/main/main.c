#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_rmaker_core.h"
#include "esp_rmaker_schedule.h"
#include "esp_rmaker_standard_devices.h"
#include "esp_rmaker_standard_params.h"
#include "esp_rmaker_standard_types.h"
#include "app_network.h"
#include "nvs_flash.h"

#if CONFIG_IDF_TARGET_ESP32C3
#define LED_GPIO GPIO_NUM_8
#else
#define LED_GPIO GPIO_NUM_2
#endif

#define LED_PWM_TIMER LEDC_TIMER_0
#define LED_PWM_CHANNEL LEDC_CHANNEL_0
#define LED_PWM_RESOLUTION LEDC_TIMER_10_BIT
#define LED_PWM_MAX_DUTY ((1U << 10) - 1)

static const char *TAG = "RMAKER_LIGHT";
static bool s_power;
static uint8_t s_brightness = 100;

static esp_err_t update_led(void)
{
    uint32_t duty = s_power ? (s_brightness * LED_PWM_MAX_DUTY) / 100 : 0;
    esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_PWM_CHANNEL, duty);
    if (err != ESP_OK) {
        return err;
    }
    return ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_PWM_CHANNEL);
}

static esp_err_t init_led(void)
{
    const ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LED_PWM_RESOLUTION,
        .timer_num = LED_PWM_TIMER,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    esp_err_t err = ledc_timer_config(&timer_config);
    if (err != ESP_OK) {
        return err;
    }

    const ledc_channel_config_t channel_config = {
        .gpio_num = LED_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LED_PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LED_PWM_TIMER,
        .duty = 0,
        .hpoint = 0,
    };
    return ledc_channel_config(&channel_config);
}

static esp_err_t write_cb(const esp_rmaker_device_t *device,
                          const esp_rmaker_param_t *param,
                          const esp_rmaker_param_val_t val,
                          void *priv_data,
                          esp_rmaker_write_ctx_t *ctx)
{
    (void)priv_data;
    if (ctx) {
        ESP_LOGI(TAG, "Request received via: %s",
                 esp_rmaker_device_cb_src_to_str(ctx->src));
    }

    const char *param_name = esp_rmaker_param_get_name(param);
    ESP_LOGI(TAG, "Device: %s | Param: %s",
             esp_rmaker_device_get_name(device), param_name);

    if (strcmp(param_name, ESP_RMAKER_DEF_POWER_NAME) == 0) {
        s_power = val.val.b;
        ESP_LOGI(TAG, "Power: %s", s_power ? "ON" : "OFF");
    } else if (strcmp(param_name, ESP_RMAKER_DEF_BRIGHTNESS_NAME) == 0) {
        if (val.val.i < 0 || val.val.i > 100) {
            ESP_LOGE(TAG, "Brightness out of range: %d", val.val.i);
            return ESP_ERR_INVALID_ARG;
        }
        s_brightness = (uint8_t)val.val.i;
        ESP_LOGI(TAG, "Brightness: %u%%", (unsigned)s_brightness);
    } else {
        ESP_LOGE(TAG, "Unsupported parameter: %s", param_name);
        return ESP_ERR_NOT_SUPPORTED;
    }

    esp_err_t err = update_led();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to update LED: %s", esp_err_to_name(err));
        return err;
    }
    return esp_rmaker_param_update_and_report(param, val);
}

void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(init_led());

    app_network_init();

    const esp_rmaker_config_t rainmaker_config = {
        .enable_time_sync = true,
    };
    esp_rmaker_node_t *node =
        esp_rmaker_node_init(&rainmaker_config, "Smart Light Node", "Lightbulb");
    if (node == NULL) {
        ESP_LOGE(TAG, "Could not initialize RainMaker node");
        abort();
    }

    esp_rmaker_device_t *light_device =
        esp_rmaker_lightbulb_device_create("Living Room Light", NULL, s_power);
    if (light_device == NULL) {
        ESP_LOGE(TAG, "Could not create light device");
        abort();
    }
    ESP_ERROR_CHECK(esp_rmaker_device_add_cb(light_device, write_cb, NULL));
    esp_rmaker_device_add_param(
        light_device,
        esp_rmaker_brightness_param_create(ESP_RMAKER_DEF_BRIGHTNESS_NAME,
                                           s_brightness));
    ESP_ERROR_CHECK(esp_rmaker_node_add_device(node, light_device));

    esp_rmaker_timezone_service_enable();
    ESP_ERROR_CHECK(esp_rmaker_schedule_enable());
    ESP_ERROR_CHECK(esp_rmaker_start());

    ESP_ERROR_CHECK(app_network_start(
        (app_network_pop_type_t)CONFIG_APP_POP_TYPE));
}
