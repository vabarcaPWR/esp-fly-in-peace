#ifndef LED_INDICATOR_MODEL_H
#define LED_INDICATOR_MODEL_H

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#include "led_indicator.h"

typedef struct led_pattern_s
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint16_t on_ticks;
    uint16_t off_ticks;
} led_pattern_t;

void led_indicator_model_reset(void);
bool led_indicator_model_is_initialized(void);
void led_indicator_model_set_initialized(bool initialized);
esp_err_t led_indicator_model_set_state(led_state_e state);
led_state_e led_indicator_model_get_state(void);
esp_err_t led_indicator_model_get_pattern(led_state_e state, led_pattern_t *pattern);
bool led_indicator_model_is_led_on(const led_pattern_t *pattern, uint32_t elapsed_ticks);
uint32_t led_indicator_model_get_ticks_until_transition(const led_pattern_t *pattern, uint32_t elapsed_ticks);

#endif
