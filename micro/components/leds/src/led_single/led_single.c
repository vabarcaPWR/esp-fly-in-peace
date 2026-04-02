#include "led.h"

#include <stdbool.h>
#include <string.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#define LED_SINGLE_GPIO GPIO_NUM_8
#define LED_TASK_PRIORITY 1U
#define LED_TASK_STACK_SIZE_BYTES 2048U
#define LED_TASK_STACK_SIZE_WORDS (LED_TASK_STACK_SIZE_BYTES / sizeof(StackType_t))
#define LED_TASK_TICK_MS 5U
#define LED_STATE_QUEUE_LENGTH 1U
#define LED_SINGLE_ON_LEVEL 0
#define LED_SINGLE_OFF_LEVEL 1
#define LED_PATTERN_BOOT_PERIOD_TICKS 1U
#define LED_PATTERN_DISCONNECTED_PERIOD_TICKS 193U
#define LED_PATTERN_CONNECTED_PERIOD_TICKS 793U
#define LED_PATTERN_WIFI_PERIOD_TICKS 98U
#define LED_PATTERN_ERROR_PERIOD_TICKS 100U
#define LED_PATTERN_STANDARD_ON_TICKS 3U
#define LED_PATTERN_ERROR_ON_TICKS 50U

typedef struct led_single_context_s
{
    led_state_e active_state;
    bool initialized;
    QueueHandle_t state_queue;
    StaticQueue_t state_queue_control;
    uint8_t state_queue_storage[LED_STATE_QUEUE_LENGTH * sizeof(led_state_e)];
    StackType_t task_stack[LED_TASK_STACK_SIZE_WORDS];
    StaticTask_t task_control;
    TaskHandle_t task_handle;
    portMUX_TYPE state_lock;
} led_single_context_t;

static led_single_context_t s_self = {
    .active_state = LED_STATE_BOOT,
    .initialized = false,
    .state_queue = NULL,
    .task_handle = NULL,
    .state_lock = portMUX_INITIALIZER_UNLOCKED,
};

static bool led_state_is_valid(led_state_e state)
{
    return state >= LED_STATE_BOOT && state < LED_STATE_COUNT;
}

static void led_store_state(led_state_e state)
{
    taskENTER_CRITICAL(&s_self.state_lock);
    s_self.active_state = state;
    taskEXIT_CRITICAL(&s_self.state_lock);
}

static led_state_e led_load_state(void)
{
    taskENTER_CRITICAL(&s_self.state_lock);
    led_state_e state = s_self.active_state;
    taskEXIT_CRITICAL(&s_self.state_lock);
    return state;
}

static void led_pattern_for_state(led_state_e state, uint16_t *on_ticks, uint16_t *period_ticks)
{
    if (state == LED_STATE_BOOT)
    {
        *on_ticks = 1U;
        *period_ticks = LED_PATTERN_BOOT_PERIOD_TICKS;
    }
    else if (state == LED_STATE_BLE_DISCONNECTED)
    {
        *on_ticks = LED_PATTERN_STANDARD_ON_TICKS;
        *period_ticks = LED_PATTERN_DISCONNECTED_PERIOD_TICKS;
    }
    else if (state == LED_STATE_BLE_CONNECTED)
    {
        *on_ticks = LED_PATTERN_STANDARD_ON_TICKS;
        *period_ticks = LED_PATTERN_CONNECTED_PERIOD_TICKS;
    }
    else if (state == LED_STATE_WIFI_ENABLED)
    {
        *on_ticks = LED_PATTERN_STANDARD_ON_TICKS;
        *period_ticks = LED_PATTERN_WIFI_PERIOD_TICKS;
    }
    else
    {
        *on_ticks = LED_PATTERN_ERROR_ON_TICKS;
        *period_ticks = LED_PATTERN_ERROR_PERIOD_TICKS;
    }
}

static bool led_state_output_on(led_state_e state, uint16_t tick_in_cycle)
{
    uint16_t on_ticks = 1U;
    uint16_t period_ticks = 1U;
    led_pattern_for_state(state, &on_ticks, &period_ticks);
    return tick_in_cycle < on_ticks;
}

static uint16_t led_next_tick_in_cycle(led_state_e state, uint16_t current_tick)
{
    uint16_t on_ticks = 1U;
    uint16_t period_ticks = 1U;
    led_pattern_for_state(state, &on_ticks, &period_ticks);
    uint16_t next_tick = (uint16_t)(current_tick + 1U);
    if (next_tick >= period_ticks)
        return 0U;

    return next_tick;
}

static void led_single_task(void *param)
{
    (void)param;

    led_state_e active_state = led_load_state();
    uint16_t tick_in_cycle = 0U;
    TickType_t last_wake_tick = xTaskGetTickCount();

    while (true)
    {
        led_state_e requested_state = LED_STATE_COUNT;
        if (xQueueReceive(s_self.state_queue, &requested_state, 0U) == pdPASS)
        {
            active_state = requested_state;
            led_store_state(requested_state);
            tick_in_cycle = 0U;
        }

        gpio_set_level(LED_SINGLE_GPIO,
                       led_state_output_on(active_state, tick_in_cycle) ? LED_SINGLE_ON_LEVEL : LED_SINGLE_OFF_LEVEL);
        tick_in_cycle = led_next_tick_in_cycle(active_state, tick_in_cycle);
        vTaskDelayUntil(&last_wake_tick, pdMS_TO_TICKS(LED_TASK_TICK_MS));
    }
}

static esp_err_t led_single_init(void)
{
    if (s_self.initialized)
        return ESP_OK;

    gpio_config_t gpio_configuration = {
        .pin_bit_mask = (1ULL << LED_SINGLE_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    esp_err_t gpio_result = gpio_config(&gpio_configuration);
    if (gpio_result != ESP_OK)
        return gpio_result;

    s_self.state_queue = xQueueCreateStatic(LED_STATE_QUEUE_LENGTH, sizeof(led_state_e), s_self.state_queue_storage,
                                            &s_self.state_queue_control);
    if (!s_self.state_queue)
        return ESP_FAIL;

    led_state_e boot_state = LED_STATE_BOOT;
    if (xQueueOverwrite(s_self.state_queue, &boot_state) != pdPASS)
        return ESP_FAIL;

    led_store_state(boot_state);
    gpio_set_level(LED_SINGLE_GPIO, LED_SINGLE_ON_LEVEL);

    s_self.task_handle = xTaskCreateStatic(led_single_task, "led_task", LED_TASK_STACK_SIZE_WORDS, NULL,
                                           LED_TASK_PRIORITY, s_self.task_stack, &s_self.task_control);
    if (!s_self.task_handle)
        return ESP_FAIL;

    s_self.initialized = true;
    return ESP_OK;
}

static esp_err_t led_single_set_state(led_state_e state)
{
    if (!led_state_is_valid(state))
        return ESP_ERR_INVALID_ARG;

    if (!s_self.initialized || !s_self.state_queue)
        return ESP_ERR_INVALID_STATE;

    if (xQueueOverwrite(s_self.state_queue, &state) != pdPASS)
        return ESP_FAIL;

    return ESP_OK;
}

static led_state_e led_single_get_state(void)
{
    return led_load_state();
}

static const char *led_single_get_name(void)
{
    return "single";
}

const led_t *get_single_led(void)
{
    static const led_t led = {
        .init = led_single_init,
        .set_state = led_single_set_state,
        .get_state = led_single_get_state,
        .get_name = led_single_get_name,
    };

    return &led;
}
