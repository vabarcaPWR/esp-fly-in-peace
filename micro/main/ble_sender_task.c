#include "ble_sender_task.h"

#include "ble_nus.h"
#include "flight_data.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lk8ex1.h"
#include <esp_log.h>
#include <string.h>

#define BLE_SENDER_PERIOD_MS 125U
#define LK8EX1_ALTITUDE_UNAVAILABLE 99999
#define LK8EX1_VARIO_UNAVAILABLE 0
#define LK8EX1_BATTERY_UNAVAILABLE 999

static const char *TAG = "ble_tx";

static flight_data_t read_flight_data(void)
{
    flight_data_t snapshot = {0};

    if (xSemaphoreTake(g_flight_data_mutex, pdMS_TO_TICKS(FLIGHT_DATA_MUTEX_TIMEOUT_MS)) == pdTRUE)
    {
        snapshot = g_flight_data;
        xSemaphoreGive(g_flight_data_mutex);
    }

    return snapshot;
}

static lk8ex1_data_t build_lk8ex1_from_flight_data(const flight_data_t *fd)
{
    lk8ex1_data_t lk = {
        .pressure_pa = fd->pressure_pa,
        .altitude_m = fd->sensor_valid ? (int32_t)fd->altitude_m : LK8EX1_ALTITUDE_UNAVAILABLE,
        .vario_cms = fd->sensor_valid ? (int32_t)(fd->vario_ms * 100.0f) : LK8EX1_VARIO_UNAVAILABLE,
        .temperature_dc = (int32_t)(fd->temperature_mc / 100),
        .battery_mv = LK8EX1_BATTERY_UNAVAILABLE,
    };
    return lk;
}

void ble_sender_task_fn(void *param)
{
    (void)param;

    TickType_t last_wake = xTaskGetTickCount();
    char sentence[LK8EX1_MAX_SENTENCE_LEN];

    while (true)
    {
        if (ble_nus_is_connected())
        {
            flight_data_t fd = read_flight_data();
            lk8ex1_data_t lk = build_lk8ex1_from_flight_data(&fd);

            if (lk8ex1_format(&lk, sentence, sizeof(sentence)) == ESP_OK)
            {
                esp_err_t ret = ble_nus_send((const uint8_t *)sentence, (uint16_t)strlen(sentence));
                if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
                    ESP_LOGW(TAG, "send failed: 0x%x", ret);
            }
        }

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(BLE_SENDER_PERIOD_MS));
    }
}
