#include "led_indicator_model.h"

static bool s_initialized = false;
static led_state_e s_current_state = LED_STATE_BOOT;

void led_indicator_model_reset(void)
{
    s_initialized = false;
    s_current_state = LED_STATE_BOOT;
}

bool led_indicator_model_is_initialized(void)
{
    return s_initialized;
}

void led_indicator_model_set_initialized(bool initialized)
{
    s_initialized = initialized;
}

esp_err_t led_indicator_model_set_state(led_state_e state)
{
    if (state < LED_STATE_BOOT || state > LED_STATE_ERROR)
        return ESP_ERR_INVALID_ARG;

    s_current_state = state;
    return ESP_OK;
}

led_state_e led_indicator_model_get_state(void)
{
    return s_current_state;
}

esp_err_t led_indicator_model_get_pattern(led_state_e state, led_pattern_t *pattern)
{
    if (!pattern)
        return ESP_ERR_INVALID_ARG;

    switch (state)
    {
    case LED_STATE_BOOT:
        pattern->red = 0;
        pattern->green = 0;
        pattern->blue = 25;
        pattern->on_ticks = 1;
        pattern->off_ticks = 0;
        return ESP_OK;
    case LED_STATE_BLE_DISCONNECTED:
        pattern->red = 25;
        pattern->green = 0;
        pattern->blue = 0;
        pattern->on_ticks = 1;
        pattern->off_ticks = 19;
        return ESP_OK;
    case LED_STATE_BLE_CONNECTED:
        pattern->red = 0;
        pattern->green = 25;
        pattern->blue = 0;
        pattern->on_ticks = 1;
        pattern->off_ticks = 49;
        return ESP_OK;
    case LED_STATE_WIFI_ENABLED:
        pattern->red = 0;
        pattern->green = 0;
        pattern->blue = 25;
        pattern->on_ticks = 1;
        pattern->off_ticks = 9;
        return ESP_OK;
    case LED_STATE_ERROR:
        pattern->red = 32;
        pattern->green = 0;
        pattern->blue = 0;
        pattern->on_ticks = 1;
        pattern->off_ticks = 1;
        return ESP_OK;
    default:
        return ESP_ERR_INVALID_ARG;
    }
}

bool led_indicator_model_is_led_on(const led_pattern_t *pattern, uint32_t elapsed_ticks)
{
    if (!pattern)
        return false;

    if (0U == pattern->off_ticks)
        return true;

    uint32_t cycle_ticks = (uint32_t)pattern->on_ticks + (uint32_t)pattern->off_ticks;
    if (0U == cycle_ticks)
        return false;

    uint32_t position_in_cycle = elapsed_ticks % cycle_ticks;
    return position_in_cycle < pattern->on_ticks;
}

uint32_t led_indicator_model_get_ticks_until_transition(const led_pattern_t *pattern, uint32_t elapsed_ticks)
{
    if (!pattern)
        return 1U;

    if (0U == pattern->off_ticks)
        return 0U;

    uint32_t cycle_ticks = (uint32_t)pattern->on_ticks + (uint32_t)pattern->off_ticks;
    if (0U == cycle_ticks)
        return 1U;

    uint32_t position_in_cycle = elapsed_ticks % cycle_ticks;
    bool led_on = position_in_cycle < pattern->on_ticks;
    if (led_on)
        return (uint32_t)pattern->on_ticks - position_in_cycle;

    return cycle_ticks - position_in_cycle;
}
