#include <stdbool.h>
#include <string.h>

#include "ble_nus.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lk8ex1.h"
#include <esp_log.h>

#define LK8EX1_TX_PERIOD_MS 250U

static const char *TAG = "main";

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

void app_main(void)
{
    ESP_LOGI(TAG, "esp-fly-in-peace firmware starting");

    ble_nus_cfg_t ble_config = {
        .device_name = BLE_NUS_DEFAULT_DEVICE_NAME,
        .adv_interval_ms = BLE_NUS_DEFAULT_ADV_INTERVAL_MS,
    };

    esp_err_t ble_result = ble_nus_init(&ble_config);
    if (ble_result != ESP_OK)
    {
        ESP_LOGE(TAG, "ble_nus_init failed: err=0x%x", ble_result);
        return;
    }

    ble_nus_register_rx_callback(ble_rx_log_callback);

    BaseType_t task_created = xTaskCreate(lk8ex1_simulated_sender_task, "lk8ex1_tx", 4096, NULL, 3, NULL);
    if (task_created != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create lk8ex1_simulated_sender_task");
    }
}
