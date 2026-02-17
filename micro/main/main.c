#include <stdbool.h>
#include <string.h>

#include "ble_nus.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lk8ex1.h"
#include <esp_log.h>

#define LK8EX1_TX_PERIOD_MS 250U
#define LK8EX1_TX_TASK_STACK_SIZE 4096U
#define LK8EX1_TX_TASK_PRIORITY 3U

static const char *TAG = "main";

typedef struct application_threads_s
{
    TaskHandle_t lk8ex1_sender_task;
} application_threads_t;

static void ble_rx_log_callback(const uint8_t *data, uint16_t len)
{
    if (data == NULL)
    {
        return;
    }

    ESP_LOGI(TAG, "BLE RX: len=%u", len);
}

static bool build_simulated_lk8ex1_sentence(char *sentence, size_t sentence_size)
{
    if (sentence == NULL)
    {
        return false;
    }

    static int32_t simulated_altitude_m = 99999;

    lk8ex1_data_t lk8ex1_data = {
        .pressure_pa = 101325,
        .altitude_m = simulated_altitude_m,
        .vario_cms = 0,
        .temperature_dc = 230,
        .battery_mv = 999,
    };

    simulated_altitude_m++;
    if (simulated_altitude_m > 100009)
    {
        simulated_altitude_m = 99999;
    }

    return lk8ex1_format(&lk8ex1_data, sentence, sentence_size) == ESP_OK;
}

static void lk8ex1_simulated_sender_task(void *param)
{
    (void)param;

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    TickType_t last_wake_tick = xTaskGetTickCount();
    char sentence[LK8EX1_MAX_SENTENCE_LEN];

    while (true)
    {
        if (build_simulated_lk8ex1_sentence(sentence, sizeof(sentence)))
        {
            esp_err_t send_result = ble_nus_send((const uint8_t *)sentence, (uint16_t)strlen(sentence));
            if (send_result != ESP_OK && send_result != ESP_ERR_INVALID_STATE)
            {
                ESP_LOGW(TAG, "ble_nus_send failed: err=0x%x", send_result);
            }
        }
        else
        {
            ESP_LOGW(TAG, "Failed to format simulated LK8EX1 sentence");
        }

        vTaskDelayUntil(&last_wake_tick, pdMS_TO_TICKS(LK8EX1_TX_PERIOD_MS));
    }
}

static esp_err_t initialize_ble_nus_module(void)
{
    ble_nus_cfg_t ble_config = {
        .device_name = BLE_NUS_DEFAULT_DEVICE_NAME,
        .adv_interval_ms = BLE_NUS_DEFAULT_ADV_INTERVAL_MS,
    };

    return ble_nus_init(&ble_config);
}

static esp_err_t initialize_modules(void)
{
    return initialize_ble_nus_module();
}

static esp_err_t configure_ble_nus_module_usage(void)
{
    ble_nus_register_rx_callback(ble_rx_log_callback);
    return ESP_OK;
}

static esp_err_t configure_modules_usage(void)
{
    return configure_ble_nus_module_usage();
}

static esp_err_t create_lk8ex1_sender_thread(application_threads_t *threads)
{
    if (threads == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    BaseType_t task_created = xTaskCreate(lk8ex1_simulated_sender_task, "lk8ex1_tx", LK8EX1_TX_TASK_STACK_SIZE, NULL,
                                          LK8EX1_TX_TASK_PRIORITY, &threads->lk8ex1_sender_task);
    if (task_created != pdPASS)
    {
        return ESP_FAIL;
    }

    return ESP_OK;
}

static esp_err_t create_threads(application_threads_t *threads)
{
    return create_lk8ex1_sender_thread(threads);
}

static esp_err_t launch_lk8ex1_sender_thread(const application_threads_t *threads)
{
    if (threads == NULL || threads->lk8ex1_sender_task == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    xTaskNotifyGive(threads->lk8ex1_sender_task);
    return ESP_OK;
}

static esp_err_t launch_threads(const application_threads_t *threads)
{
    return launch_lk8ex1_sender_thread(threads);
}

void app_main(void)
{
    ESP_LOGI(TAG, "esp-fly-in-peace firmware starting");

    application_threads_t threads = {
        .lk8ex1_sender_task = NULL,
    };

    esp_err_t modules_result = initialize_modules();
    if (modules_result != ESP_OK)
    {
        ESP_LOGE(TAG, "initialize_modules failed: err=0x%x", modules_result);
        return;
    }

    esp_err_t modules_usage_result = configure_modules_usage();
    if (modules_usage_result != ESP_OK)
    {
        ESP_LOGE(TAG, "configure_modules_usage failed: err=0x%x", modules_usage_result);
        return;
    }

    esp_err_t create_threads_result = create_threads(&threads);
    if (create_threads_result != ESP_OK)
    {
        ESP_LOGE(TAG, "create_threads failed: err=0x%x", create_threads_result);
        return;
    }

    esp_err_t launch_threads_result = launch_threads(&threads);
    if (launch_threads_result != ESP_OK)
    {
        ESP_LOGE(TAG, "launch_threads failed: err=0x%x", launch_threads_result);
        return;
    }
}
