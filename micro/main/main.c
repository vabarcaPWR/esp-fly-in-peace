#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "ble_nus.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "led_indicator.h"
#include "lk8ex1.h"
#include <esp_log.h>

#define LK8EX1_TX_PERIOD_MS 125U
#define LK8EX1_TX_TASK_STACK_SIZE 4096U
#define LK8EX1_TX_TASK_PRIORITY 3U
#define LK8EX1_PROFILE_COMMAND_MAX_LEN 64U

#ifndef BLE_COMPAT_DEVICE_NAME
#define BLE_COMPAT_DEVICE_NAME "FlyInPeace"
#endif

#ifndef LK8EX1_SIM_PROFILE_DEFAULT
#define LK8EX1_SIM_PROFILE_DEFAULT LK8EX1_SIM_PROFILE_NOMINAL
#endif

static const char *TAG = "main";

typedef enum lk8ex1_sim_profile_e
{
    LK8EX1_SIM_PROFILE_NOMINAL = 0,
    LK8EX1_SIM_PROFILE_CLIMB,
    LK8EX1_SIM_PROFILE_SINK,
    LK8EX1_SIM_PROFILE_EDGE,
    LK8EX1_SIM_PROFILE_MALFORMED_CHECKSUM,
    LK8EX1_SIM_PROFILE_MALFORMED_SHAPE,
} lk8ex1_sim_profile_e;

typedef struct lk8ex1_simulation_state_s
{
    lk8ex1_sim_profile_e profile;
    uint32_t frame_index;
} lk8ex1_simulation_state_t;

typedef struct application_threads_s
{
    TaskHandle_t lk8ex1_sender_task;
} application_threads_t;

static QueueHandle_t lk8ex1_profile_queue = NULL;

static const char *lk8ex1_profile_to_name(lk8ex1_sim_profile_e profile)
{
    switch (profile)
    {
    case LK8EX1_SIM_PROFILE_NOMINAL:
        return "nominal";
    case LK8EX1_SIM_PROFILE_CLIMB:
        return "climb";
    case LK8EX1_SIM_PROFILE_SINK:
        return "sink";
    case LK8EX1_SIM_PROFILE_EDGE:
        return "edge";
    case LK8EX1_SIM_PROFILE_MALFORMED_CHECKSUM:
        return "malformed-checksum";
    case LK8EX1_SIM_PROFILE_MALFORMED_SHAPE:
        return "malformed-shape";
    default:
        return "unknown";
    }
}

static int32_t lk8ex1_battery_percent_for_frame(uint32_t frame_index)
{
    int32_t battery = 96 - (int32_t)(frame_index / 240U);
    if (battery < 15)
        battery = 15;

    return battery;
}

static void lk8ex1_build_nominal_data(uint32_t frame_index, lk8ex1_data_t *data)
{
    static const int32_t pressure_offsets[8] = {0, -3, -2, -1, 0, 1, 2, 1};
    static const int32_t altitude_offsets[8] = {0, 1, 1, 0, 0, -1, -1, 0};
    static const int32_t vario_values[8] = {5, 8, 3, 0, -2, -4, -1, 2};
    static const int32_t temperature_offsets[8] = {0, 1, 1, 0, 0, -1, -1, 0};
    uint32_t sample = frame_index % 8U;

    data->pressure_pa = 100900 + pressure_offsets[sample];
    data->altitude_m = 1035 + altitude_offsets[sample];
    data->vario_cms = vario_values[sample];
    data->temperature_dc = 235 + temperature_offsets[sample];
    data->battery_mv = lk8ex1_battery_percent_for_frame(frame_index);
}

static void lk8ex1_build_climb_data(uint32_t frame_index, lk8ex1_data_t *data)
{
    uint32_t trend_sample = frame_index % 320U;
    int32_t altitude = 920 + (int32_t)(trend_sample / 2U);
    int32_t pressure = 101325 - (altitude * 12);

    data->pressure_pa = pressure;
    data->altitude_m = altitude;
    data->vario_cms = 180 + (int32_t)(frame_index % 6U);
    data->temperature_dc = 228;
    data->battery_mv = lk8ex1_battery_percent_for_frame(frame_index);
}

static void lk8ex1_build_sink_data(uint32_t frame_index, lk8ex1_data_t *data)
{
    uint32_t trend_sample = frame_index % 320U;
    int32_t altitude = 1260 - (int32_t)(trend_sample / 2U);
    int32_t pressure = 101325 - (altitude * 12);

    data->pressure_pa = pressure;
    data->altitude_m = altitude;
    data->vario_cms = -180 - (int32_t)(frame_index % 6U);
    data->temperature_dc = 224;
    data->battery_mv = lk8ex1_battery_percent_for_frame(frame_index);
}

