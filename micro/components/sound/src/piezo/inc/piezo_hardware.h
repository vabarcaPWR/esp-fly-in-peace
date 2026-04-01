#ifndef PIEZO_HARDWARE_H
#define PIEZO_HARDWARE_H

#include "tone_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    esp_err_t piezo_hw_init(uint8_t gpio_num);
    esp_err_t piezo_hw_set_tone(uint16_t freq_hz, uint8_t duty_pct);
    void piezo_hw_mute(void);

#ifdef __cplusplus
}
#endif

#endif // PIEZO_HARDWARE_H
