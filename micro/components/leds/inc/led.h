#ifndef LED_H
#define LED_H

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct led_s
    {
        esp_err_t (*init)(void);
        const char *(*get_name)(void);
    } led_t;


    const led_t *get_led(const char *led_name);

#ifdef __cplusplus
}
#endif

#endif // LED_H
