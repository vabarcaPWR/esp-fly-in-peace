#include "led_rgb_model.h"

#define LED_RGB_BOOT_PERIOD_TICKS 1U
#define LED_RGB_DISCONNECTED_PERIOD_TICKS 193U
#define LED_RGB_CONNECTED_PERIOD_TICKS 793U
#define LED_RGB_WIFI_PERIOD_TICKS 98U
#define LED_RGB_ERROR_PERIOD_TICKS 100U
#define LED_RGB_STANDARD_ON_TICKS 3U
#define LED_RGB_ERROR_ON_TICKS 50U

led_rgb_pattern_t led_rgb_pattern_for_state(led_state_e state)
{
    switch (state)
    {
    case LED_STATE_BOOT:
        return (led_rgb_pattern_t){{0, 0, 40}, LED_RGB_BOOT_PERIOD_TICKS, LED_RGB_BOOT_PERIOD_TICKS};
    case LED_STATE_BLE_DISCONNECTED:
        return (led_rgb_pattern_t){{0, 0, 30}, LED_RGB_STANDARD_ON_TICKS, LED_RGB_DISCONNECTED_PERIOD_TICKS};
    case LED_STATE_BLE_CONNECTED:
        return (led_rgb_pattern_t){{0, 30, 0}, LED_RGB_STANDARD_ON_TICKS, LED_RGB_CONNECTED_PERIOD_TICKS};
    case LED_STATE_WIFI_ENABLED:
        return (led_rgb_pattern_t){{0, 30, 30}, LED_RGB_STANDARD_ON_TICKS, LED_RGB_WIFI_PERIOD_TICKS};
    case LED_STATE_ERROR:
        return (led_rgb_pattern_t){{50, 0, 0}, LED_RGB_ERROR_ON_TICKS, LED_RGB_ERROR_PERIOD_TICKS};
    default:
        return (led_rgb_pattern_t){{0, 0, 0}, 0U, 1U};
    }
}
