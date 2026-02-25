#include "led_indicator.h"

#include "led_indicator_conductor.h"

esp_err_t led_indicator_init(void)
{
    return led_indicator_conductor_init();
}

esp_err_t led_indicator_deinit(void)
{
    return led_indicator_conductor_deinit();
}

esp_err_t led_indicator_set_state(led_state_e state)
{
    return led_indicator_conductor_set_state(state);
}

led_state_e led_indicator_get_state(void)
{
    return led_indicator_conductor_get_state();
}
