#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "ble_nus.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_indicator.h"
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
    if (!data)
        return;

    ESP_LOGI(TAG, "BLE RX: len=%u", len);
}

static void ble_led_state_callback(bool connected, uint16_t conn_handle)
{
    (void)conn_handle;

    led_state_e next_state = connected ? LED_STATE_BLE_CONNECTED : LED_STATE_BLE_DISCONNECTED;
    esp_err_t set_state_result = led_indicator_set_state(next_state);
    if (ESP_OK != set_state_result)
        ESP_LOGW(TAG, "led_indicator_set_state failed: err=0x%x", set_state_result);
}

static bool build_simulated_lk8ex1_sentence(char *sentence, size_t sentence_size)
{
    if (!sentence)
        return false;

    static uint32_t sample_counter = 0;
    static float simulated_altitude_m = 1020.0f;
    static float simulated_phase = 0.0f;
    static int32_t simulated_battery_percent = 96;

    float vertical_speed_ms = 1.8f * sinf(simulated_phase);
    simulated_altitude_m += vertical_speed_ms * 0.25f;

    simulated_phase += 0.12f;
    if (simulated_phase >= 6.2831853f)
    {
        simulated_phase -= 6.2831853f;
    }

    float pressure_ratio = 1.0f - (simulated_altitude_m / 44330.0f);
    int32_t simulated_pressure_pa = (int32_t)(101325.0f * powf(pressure_ratio, 5.255f));
    int32_t simulated_vario_cms = (int32_t)(vertical_speed_ms * 100.0f);
    int32_t simulated_temperature_dc = 235 + (int32_t)(8.0f * sinf(simulated_phase * 0.5f));

    if ((0U == (sample_counter % 240U)) && (simulated_battery_percent > 15))
    {
        simulated_battery_percent--;
    }
    sample_counter++;

    lk8ex1_data_t lk8ex1_data = {
        .pressure_pa = simulated_pressure_pa,
        .altitude_m = (int32_t)simulated_altitude_m,
        .vario_cms = simulated_vario_cms,
        .temperature_dc = simulated_temperature_dc,
        .battery_mv = simulated_battery_percent,
    };

    return ESP_OK == lk8ex1_format(&lk8ex1_data, sentence, sentence_size);
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
            if (ESP_OK != send_result && ESP_ERR_INVALID_STATE != send_result)
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
    esp_err_t led_result = led_indicator_init();
    if (ESP_OK != led_result)
        return led_result;

    esp_err_t ble_result = initialize_ble_nus_module();
    if (ESP_OK != ble_result)
    {
        led_indicator_set_state(LED_STATE_ERROR);
        return ble_result;
    }

    return ESP_OK;
}

static esp_err_t configure_ble_nus_module_usage(void)
{
    ble_nus_register_rx_callback(ble_rx_log_callback);
    ble_nus_register_state_callback(ble_led_state_callback);
    return ESP_OK;
}

static esp_err_t configure_modules_usage(void)
{
    esp_err_t ble_usage_result = configure_ble_nus_module_usage();
    if (ESP_OK != ble_usage_result)
        return ble_usage_result;

    return led_indicator_set_state(LED_STATE_BLE_DISCONNECTED);
}

static esp_err_t create_lk8ex1_sender_thread(application_threads_t *threads)
{
    if (!threads)
        return ESP_ERR_INVALID_ARG;

    BaseType_t task_created = xTaskCreate(lk8ex1_simulated_sender_task, "lk8ex1_tx", LK8EX1_TX_TASK_STACK_SIZE, NULL,
                                          LK8EX1_TX_TASK_PRIORITY, &threads->lk8ex1_sender_task);
    if (task_created != pdPASS)
        return ESP_FAIL;

    return ESP_OK;
}

static esp_err_t create_threads(application_threads_t *threads)
{
    return create_lk8ex1_sender_thread(threads);
}

static esp_err_t launch_lk8ex1_sender_thread(const application_threads_t *threads)
{
    if (!threads || !threads->lk8ex1_sender_task)
        return ESP_ERR_INVALID_ARG;

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
    if (ESP_OK != modules_result)
    {
        ESP_LOGE(TAG, "initialize_modules failed: err=0x%x", modules_result);
        return;
    }

    esp_err_t modules_usage_result = configure_modules_usage();
    if (ESP_OK != modules_usage_result)
    {
        ESP_LOGE(TAG, "configure_modules_usage failed: err=0x%x", modules_usage_result);
        return;
    }

    esp_err_t create_threads_result = create_threads(&threads);
    if (ESP_OK != create_threads_result)
    {
        ESP_LOGE(TAG, "create_threads failed: err=0x%x", create_threads_result);
        return;
    }

    esp_err_t launch_threads_result = launch_threads(&threads);
    if (ESP_OK != launch_threads_result)
    {
        ESP_LOGE(TAG, "launch_threads failed: err=0x%x", launch_threads_result);
        return;
    }
}
