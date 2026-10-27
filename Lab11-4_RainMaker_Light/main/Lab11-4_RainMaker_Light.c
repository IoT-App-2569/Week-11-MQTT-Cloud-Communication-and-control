#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

#include "esp_rmaker_core.h"
#include "esp_rmaker_standard_types.h"
#include "esp_rmaker_standard_params.h"
#include "esp_rmaker_standard_devices.h"
#include "esp_rmaker_standard_services.h"
#include "esp_rmaker_schedule.h"
#include "app_network.h"

#if CONFIG_IDF_TARGET_ESP32C3
#define LED_PIN GPIO_NUM_8
#else
#define LED_PIN GPIO_NUM_2
#endif

#define LEDC_MODE       LEDC_LOW_SPEED_MODE
#define LEDC_TIMER      LEDC_TIMER_0
#define LEDC_CHANNEL    LEDC_CHANNEL_0
#define LEDC_RES        LEDC_TIMER_10_BIT
#define LEDC_FREQ_HZ    5000

#define LEDC_MAX_DUTY   1023

static const char *TAG = "RMAKER_LIGHT";

static esp_rmaker_device_t *s_light_device = NULL;

static bool s_power = false;
static int s_brightness = 100;


/* Initialize LEDC PWM */
static void led_init(void)
{
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_RES,
        .timer_num = LEDC_TIMER,
        .freq_hz = LEDC_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    ledc_channel_config_t channel = {
        .gpio_num = LED_PIN,
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER,
        .duty = 0,
        .hpoint = 0,
    };

    ESP_ERROR_CHECK(ledc_channel_config(&channel));

    ESP_LOGI(TAG, "LEDC PWM initialized on GPIO %d", LED_PIN);
}


/* Apply power and brightness to LED */
static void led_apply(void)
{
    int brightness = s_brightness;

    /* Keep brightness within 0-100% */
    if (brightness < 0) {
        brightness = 0;
    } else if (brightness > 100) {
        brightness = 100;
    }

    uint32_t duty = 0;

    if (s_power) {
        duty = ((uint32_t)brightness * LEDC_MAX_DUTY) / 100;
    }

    ESP_ERROR_CHECK(ledc_set_duty(
        LEDC_MODE,
        LEDC_CHANNEL,
        duty
    ));

    ESP_ERROR_CHECK(ledc_update_duty(
        LEDC_MODE,
        LEDC_CHANNEL
    ));

    ESP_LOGI(
        TAG,
        "LED Power: %s | Brightness: %d%% | PWM Duty: %lu",
        s_power ? "ON" : "OFF",
        brightness,
        (unsigned long)duty
    );
}


/* Handle commands from ESP RainMaker */
static esp_err_t write_cb(
    const esp_rmaker_device_t *device,
    const esp_rmaker_param_t *param,
    const esp_rmaker_param_val_t val,
    void *priv_data,
    esp_rmaker_write_ctx_t *ctx)
{
    if (ctx) {
        ESP_LOGI(
            TAG,
            "Request via: %s",
            esp_rmaker_device_cb_src_to_str(ctx->src)
        );
    }

    const char *param_name = esp_rmaker_param_get_name(param);

    ESP_LOGI(
        TAG,
        "Device: %s | Param: %s",
        esp_rmaker_device_get_name(device),
        param_name
    );

    if (strcmp(param_name, ESP_RMAKER_DEF_POWER_NAME) == 0) {

        s_power = val.val.b;

        led_apply();

        esp_rmaker_param_update_and_report(param, val);

    } else if (
        strcmp(param_name, ESP_RMAKER_DEF_BRIGHTNESS_NAME) == 0
    ) {

        s_brightness = val.val.i;

        led_apply();

        ESP_LOGI(TAG, "Brightness: %d%%", s_brightness);

        esp_rmaker_param_update_and_report(param, val);
    }

    return ESP_OK;
}


void app_main(void)
{
    /* Initialize hardware PWM before accepting cloud commands */
    led_init();
    led_apply();

    /* Initialize NVS */
    esp_err_t err = nvs_flash_init();

    if (
        err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND
    ) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    ESP_ERROR_CHECK(err);

    /* Initialize network */
    app_network_init();

    /* Initialize RainMaker node */
    esp_rmaker_config_t cfg = {
        .enable_time_sync = true
    };

    esp_rmaker_node_t *node = esp_rmaker_node_init(
        &cfg,
        "Smart Light Node",
        "Lightbulb"
    );

    if (!node) {
        ESP_LOGE(TAG, "Could not initialise node.");
        return;
    }

    /* Create lightbulb device */
    s_light_device = esp_rmaker_lightbulb_device_create(
        "Living Room Light",
        NULL,
        false
    );

    if (!s_light_device) {
        ESP_LOGE(TAG, "Could not create light device.");
        return;
    }

    esp_rmaker_device_add_cb(
        s_light_device,
        write_cb,
        NULL
    );

    /* Add brightness parameter */
    esp_rmaker_param_t *brightness =
        esp_rmaker_brightness_param_create(
            ESP_RMAKER_DEF_BRIGHTNESS_NAME,
            100
        );

    if (!brightness) {
        ESP_LOGE(TAG, "Could not create brightness parameter.");
        return;
    }

    esp_rmaker_device_add_param(
        s_light_device,
        brightness
    );

    /* Register device with RainMaker node */
    esp_rmaker_node_add_device(
        node,
        s_light_device
    );

    /* Enable timezone and scheduling services */
    esp_rmaker_timezone_service_enable();
    esp_rmaker_schedule_enable();

    /* Start RainMaker and network */
    esp_rmaker_start();

    app_network_start(POP_TYPE_RANDOM);
}