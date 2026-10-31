
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

// ESP RainMaker Headers
#include "esp_rmaker_core.h"
#include "esp_rmaker_standard_types.h"
#include "esp_rmaker_standard_params.h"
#include "esp_rmaker_standard_devices.h"
#include "esp_rmaker_schedule.h"

// Network Provisioning
#include "app_network.h"

#if CONFIG_IDF_TARGET_ESP32C3
#define LED_PIN GPIO_NUM_8
#else
#define LED_PIN GPIO_NUM_2
#endif

static const char *TAG = "RMAKER_LIGHT";
static esp_rmaker_device_t *s_light_device = NULL;

/* ========================================================
 * 1. Write Callback: รับคำสั่งควบคุมจาก RainMaker Cloud
 * ======================================================== */
static esp_err_t write_cb(
    const esp_rmaker_device_t *device,
    const esp_rmaker_param_t *param,
    const esp_rmaker_param_val_t val,
    void *priv_data,
    esp_rmaker_write_ctx_t *ctx)
{
    if (ctx) {
        ESP_LOGI(TAG, "Request received via: %s",
                 esp_rmaker_device_cb_src_to_str(ctx->src));
    }

    const char *device_name = esp_rmaker_device_get_name(device);
    const char *param_name = esp_rmaker_param_get_name(param);

    ESP_LOGI(TAG, "Device: %s | Param: %s",
             device_name, param_name);

    // ควบคุมการเปิดและปิด LED
    if (strcmp(param_name, ESP_RMAKER_DEF_POWER_NAME) == 0) {
        bool state = val.val.b;

        ESP_LOGI(TAG, "Changing LED state to: %s",
                 state ? "ON" : "OFF");

        gpio_set_level(LED_PIN, state ? 1 : 0);

        // รายงานสถานะกลับไปยัง RainMaker Cloud
        esp_rmaker_param_update_and_report(param, val);
    }

    // รับค่าความสว่างจาก RainMaker
    else if (strcmp(param_name, ESP_RMAKER_DEF_BRIGHTNESS_NAME) == 0) {
        int brightness = val.val.i;

        ESP_LOGI(TAG, "Changing Brightness to: %d%%", brightness);

        // ส่วนนี้ยังไม่ได้ควบคุม PWM จริง
        esp_rmaker_param_update_and_report(param, val);
    }

    return ESP_OK;
}

/* ========================================================
 * 2. Main Application
 * ======================================================== */
void app_main(void)
{
    // ตั้งค่า GPIO สำหรับ LED
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);

    ESP_LOGI(TAG, "LED initialized on GPIO %d", (int)LED_PIN);

    // เริ่มต้น NVS Flash และตรวจสอบผลลัพธ์
    esp_err_t err = nvs_flash_init();

    ESP_LOGI(TAG, "Initial NVS result: %s", esp_err_to_name(err));

    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {

        ESP_LOGW(TAG, "NVS needs reinitialization");

        // ล้างเฉพาะ NVS partition เมื่อพบข้อผิดพลาดที่กำหนด
        err = nvs_flash_erase();
        ESP_ERROR_CHECK(err);

        err = nvs_flash_init();

        ESP_LOGI(TAG, "NVS reinitialization result: %s",
                 esp_err_to_name(err));
    }

    ESP_ERROR_CHECK(err);

    ESP_LOGI(TAG, "NVS initialized successfully");

    // 1. เริ่มต้นระบบ Network Provisioning
    app_network_init();

    // 2. สร้าง RainMaker Node
    esp_rmaker_config_t rainmaker_cfg = {
        .enable_time_sync = true,
    };

    esp_rmaker_node_t *node = esp_rmaker_node_init(
        &rainmaker_cfg,
        "Smart Light Node",
        "Lightbulb"
    );

    if (node == NULL) {
        ESP_LOGE(TAG, "Could not initialise RainMaker node");
        return;
    }

    ESP_LOGI(TAG, "RainMaker Node created successfully");

    // 3. สร้างอุปกรณ์ Lightbulb
    s_light_device = esp_rmaker_lightbulb_device_create(
        "Living Room Light",
        NULL,
        false
    );

    if (s_light_device == NULL) {
        ESP_LOGE(TAG, "Could not create light device");
        return;
    }

    // ผูก Callback สำหรับรับคำสั่งจาก Cloud
    esp_rmaker_device_add_cb(s_light_device, write_cb, NULL);

    // 4. เพิ่ม Parameter ความสว่าง เริ่มต้นที่ 100%
    esp_rmaker_param_t *brightness_param =
        esp_rmaker_brightness_param_create(
            ESP_RMAKER_DEF_BRIGHTNESS_NAME,
            100
        );

    if (brightness_param == NULL) {
        ESP_LOGE(TAG, "Could not create brightness parameter");
        return;
    }

    esp_rmaker_device_add_param(s_light_device, brightness_param);

    // 5. เพิ่มอุปกรณ์เข้า Node
    esp_rmaker_node_add_device(node, s_light_device);

    // 6. เปิดบริการ Timezone และ Schedule
    esp_rmaker_timezone_service_enable();
    esp_rmaker_schedule_enable();

    // 7. เริ่ม RainMaker Core
    ESP_LOGI(TAG, "Starting RainMaker Core");

    err = esp_rmaker_start();

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "RainMaker start failed: %s",
                 esp_err_to_name(err));
        return;
    }

    // 8. เริ่มระบบ Provisioning และเชื่อมต่อเครือข่าย
    ESP_LOGI(TAG, "Starting network provisioning");

    err = app_network_start(POP_TYPE_RANDOM);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Network start failed: %s",
                 esp_err_to_name(err));
        return;
    }

    ESP_LOGI(TAG, "RainMaker Smart Light started");
}
