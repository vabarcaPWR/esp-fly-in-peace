#ifndef MAX98357_HARDWARE_H
#define MAX98357_HARDWARE_H

#include "esp_err.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct max98357_gpio_cfg_s
    {
        uint8_t bclk;
        uint8_t ws;
        uint8_t dout;
        uint8_t sd;
    } max98357_gpio_cfg_t;

    esp_err_t max98357_hw_init(const max98357_gpio_cfg_t *cfg);
    esp_err_t max98357_hw_write(const int16_t *buf, size_t samples);
    void max98357_hw_mute(void);
    void max98357_hw_shutdown(bool enable);

#ifdef __cplusplus
}
#endif

#endif // MAX98357_HARDWARE_H
