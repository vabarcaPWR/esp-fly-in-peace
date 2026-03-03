#ifndef LED_INDICATOR_CONDUCTOR_H
#define LED_INDICATOR_CONDUCTOR_H

#include "esp_err.h"

#include "led_indicator.h"

esp_err_t led_indicator_conductor_init(void);
esp_err_t led_indicator_conductor_set_state(led_state_e state);
led_state_e led_indicator_conductor_get_state(void);

#endif
