#include "led_rgb.h"
#include "led_rgb_model.h"

#include "driver/rmt_tx.h"
#include "led_strip.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifndef CONFIG_LED_RGB_GPIO
#define CONFIG_LED_RGB_GPIO 8
#endif

#define LED_RGB_TASK_PRIORITY 1U
#define LED_RGB_TASK_STACK_SIZE_BYTES 2048U
#define LED_RGB_TASK_STACK_SIZE_WORDS (LED_RGB_TASK_STACK_SIZE_BYTES / sizeof(StackType_t))
#define LED_RGB_TASK_TICK_MS 5U

typedef struct led_rgb_context_s
{
    led_state_e active_state;
    bool initialized;
    led_strip_handle_t strip;
    StackType_t task_stack[LED_RGB_TASK_STACK_SIZE_WORDS];
    StaticTask_t task_control;
    TaskHandle_t task_handle;
    portMUX_TYPE state_lock;
} led_rgb_context_t;

static led_rgb_context_t s_self = {
    .active_state = LED_STATE_BOOT,
    .initialized = false,
    .strip = NULL,
    .task_handle = NULL,
    .state_lock = portMUX_INITIALIZER_UNLOCKED,
};

static void store_state(led_state_e state)
{
    taskENTER_CRITICAL(&s_self.state_lock);
    s_self.active_state = state;
    taskEXIT_CRITICAL(&s_self.state_lock);
}

static led_state_e load_state(void)
{
    taskENTER_CRITICAL(&s_self.state_lock);
    led_state_e state = s_self.active_state;
    taskEXIT_CRITICAL(&s_self.state_lock);
    return state;
}

static void set_color(const rgb_color_t *color)
{
    led_strip_set_pixel(s_self.strip, 0, color->r, color->g, color->b);
    led_strip_refresh(s_self.strip);
}

static void clear_pixel(void)
{
    led_strip_clear(s_self.strip);
}

static void led_rgb_task(void *param)
{
    (void)param;

    led_state_e current_state = load_state();
    led_rgb_pattern_t pattern = led_rgb_pattern_for_state(current_state);
    uint16_t tick_in_cycle = 0U;
    TickType_t last_wake = xTaskGetTickCount();

    while (true)
    {
        led_state_e new_state = load_state();
        if (new_state != current_state)
        {
            current_state = new_state;
            pattern = led_rgb_pattern_for_state(current_state);
            tick_in_cycle = 0U;
        }

        if (tick_in_cycle < pattern.on_ticks)
            set_color(&pattern.color);
        else
            clear_pixel();

        tick_in_cycle++;
        if (tick_in_cycle >= pattern.period_ticks)
            tick_in_cycle = 0U;

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(LED_RGB_TASK_TICK_MS));
    }
}

static esp_err_t led_rgb_init(void)
{
    if (s_self.initialized)
        return ESP_OK;

    led_strip_config_t strip_cfg = {
        .strip_gpio_num = CONFIG_LED_RGB_GPIO,
        .max_leds = 1,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };

    led_strip_rmt_config_t rmt_cfg = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false,
    };

    esp_err_t ret = led_strip_new_rmt_device(&strip_cfg, &rmt_cfg, &s_self.strip);
    if (ret != ESP_OK)
        return ret;

    led_strip_clear(s_self.strip);
    store_state(LED_STATE_BOOT);

    s_self.task_handle = xTaskCreateStatic(led_rgb_task, "led_rgb", LED_RGB_TASK_STACK_SIZE_WORDS, NULL,
                                           LED_RGB_TASK_PRIORITY, s_self.task_stack, &s_self.task_control);
    if (!s_self.task_handle)
        return ESP_FAIL;

    s_self.initialized = true;
    return ESP_OK;
}

static esp_err_t led_rgb_set_state(led_state_e state)
{
    if (state >= LED_STATE_COUNT)
        return ESP_ERR_INVALID_ARG;

    if (!s_self.initialized)
        return ESP_ERR_INVALID_STATE;

    store_state(state);
    return ESP_OK;
}

static led_state_e led_rgb_get_state(void)
{
    return load_state();
}

static const char *led_rgb_get_name(void)
{
    return "rgb";
}

const led_t *get_rgb_led(void)
{
    static const led_t instance = {
        .init = led_rgb_init,
        .set_state = led_rgb_set_state,
        .get_state = led_rgb_get_state,
        .get_name = led_rgb_get_name,
    };

    return &instance;
}
