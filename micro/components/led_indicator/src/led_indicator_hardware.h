#ifndef LED_INDICATOR_HARDWARE_H
#define LED_INDICATOR_HARDWARE_H

#include <stdint.h>

#include "esp_err.h"

esp_err_t led_indicator_hardware_init(void);
esp_err_t led_indicator_hardware_deinit(void);
esp_err_t led_indicator_hardware_set_rgb(uint8_t red, uint8_t green, uint8_t blue);
esp_err_t led_indicator_hardware_off(void);

#endif