static void lk8ex1_build_edge_data(uint32_t frame_index, lk8ex1_data_t *data)
{
    uint32_t sample = frame_index % 6U;

    if (0U == sample)
    {
        data->pressure_pa = 100840;
        data->altitude_m = 99999;
        data->vario_cms = 0;
        data->temperature_dc = 230;
        data->battery_mv = 93;
        return;
    }

    if (1U == sample)
    {
        data->pressure_pa = 100860;
        data->altitude_m = 1010;
        data->vario_cms = -5;
        data->temperature_dc = 232;
        data->battery_mv = 999;
        return;
    }

    if (2U == sample)
    {
        data->pressure_pa = 30000;
        data->altitude_m = -500;
        data->vario_cms = 2500;
        data->temperature_dc = -200;
        data->battery_mv = 100;
        return;
    }

    if (3U == sample)
    {
        data->pressure_pa = 120000;
        data->altitude_m = 9000;
        data->vario_cms = -2500;
        data->temperature_dc = 600;
        data->battery_mv = 5;
        return;
    }

    if (4U == sample)
    {
        data->pressure_pa = 101325;
        data->altitude_m = 0;
        data->vario_cms = 0;
        data->temperature_dc = -400;
        data->battery_mv = 50;
        return;
    }

    data->pressure_pa = 101325;
    data->altitude_m = 0;
    data->vario_cms = 0;
    data->temperature_dc = 850;
    data->battery_mv = 50;
}

static bool lk8ex1_parse_profile_from_command(const uint8_t *data, uint16_t len, lk8ex1_sim_profile_e *profile)
{
    if (!data || !profile || (0U == len))
        return false;

    size_t copy_len = len;
    if (copy_len > (LK8EX1_PROFILE_COMMAND_MAX_LEN - 1U))
        copy_len = LK8EX1_PROFILE_COMMAND_MAX_LEN - 1U;

    char command[LK8EX1_PROFILE_COMMAND_MAX_LEN];
    memset(command, 0, sizeof(command));
    memcpy(command, data, copy_len);

    for (size_t i = 0; i < copy_len; i++)
    {
        command[i] = (char)toupper((unsigned char)command[i]);
    }
    if (strstr(command, "NOMINAL"))

    {
        *profile = LK8EX1_SIM_PROFILE_NOMINAL;
        return true;
    }

    if (strstr(command, "CLIMB"))
    {
        *profile = LK8EX1_SIM_PROFILE_CLIMB;
        return true;
    }

    if (strstr(command, "SINK"))
    {
        *profile = LK8EX1_SIM_PROFILE_SINK;
        return true;
    }

    if (strstr(command, "EDGE"))
    {
        *profile = LK8EX1_SIM_PROFILE_EDGE;
        return true;
    }

    if (strstr(command, "CHECKSUM"))
    {
        *profile = LK8EX1_SIM_PROFILE_MALFORMED_CHECKSUM;
        return true;
    }

    if (strstr(command, "SHAPE"))
    {
        *profile = LK8EX1_SIM_PROFILE_MALFORMED_SHAPE;
        return true;
    }

    return false;
}

static bool lk8ex1_corrupt_sentence_checksum(char *sentence)
{
    if (!sentence)
        return false;

    char *asterisk = strchr(sentence, '*');
    if (!asterisk || ('\0' == asterisk[1]))
        return false;

    asterisk[1] = ('A' == asterisk[1]) ? 'B' : 'A';
    return true;
}

static bool lk8ex1_build_malformed_shape_sentence(char *sentence, size_t sentence_size)
{
    if (!sentence || (0U == sentence_size))
        return false;

    int32_t written = snprintf(sentence, sentence_size, "$LK8EX1,101325,1000,120*00\r\n");
    return (written > 0) && ((size_t)written < sentence_size);
}

static bool lk8ex1_build_profile_sentence(const lk8ex1_simulation_state_t *state, char *sentence, size_t sentence_size)
{
    if (!state || !sentence)
        return false;

    if (LK8EX1_SIM_PROFILE_MALFORMED_SHAPE == state->profile)
    {
        return lk8ex1_build_malformed_shape_sentence(sentence, sentence_size);
    }

    lk8ex1_data_t lk8ex1_data = {
        .pressure_pa = 0,
        .altitude_m = 0,
        .vario_cms = 0,
        .temperature_dc = 0,
        .battery_mv = 0,
    };

    if (LK8EX1_SIM_PROFILE_NOMINAL == state->profile)
        lk8ex1_build_nominal_data(state->frame_index, &lk8ex1_data);
    else if (LK8EX1_SIM_PROFILE_CLIMB == state->profile)
        lk8ex1_build_climb_data(state->frame_index, &lk8ex1_data);
    else if (LK8EX1_SIM_PROFILE_SINK == state->profile)
        lk8ex1_build_sink_data(state->frame_index, &lk8ex1_data);
    else if (LK8EX1_SIM_PROFILE_EDGE == state->profile)
        lk8ex1_build_edge_data(state->frame_index, &lk8ex1_data);
    else if (LK8EX1_SIM_PROFILE_MALFORMED_CHECKSUM == state->profile)
        lk8ex1_build_nominal_data(state->frame_index, &lk8ex1_data);
    else
        return false;

    if (ESP_OK != lk8ex1_format(&lk8ex1_data, sentence, sentence_size))
        return false;

    if (LK8EX1_SIM_PROFILE_MALFORMED_CHECKSUM == state->profile)
        return lk8ex1_corrupt_sentence_checksum(sentence);

    return true;
}

