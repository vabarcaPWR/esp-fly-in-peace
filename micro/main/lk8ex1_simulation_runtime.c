#include "lk8ex1_simulation_runtime.h"

#include <string.h>

#include "ble_nus.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "lk8ex1.h"
#include "lk8ex1_simulation_profile.h"
#include <esp_log.h>

#define LK8EX1_TX_PERIOD_MS 125U

#ifndef LK8EX1_SIM_PROFILE_DEFAULT
#define LK8EX1_SIM_PROFILE_DEFAULT LK8EX1_SIM_PROFILE_NOMINAL
#endif

static const char *TAG = "lk8ex1_sim";
typedef struct lk8ex1_sim_context_s
{
    QueueHandle_t profile_queue;
} lk8ex1_sim_context_t;

static lk8ex1_sim_context_t lk8ex1_sim_context;

static void lk8ex1_simulation_apply_profile_update_if_requested(lk8ex1_simulation_state_t *state)
{
    if (!state || !lk8ex1_sim_context.profile_queue)
    {
        return;
    }

    lk8ex1_sim_profile_e next_profile = LK8EX1_SIM_PROFILE_NOMINAL;
    if (xQueueReceive(lk8ex1_sim_context.profile_queue, &next_profile, 0) != pdTRUE)
    {
        return;
    }

    if (next_profile == state->profile)
    {
        return;
    }

    state->profile = next_profile;
    state->frame_index = 0;
    ESP_LOGI(TAG, "LK8EX1 profile switched: %s", lk8ex1_simulation_profile_to_name(next_profile));
}

esp_err_t lk8ex1_simulation_runtime_init(void)
{
    if (lk8ex1_sim_context.profile_queue)
    {
        return ESP_OK;
    }

    lk8ex1_sim_context.profile_queue = xQueueCreate(1U, sizeof(lk8ex1_sim_profile_e));
    if (!lk8ex1_sim_context.profile_queue)
    {
        return ESP_ERR_NO_MEM;
    }

    lk8ex1_sim_profile_e default_profile = LK8EX1_SIM_PROFILE_DEFAULT;
    if (xQueueOverwrite(lk8ex1_sim_context.profile_queue, &default_profile) != pdTRUE)
    {
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "LK8EX1 profile default: %s", lk8ex1_simulation_profile_to_name(default_profile));
    return ESP_OK;
}

void lk8ex1_simulation_ble_rx_callback(const uint8_t *data, uint16_t len)
{
    if (!data)
    {
        return;
    }

    ESP_LOGI(TAG, "BLE RX: len=%u", len);

    lk8ex1_sim_profile_e next_profile = LK8EX1_SIM_PROFILE_NOMINAL;
    if (!lk8ex1_simulation_parse_profile_command(data, len, &next_profile))
    {
        return;
    }

    if (!lk8ex1_sim_context.profile_queue)
    {
        ESP_LOGW(TAG, "Profile queue not initialized");
        return;
    }

    if (xQueueOverwrite(lk8ex1_sim_context.profile_queue, &next_profile) != pdTRUE)
    {
        ESP_LOGW(TAG, "Failed to queue profile change: %s", lk8ex1_simulation_profile_to_name(next_profile));
        return;
    }

    ESP_LOGI(TAG, "Queued LK8EX1 profile change: %s", lk8ex1_simulation_profile_to_name(next_profile));
}

void lk8ex1_simulation_sender_task(void *param)
{
    (void)param;

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    TickType_t last_wake_tick = xTaskGetTickCount();
    char sentence[LK8EX1_MAX_SENTENCE_LEN];
    lk8ex1_simulation_state_t simulation_state = {
        .profile = LK8EX1_SIM_PROFILE_DEFAULT,
        .frame_index = 0,
    };

    while (true)
    {
        lk8ex1_simulation_apply_profile_update_if_requested(&simulation_state);

        if (lk8ex1_simulation_build_profile_sentence(&simulation_state, sentence, sizeof(sentence)))
        {
            esp_err_t send_result = ble_nus_send((const uint8_t *)sentence, (uint16_t)strlen(sentence));
            if ((send_result != ESP_OK) && (send_result != ESP_ERR_INVALID_STATE))
            {
                ESP_LOGW(TAG, "ble_nus_send failed: err=0x%x", send_result);
            }
        }
        else
        {
            ESP_LOGW(TAG, "Failed to build simulated LK8EX1 sentence: profile=%s",
                     lk8ex1_simulation_profile_to_name(simulation_state.profile));
        }

        lk8ex1_simulation_advance_state(&simulation_state);
        vTaskDelayUntil(&last_wake_tick, pdMS_TO_TICKS(LK8EX1_TX_PERIOD_MS));
    }
}
