#include "sound_task.h"
#include "flight_data.h"
#include "sound.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#define SOUND_TASK_PERIOD_MS 50U
#define SOUND_BACKEND_NAME "piezo"

static const char *TAG = "sound_task";

static void read_vario_data(float *vario_ms, float *altitude_m, bool *valid)
{
    *vario_ms = 0.0f;
    *altitude_m = 0.0f;
    *valid = false;

    if (xSemaphoreTake(g_flight_data_mutex, pdMS_TO_TICKS(FLIGHT_DATA_MUTEX_TIMEOUT_MS)) != pdTRUE)
        return;

    *vario_ms = g_flight_data.vario_ms;
    *altitude_m = g_flight_data.altitude_m;
    *valid = g_flight_data.sensor_valid;
    xSemaphoreGive(g_flight_data_mutex);
}

void sound_task_fn(void *param)
{
    (void)param;

    const sound_generator_t *gen = get_sound_generator(SOUND_BACKEND_NAME);
    if (!gen)
    {
        ESP_LOGW(TAG, "no sound backend, task exiting");
        vTaskDelete(NULL);
        return;
    }

    esp_err_t ret = gen->init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "sound init failed: 0x%x", ret);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "started with backend: %s", gen->get_name());

    TickType_t last_wake = xTaskGetTickCount();
    bool had_valid_data = false;

    for (;;)
    {
        float vario_ms = 0.0f;
        float altitude_m = 0.0f;
        bool valid = false;

        read_vario_data(&vario_ms, &altitude_m, &valid);

        if (!valid)
        {
            if (had_valid_data)
                gen->update(0.0);
            had_valid_data = false;
        }
        else
        {
            had_valid_data = true;
            double vario_cms = (double)(vario_ms * 100.0f);
            gen->update(vario_cms);
        }

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(SOUND_TASK_PERIOD_MS));
    }
}
