#include "sound_task.h"
#include "flight_data.h"
#include "sound.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SOUND_TASK_PERIOD_MS 50U
#define SOUND_BACKEND_NAME "piezo"

static const char *TAG = "sound_task";

static void read_vario_data(float *vario_ms, float *altitude_m, bool *valid)
{
    *vario_ms = 0.0f;
    *altitude_m = 0.0f;
    *valid = false;

    flight_data_t snapshot;
    if (flight_data_read(&snapshot) != ESP_OK)
        return;

    *vario_ms = snapshot.vario_ms;
    *altitude_m = snapshot.altitude_m;
    *valid = snapshot.sensor_valid;
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

    if (gen->play_startup)
    {
        ret = gen->play_startup();
        if (ret != ESP_OK)
            ESP_LOGW(TAG, "startup sequence failed: 0x%x", ret);
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
