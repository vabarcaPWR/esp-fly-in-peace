#ifndef LED_RGB_MODEL_H
#define LED_RGB_MODEL_H

#include "led.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct rgb_color_s
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    } rgb_color_t;

    typedef struct led_rgb_pattern_s
    {
        rgb_color_t color;
        uint16_t on_ticks;
        uint16_t period_ticks;
    } led_rgb_pattern_t;

    /**
     * @brief Get the RGB color and blink pattern for a given LED state.
     */
    led_rgb_pattern_t led_rgb_pattern_for_state(led_state_e state);

#ifdef __cplusplus
}
#endif

#endif // LED_RGB_MODEL_H
