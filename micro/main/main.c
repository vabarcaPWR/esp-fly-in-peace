#include <stdbool.h>

#include "ble_nus.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lk8ex1_simulation_runtime.h"
#include "sensor.h"
#include <esp_log.h>

#define SENSOR_READ_PERIOD_MS 100U
#define SENSOR_READ_TASK_STACK_SIZE 3072U
#define SENSOR_READ_TASK_PRIORITY 4U
#define LK8EX1_TX_TASK_STACK_SIZE 4096U
#define LK8EX1_TX_TASK_PRIORITY 3U

#ifndef BLE_COMPAT_DEVICE_NAME
#define BLE_COMPAT_DEVICE_NAME "FlyInPeace"
#endif

static const char *TAG = "main";

typedef struct application_threads_s
{
    TaskHandle_t lk8ex1_sender_task;
    TaskHandle_t sensor_read_task;
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

    threads->lk8ex1_sender_task = NULL;
    threads->sensor_read_task = NULL;
}

static void sensor_read_task_fn(void *param)
{
    (void)param;

    TickType_t last_wake_tick = xTaskGetTickCount();

    const baro_sensor_t *sensor = get_baro_sensor("ms5611");
    if (!sensor)
    {
        ESP_LOGW(TAG, "Barometric sensor not found");
        vTaskDelete(NULL);
        return;
    }

    if (sensor->init() != ESP_OK)
    {
        ESP_LOGW(TAG, "Barometric sensor init failed");
        vTaskDelete(NULL);
        return;
    }

    const char *sensor_name = sensor->get_name() ? sensor->get_name() : "unknown";

    while (true)
    {
        baro_data_t data = {0};
        esp_err_t ret = sensor->read(&data);
        if (ret == ESP_OK)
        {
            ESP_LOGI(TAG, "[%s] P=%ld Pa  T=%ld m°C", sensor_name, (long)data.pressure_pa, (long)data.temperature_mc);
        }
        else
        {
            ESP_LOGW(TAG, "sensor read error: 0x%x", ret);
        }
        vTaskDelayUntil(&last_wake_tick, pdMS_TO_TICKS(SENSOR_READ_PERIOD_MS));
    }
}

static esp_err_t initialize_ble_nus_module(void)
{
    ble_nus_cfg_t ble_config = {
        .device_name = BLE_COMPAT_DEVICE_NAME,
        .adv_interval_ms = BLE_NUS_DEFAULT_ADV_INTERVAL_MS,
    };

    return ble_nus_init(&ble_config);
}

static esp_err_t initialize_modules(void)
{
    esp_err_t ble_result = initialize_ble_nus_module();
    if (ble_result != ESP_OK)
        return ble_result;

    return lk8ex1_simulation_runtime_init();
}

static esp_err_t configure_modules_usage(void)
{
    ble_nus_register_rx_callback(lk8ex1_simulation_ble_rx_callback);
    return ESP_OK;
}

static esp_err_t create_lk8ex1_sender_thread(application_threads_t *threads)
{
    if (!threads)
        return ESP_ERR_INVALID_ARG;

    BaseType_t task_created = xTaskCreate(lk8ex1_simulation_sender_task, "lk8ex1_tx", LK8EX1_TX_TASK_STACK_SIZE, NULL,
                                          LK8EX1_TX_TASK_PRIORITY, &threads->lk8ex1_sender_task);
    if (task_created != pdPASS)
        return ESP_FAIL;

    return ESP_OK;
}

static esp_err_t create_sensor_read_thread(application_threads_t *threads)
{
    if (!threads)
        return ESP_ERR_INVALID_ARG;

    BaseType_t task_created = xTaskCreate(sensor_read_task_fn, "sensor_read", SENSOR_READ_TASK_STACK_SIZE, NULL,
                                          SENSOR_READ_TASK_PRIORITY, &threads->sensor_read_task);
    if (task_created != pdPASS)
        return ESP_FAIL;

    return ESP_OK;
}

static esp_err_t create_threads(application_threads_t *threads)
{
    esp_err_t sender_result = create_lk8ex1_sender_thread(threads);
    if (sender_result != ESP_OK)
        return sender_result;

    return create_sensor_read_thread(threads);
}

static esp_err_t launch_threads(const application_threads_t *threads)
{
    if (!threads || !threads->lk8ex1_sender_task)
        return ESP_ERR_INVALID_ARG;

    xTaskNotifyGive(threads->lk8ex1_sender_task);
    return ESP_OK;
}

void app_main(void)
{
    ESP_LOGI(TAG, BLE_COMPAT_DEVICE_NAME " firmware starting");

    application_threads_t threads;
    application_threads_reset(&threads);

    bool ret = startup_step_succeeded("initialize_modules", initialize_modules());
    ret ? ret = startup_step_succeeded("configure_modules_usage", configure_modules_usage()) : ret;
    ret ? ret = startup_step_succeeded("create_threads", create_threads(&threads)) : ret;
    ret ? ret = startup_step_succeeded("launch_threads", launch_threads(&threads)) : ret;
}
