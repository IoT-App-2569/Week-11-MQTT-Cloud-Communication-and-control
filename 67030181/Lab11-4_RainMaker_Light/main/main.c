/**
 * ==============================================================================
 * Course: Week 11 - MQTT Cloud Communication and Control
 * Lab 11.4: ESP RainMaker Smart Device & Cloud Integration
 * Student ID: 67030281
 * Description: Smart Lightbulb firmware using ESP RainMaker SDK.
 *              Supports BLE Wi-Fi Provisioning, Assisted Claiming,
 *              Cloud State Synchronization (Power and Brightness parameters),
 *              and automatic SNTP time synchronization.
 * ==============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <driver/gpio.h>

// ESP RainMaker Core Headers
#include <esp_rmaker_core.h>
#include <esp_rmaker_standard_types.h>
#include <esp_rmaker_standard_params.h>
#include <esp_rmaker_standard_devices.h>
#include <esp_rmaker_schedule.h>
#include <app_wifi.h>

// -----------------------------------------------------------------------------
// Pin Configuration (Compatible with ESP32-C3 and ESP32 Classic)
// -----------------------------------------------------------------------------
#if CONFIG_IDF_TARGET_ESP32C3
#define LED_PIN GPIO_NUM_8
#else
#define LED_PIN GPIO_NUM_2  // Onboard / External LED on ESP32
#endif

static const char *TAG = "LAB11_4_RMAKER";
static esp_rmaker_device_t *s_light_device = NULL;

/* ==============================================================================
 * 1. Write Callback ฟังก์ชันรับคำสั่งควบคุมจากคลาวด์ (RainMaker Cloud Backend)
 * ============================================================================== */
static esp_err_t write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
                          const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    if (ctx) {
        ESP_LOGI(TAG, "Command received via transport: %s", esp_rmaker_device_cb_src_to_str(ctx->src));
    }
    const char *device_name = esp_rmaker_device_get_name(device);
    const char *param_name = esp_rmaker_param_get_name(param);

    ESP_LOGI(TAG, "Device: [%s] | Target Parameter: [%s]", device_name, param_name);

    // กรณีที่ 1: ตรวจสอบคำสั่ง Power (เปิด / ปิด หลอดไฟ)
    if (strcmp(param_name, ESP_RMAKER_DEF_POWER_NAME) == 0) {
        bool state = val.val.b;
        ESP_LOGI(TAG, ">> Setting LED Hardware Output -> %s (GPIO %d = %d)", state ? "ON" : "OFF", LED_PIN, state ? 1 : 0);
        gpio_set_level(LED_PIN, state ? 1 : 0);

        // หัวใจสำคัญ: รายงานค่าสถานะปัจจุบันกลับสู่คลาวด์ทันที เพื่อให้แอปมือถือซิงก์ตรงกัน
        esp_rmaker_param_update_and_report(param, val);
    }
    // กรณีที่ 2: ตรวจสอบคำสั่ง Brightness (ปรับระดับความสว่าง 0 - 100%)
    else if (strcmp(param_name, ESP_RMAKER_DEF_BRIGHTNESS_NAME) == 0) {
        int brightness = val.val.i;
        ESP_LOGI(TAG, ">> Changing Brightness Level to: %d%%", brightness);

        // (ในโครงงานจริงสามารถนำค่า brightness 0-100 ไปปรับ Pulse Width ผ่าน ledc_set_duty)
        esp_rmaker_param_update_and_report(param, val);
    } else {
        ESP_LOGW(TAG, "Unknown parameter command received: %s", param_name);
    }

    return ESP_OK;
}

/* ==============================================================================
 * 2. ฟังก์ชันหลัก (app_main)
 * ============================================================================== */
void app_main(void)
{
    ESP_LOGI(TAG, "==========================================================");
    ESP_LOGI(TAG, "Starting Lab 11.4: ESP RainMaker Smart Light (ID: 67030281)");
    ESP_LOGI(TAG, "Target Chip: %s | Platform: ESP RainMaker Ecosystem", CONFIG_IDF_TARGET);
    ESP_LOGI(TAG, "==========================================================");

    // 1. กำหนดค่าเริ่มต้น GPIO สำหรับควบคุมหลอดไฟ LED
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);
    ESP_LOGI(TAG, "Initialized Actuator GPIO %d as OUTPUT (Initial: OFF)", LED_PIN);

    // 2. เริ่มต้นระบบจัดเก็บข้อมูล NVS Flash (จำเป็นสำหรับเก็บใบรับรอง Claiming และ Wi-Fi credentials)
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // 3. เริ่มต้นระบบ Wi-Fi Provisioning
    app_wifi_init();

    // 4. สร้างและเริ่มต้นโหนดในระบบ ESP RainMaker
    esp_rmaker_config_t rainmaker_cfg = {
        .enable_time_sync = true, // เปิดบริการซิงก์เวลาโลกผ่าน SNTP อัตโนมัติ
    };
    esp_rmaker_node_t *node = esp_rmaker_node_init(&rainmaker_cfg, "Smart Light Node 67030281", "Lightbulb");
    if (!node) {
        ESP_LOGE(TAG, "Failed to initialize ESP RainMaker node!");
        return;
    }
    ESP_LOGI(TAG, "Initialized RainMaker Node: 'Smart Light Node 67030281'");

    // 5. สร้าง Device ประเภทมาตรฐาน Lightbulb (หลอดไฟอัจฉริยะ)
    s_light_device = esp_rmaker_lightbulb_device_create("Living Room Light", NULL, false);
    // ผูก Callback เพื่อรอรับคำสั่งควบคุม
    esp_rmaker_device_add_cb(s_light_device, write_cb, NULL);

    // 6. เพิ่ม Parameter Brightness (ความสว่างเริ่มต้น 100%)
    esp_rmaker_param_t *brightness_param = esp_rmaker_brightness_param_create(ESP_RMAKER_DEF_BRIGHTNESS_NAME, 100);
    esp_rmaker_device_add_param(s_light_device, brightness_param);

    // 7. บรรจุ Device เข้าสู่ Node
    esp_rmaker_node_add_device(node, s_light_device);
    ESP_LOGI(TAG, "Added 'Living Room Light' Device with Power & Brightness Params.");

    // 8. เปิดใช้งานบริการระดับระบบ (System Services)
    esp_rmaker_timezone_service_enable(); // บริการโซนเวลา
    esp_rmaker_schedule_enable();         // บริการตั้งเวลาเปิด-ปิดอัตโนมัติ (Scheduler)
    ESP_LOGI(TAG, "Enabled RainMaker System Services: Timezone & Schedule Services.");

    // 9. เริ่มต้นการทำงานของ RainMaker Core Agent
    esp_rmaker_start();
    ESP_LOGI(TAG, "ESP RainMaker Core Engine started.");

    // 10. เริ่มต้นระบบค้นหาอุปกรณ์และรับการตั้งค่า Wi-Fi ผ่านบลูทูธ (BLE Wi-Fi Provisioning)
    // ใช้สุ่มรหัส Proof of Possession (POP) เพื่อความปลอดภัยในการจับคู่อุปกรณ์
    ESP_LOGI(TAG, "Starting BLE Provisioning with Random POP (Scanning QR Code)...");
    app_wifi_start(POP_TYPE_RANDOM);
}

