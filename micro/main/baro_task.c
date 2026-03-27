#include "baro_task.h"

#include "flight_data.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sensor.h"
#include <esp_log.h>

#define BARO_TASK_PERIOD_MS 100U
#define BARO_MAX_CONSECUTIVE_ERRORS 3U

static const char *TAG = "baro_task";

void baro_task_fn(void *param)
{
    const sensor_baro_t *sensor = (const sensor_baro_t *)param;
    if (!sensor)
    {
        ESP_LOGE(TAG, "NULL sensor, deleting task");
        vTaskDelete(NULL);
        return;
    }

    TickType_t last_wake = xTaskGetTickCount();
    uint32_t error_count = 0;

    while (true)
    {
        data_baro_t reading = {0};
        esp_err_t ret = sensor->read(&reading);

        if (ret == ESP_OK)
        {
            error_count = 0;
            xQueueOverwrite(g_baro_queue, &reading);
        }
        else
        {
            error_count++;
            if (error_count >= BARO_MAX_CONSECUTIVE_ERRORS)
                ESP_LOGW(TAG, "baro read failed %lu consecutive times", (unsigned long)error_count);
        }

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(BARO_TASK_PERIOD_MS));
    }
}
