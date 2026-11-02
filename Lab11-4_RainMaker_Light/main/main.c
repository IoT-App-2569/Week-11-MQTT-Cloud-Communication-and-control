#include <stdbool.h>
#include <string.h>

#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_rmaker_core.h"
#include "esp_rmaker_standard_devices.h"
#include "esp_rmaker_standard_params.h"
#include "esp_rmaker_schedule.h"
#include "nvs_flash.h"
#include "app_network.h"

#if CONFIG_IDF_TARGET_ESP32C3
#define LED_PIN GPIO_NUM_8
#else
#define LED_PIN GPIO_NUM_2
#endif

#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_10_BIT
#define LEDC_MAX_DUTY ((1U << 10) - 1)

static const char *TAG = "RMAKER_LIGHT";
static esp_rmaker_device_t *s_light_device;
static bool s_power;
static int s_brightness = 100;

static void apply_led_state(void)
{
    uint32_t duty = s_power ? (uint32_t)s_brightness * LEDC_MAX_DUTY / 100U : 0U;
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}

static void init_led(void)
{
    const ledc_timer_config_t timer = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    const ledc_channel_config_t channel = {
        .gpio_num = LED_PIN,
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));
    ESP_ERROR_CHECK(ledc_channel_config(&channel));
}

static esp_err_t write_cb(const esp_rmaker_device_t *device,
        const esp_rmaker_param_t *param, const esp_rmaker_param_val_t val,
        void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    (void)priv_data;
    if (ctx) {
        ESP_LOGI(TAG, "Request received via: %s", esp_rmaker_device_cb_src_to_str(ctx->src));
    }

    const char *name = esp_rmaker_param_get_name(param);
    if (strcmp(name, ESP_RMAKER_DEF_POWER_NAME) == 0) {
        s_power = val.val.b;
        ESP_LOGI(TAG, "Power: %s", s_power ? "ON" : "OFF");
    } else if (strcmp(name, ESP_RMAKER_DEF_BRIGHTNESS_NAME) == 0) {
        if (val.val.i < 0 || val.val.i > 100) {
            ESP_LOGW(TAG, "Brightness out of range: %d", val.val.i);
            return ESP_ERR_INVALID_ARG;
        }
        s_brightness = val.val.i;
        ESP_LOGI(TAG, "Brightness: %d%%", s_brightness);
    } else {
        ESP_LOGW(TAG, "Ignoring unsupported parameter: %s", name);
        return ESP_OK;
    }

    apply_led_state();
    esp_rmaker_param_update_and_report(param, val);
    (void)device;
    return ESP_OK;
}

void app_main(void)
{
    init_led();

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Initialize networking before creating the RainMaker node.
    app_network_init();

    const esp_rmaker_config_t cfg = { .enable_time_sync = true };
    esp_rmaker_node_t *node = esp_rmaker_node_init(&cfg, "Smart Light Node", "Lightbulb");
    if (!node) {
        ESP_LOGE(TAG, "Could not initialize RainMaker node");
        return;
    }

    s_light_device = esp_rmaker_lightbulb_device_create("Living Room Light", NULL, false);
    if (!s_light_device) {
        ESP_LOGE(TAG, "Could not create light device");
        return;
    }
    ESP_ERROR_CHECK(esp_rmaker_device_add_cb(s_light_device, write_cb, NULL));

    esp_rmaker_param_t *brightness = esp_rmaker_brightness_param_create(
            ESP_RMAKER_DEF_BRIGHTNESS_NAME, s_brightness);
    if (!brightness) {
        ESP_LOGE(TAG, "Could not create brightness parameter");
        return;
    }
    ESP_ERROR_CHECK(esp_rmaker_device_add_param(s_light_device, brightness));
    ESP_ERROR_CHECK(esp_rmaker_node_add_device(node, s_light_device));

    ESP_ERROR_CHECK(esp_rmaker_timezone_service_enable());
    ESP_ERROR_CHECK(esp_rmaker_schedule_enable());
    ESP_ERROR_CHECK(esp_rmaker_start());
    // Starts BLE provisioning when unconfigured and connects to Wi-Fi otherwise.
    ESP_ERROR_CHECK(app_network_start(POP_TYPE_RANDOM));
}