static void lk8ex1_apply_profile_update_if_requested(lk8ex1_simulation_state_t *state)
{
    if (!state || !lk8ex1_profile_queue)
        return;

    lk8ex1_sim_profile_e next_profile = LK8EX1_SIM_PROFILE_NOMINAL;
    if (pdTRUE != xQueueReceive(lk8ex1_profile_queue, &next_profile, 0))
        return;

    if (next_profile == state->profile)
        return;

    state->profile = next_profile;
    state->frame_index = 0;
    ESP_LOGI(TAG, "LK8EX1 profile switched: %s", lk8ex1_profile_to_name(next_profile));
}

static void lk8ex1_advance_state(lk8ex1_simulation_state_t *state)
{
    if (!state)
        return;

    state->frame_index++;
}

static esp_err_t lk8ex1_create_profile_queue(void)
{
    lk8ex1_profile_queue = xQueueCreate(1U, sizeof(lk8ex1_sim_profile_e));
    if (!lk8ex1_profile_queue)
        return ESP_ERR_NO_MEM;

    lk8ex1_sim_profile_e default_profile = LK8EX1_SIM_PROFILE_DEFAULT;
    if (pdTRUE != xQueueOverwrite(lk8ex1_profile_queue, &default_profile))
        return ESP_FAIL;

    ESP_LOGI(TAG, "LK8EX1 profile default: %s", lk8ex1_profile_to_name(default_profile));
    return ESP_OK;
}

static void ble_rx_log_callback(const uint8_t *data, uint16_t len)
{
    if (!data)
        return;

    ESP_LOGI(TAG, "BLE RX: len=%u", len);

    lk8ex1_sim_profile_e next_profile = LK8EX1_SIM_PROFILE_NOMINAL;
    if (!lk8ex1_parse_profile_from_command(data, len, &next_profile))
        return;

    if (!lk8ex1_profile_queue)
    {
        ESP_LOGW(TAG, "Profile queue not initialized");
        return;
    }

    if (pdTRUE != xQueueOverwrite(lk8ex1_profile_queue, &next_profile))
    {
        ESP_LOGW(TAG, "Failed to queue profile change: %s", lk8ex1_profile_to_name(next_profile));
        return;
    }

    ESP_LOGI(TAG, "Queued LK8EX1 profile change: %s", lk8ex1_profile_to_name(next_profile));
}

static void ble_led_state_callback(bool connected, uint16_t conn_handle)
{
    (void)conn_handle;

    led_state_e next_state = connected ? LED_STATE_BLE_CONNECTED : LED_STATE_BLE_DISCONNECTED;
    esp_err_t set_state_result = led_indicator_set_state(next_state);
    if (ESP_OK != set_state_result)
        ESP_LOGW(TAG, "led_indicator_set_state failed: err=0x%x", set_state_result);
}

static void lk8ex1_simulated_sender_task(void *param)
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
        lk8ex1_apply_profile_update_if_requested(&simulation_state);

        if (lk8ex1_build_profile_sentence(&simulation_state, sentence, sizeof(sentence)))
        {
            esp_err_t send_result = ble_nus_send((const uint8_t *)sentence, (uint16_t)strlen(sentence));
            if (ESP_OK != send_result && ESP_ERR_INVALID_STATE != send_result)
            {
                ESP_LOGW(TAG, "ble_nus_send failed: err=0x%x", send_result);
            }
        }
        else
        {
            ESP_LOGW(TAG, "Failed to build simulated LK8EX1 sentence: profile=%s",
                     lk8ex1_profile_to_name(simulation_state.profile));
        }

        lk8ex1_advance_state(&simulation_state);

        vTaskDelayUntil(&last_wake_tick, pdMS_TO_TICKS(LK8EX1_TX_PERIOD_MS));
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
    esp_err_t queue_result = lk8ex1_create_profile_queue();
    if (ESP_OK != queue_result)
        return queue_result;

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
