#include "led_indicator_conductor.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "led_indicator_hardware.h"
#include "led_indicator_model.h"

#define LED_INDICATOR_TASK_STACK_SIZE 2048U
#define LED_INDICATOR_TASK_PRIORITY 1U
#define LED_INDICATOR_TICK_MS 100U

static const char *TAG = "led_conductor";

static QueueHandle_t s_state_queue = NULL;
static TaskHandle_t s_led_task = NULL;

static TickType_t led_indicator_pattern_ticks_to_wait(uint32_t pattern_ticks)
{
    if (0U == pattern_ticks)
        return portMAX_DELAY;

    TickType_t wait_ticks = pdMS_TO_TICKS(pattern_ticks * LED_INDICATOR_TICK_MS);
    if (0 == wait_ticks)
        return 1;

    return wait_ticks;
}

static void led_indicator_task(void *param)
{
    (void)param;

    led_state_e active_state = led_indicator_model_get_state();
    uint32_t elapsed_ticks = 0;
    bool led_output_initialized = false;
    bool led_output_on = false;
    uint8_t last_red = 0;
    uint8_t last_green = 0;
    uint8_t last_blue = 0;

    while (true)
    {
        led_pattern_t pattern;
        if (ESP_OK == led_indicator_model_get_pattern(active_state, &pattern))
        {
            bool should_turn_on = led_indicator_model_is_led_on(&pattern, elapsed_ticks);
            if (should_turn_on)
            {
                if (!led_output_initialized || !led_output_on || last_red != pattern.red ||
                    last_green != pattern.green || last_blue != pattern.blue)
                {
                    led_indicator_hardware_set_rgb(pattern.red, pattern.green, pattern.blue);
                    led_output_initialized = true;
                    led_output_on = true;
                    last_red = pattern.red;
                    last_green = pattern.green;
                    last_blue = pattern.blue;
                }
            }
            else
            {
                if (!led_output_initialized || led_output_on)
                {
                    led_indicator_hardware_off();
                    led_output_initialized = true;
                    led_output_on = false;
                }
            }

            uint32_t wait_pattern_ticks = led_indicator_model_get_ticks_until_transition(&pattern, elapsed_ticks);
            TickType_t wait_ticks = led_indicator_pattern_ticks_to_wait(wait_pattern_ticks);

            led_state_e pending_state;
            if (pdPASS == xQueueReceive(s_state_queue, &pending_state, wait_ticks))
            {
                if (ESP_OK == led_indicator_model_set_state(pending_state))
                {
                    active_state = pending_state;
                    elapsed_ticks = 0;
                    continue;
                }
            }

            if (portMAX_DELAY != wait_ticks)
                elapsed_ticks += wait_pattern_ticks;

            continue;
        }

        vTaskDelay(pdMS_TO_TICKS(LED_INDICATOR_TICK_MS));
    }
}

esp_err_t led_indicator_conductor_init(void)
{
    if (led_indicator_model_is_initialized())
        return ESP_ERR_INVALID_STATE;

    esp_err_t hardware_result = led_indicator_hardware_init();
    if (ESP_OK != hardware_result)
        return hardware_result;

    led_indicator_model_reset();

    s_state_queue = xQueueCreate(1, sizeof(led_state_e));
    if (!s_state_queue)
    {
        led_indicator_hardware_deinit();
        return ESP_ERR_NO_MEM;
    }

    BaseType_t create_result = xTaskCreate(led_indicator_task, "led_indicator", LED_INDICATOR_TASK_STACK_SIZE, NULL,
                                           LED_INDICATOR_TASK_PRIORITY, &s_led_task);
    if (pdPASS != create_result)
    {
        vQueueDelete(s_state_queue);
        s_state_queue = NULL;
        led_indicator_hardware_deinit();
        return ESP_ERR_NO_MEM;
    }

    led_indicator_model_set_initialized(true);

    led_state_e default_state = LED_STATE_BOOT;
    xQueueOverwrite(s_state_queue, &default_state);

    ESP_LOGI(TAG, "LED indicator initialized");
    return ESP_OK;
}

esp_err_t led_indicator_conductor_set_state(led_state_e state)
{
    if (!led_indicator_model_is_initialized())
        return ESP_ERR_INVALID_STATE;

    if (state < LED_STATE_BOOT || state > LED_STATE_ERROR)
        return ESP_ERR_INVALID_ARG;

    if (!s_state_queue)
        return ESP_ERR_INVALID_STATE;

    BaseType_t overwrite_result = xQueueOverwrite(s_state_queue, &state);
    if (pdPASS != overwrite_result)
        return ESP_FAIL;

    return ESP_OK;
}

led_state_e led_indicator_conductor_get_state(void)
{
    return led_indicator_model_get_state();
}
