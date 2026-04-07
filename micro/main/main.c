#include <stdbool.h>

#include "baro_task.h"
#include "ble_nus.h"
#include "ble_sender_task.h"
#include "flight_data.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "fusion_task.h"
#include "led.h"
#include "sensor.h"
#include "sound_task.h"
#include <esp_log.h>

#define BARO_TASK_STACK_SIZE 4096U
#define BARO_TASK_PRIORITY 5U
#define FUSION_TASK_STACK_SIZE 4096U
#define FUSION_TASK_PRIORITY 6U
#define BLE_SENDER_TASK_STACK_SIZE 4096U
#define BLE_SENDER_TASK_PRIORITY 3U
#define SOUND_TASK_STACK_SIZE 2048U
#define SOUND_TASK_PRIORITY 1U

#ifndef BLE_COMPAT_DEVICE_NAME
#define BLE_COMPAT_DEVICE_NAME "FlyInPeace"
#endif

static const char *TAG = "main";
typedef struct app_context_s
{
    const led_t *led;
    const sensor_baro_t *baro_sensor;
} app_context_t;

static app_context_t s_self;

typedef struct application_threads_s
{
    TaskHandle_t baro_task;
    TaskHandle_t fusion_task;
    TaskHandle_t ble_sender_task;
    TaskHandle_t sound_task;
} application_threads_t;

static bool startup_step_succeeded(const char *step_name, esp_err_t result)
{
    if (result == ESP_OK)
        return true;

    ESP_LOGE(TAG, "%s failed: err=0x%x", step_name, result);
    return false;
}

static void application_threads_reset(application_threads_t *threads)
{
    if (!threads)
        return;

    threads->baro_task = NULL;
    threads->fusion_task = NULL;
    threads->ble_sender_task = NULL;
    threads->sound_task = NULL;
}

static esp_err_t initialize_ble_nus_module(void)
{
    ble_nus_cfg_t ble_config = {
        .device_name = BLE_COMPAT_DEVICE_NAME,
        .adv_interval_ms = BLE_NUS_DEFAULT_ADV_INTERVAL_MS,
    };

    return ble_nus_init(&ble_config);
}

static esp_err_t initialize_led_module(void)
{
#ifdef CONFIG_LED_RGB
    s_self.led = get_led("rgb");
#else
    s_self.led = get_led("single");
#endif
    if (!s_self.led || !s_self.led->init || !s_self.led->set_state)
        return ESP_ERR_NOT_FOUND;

    return s_self.led->init();
}

static void ble_connection_led_state_handler(bool connected, uint16_t conn_handle)
{
    (void)conn_handle;

    if (!s_self.led || !s_self.led->set_state)
        return;

    led_state_e state = connected ? LED_STATE_BLE_CONNECTED : LED_STATE_BLE_DISCONNECTED;
    s_self.led->set_state(state);
}

static esp_err_t initialize_sensors(void)
{
#ifdef CONFIG_SENSOR_MS5611
    s_self.baro_sensor = get_baro_sensor("ms5611");
#endif
#ifdef CONFIG_SENSOR_BMP390
    s_self.baro_sensor = get_baro_sensor("bmp390");
#endif
    if (s_self.baro_sensor)
    {
        esp_err_t result = s_self.baro_sensor->init();
        if (result != ESP_OK)
        {
            ESP_LOGW(TAG, "Baro sensor init failed: 0x%x", result);
            s_self.baro_sensor = NULL;
        }
    }

    return ESP_OK;
}

static esp_err_t initialize_modules(void)
{
    esp_err_t led_result = initialize_led_module();
    if (led_result != ESP_OK)
        return led_result;

    esp_err_t i2c_result = sensor_i2c_bus_init();
    if (i2c_result != ESP_OK)
        return i2c_result;

    esp_err_t sensor_result = initialize_sensors();
    if (sensor_result != ESP_OK)
        return sensor_result;

    esp_err_t ble_result = initialize_ble_nus_module();
    if (ble_result != ESP_OK)
        return ble_result;

    return flight_data_init();
}

static esp_err_t configure_modules_usage(void)
{
    ble_nus_register_state_callback(ble_connection_led_state_handler);
    if (s_self.led && s_self.led->set_state)
        s_self.led->set_state(ble_nus_is_connected() ? LED_STATE_BLE_CONNECTED : LED_STATE_BLE_DISCONNECTED);

    return ESP_OK;
}

static esp_err_t create_baro_task(application_threads_t *threads)
{
    if (!threads || !s_self.baro_sensor)
        return ESP_OK;

    BaseType_t ok = xTaskCreate(baro_task_fn, "baro", BARO_TASK_STACK_SIZE, (void *)s_self.baro_sensor,
                                BARO_TASK_PRIORITY, &threads->baro_task);
    return ok == pdPASS ? ESP_OK : ESP_FAIL;
}

static esp_err_t create_fusion_task(application_threads_t *threads)
{
    if (!threads)
        return ESP_ERR_INVALID_ARG;

    BaseType_t ok = xTaskCreate(fusion_task_fn, "fusion", FUSION_TASK_STACK_SIZE, NULL, FUSION_TASK_PRIORITY,
                                &threads->fusion_task);
    return ok == pdPASS ? ESP_OK : ESP_FAIL;
}

static esp_err_t create_ble_sender_task(application_threads_t *threads)
{
    if (!threads)
        return ESP_ERR_INVALID_ARG;

    BaseType_t ok = xTaskCreate(ble_sender_task_fn, "ble_tx", BLE_SENDER_TASK_STACK_SIZE, NULL,
                                BLE_SENDER_TASK_PRIORITY, &threads->ble_sender_task);
    return ok == pdPASS ? ESP_OK : ESP_FAIL;
}

static esp_err_t create_sound_task(application_threads_t *threads)
{
    if (!threads)
        return ESP_ERR_INVALID_ARG;

    BaseType_t ok =
        xTaskCreate(sound_task_fn, "sound", SOUND_TASK_STACK_SIZE, NULL, SOUND_TASK_PRIORITY, &threads->sound_task);
    return ok == pdPASS ? ESP_OK : ESP_FAIL;
}

static esp_err_t create_threads(application_threads_t *threads)
{
    esp_err_t ret = create_baro_task(threads);
    ret == ESP_OK ? ret = create_fusion_task(threads) : ret;
    ret == ESP_OK ? ret = create_ble_sender_task(threads) : ret;
    ret == ESP_OK ? ret = create_sound_task(threads) : ret;
    return ret;
}

void app_main(void)
{
    ESP_LOGI(TAG, BLE_COMPAT_DEVICE_NAME " firmware starting");

    application_threads_t threads;
    application_threads_reset(&threads);

    bool ret = startup_step_succeeded("initialize_modules", initialize_modules());
    ret ? ret = startup_step_succeeded("configure_modules_usage", configure_modules_usage()) : ret;
    ret ? ret = startup_step_succeeded("create_threads", create_threads(&threads)) : ret;

    if (ret)
        ESP_LOGI(TAG, "pipeline running: baro=%s", s_self.baro_sensor ? "yes" : "no");
}
