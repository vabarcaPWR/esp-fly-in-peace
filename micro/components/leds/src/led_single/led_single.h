#ifndef LED_SINGLE_H
#define LED_SINGLE_H

#include "led.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    const led_t *get_single_led(void);

#ifdef __cplusplus
}
#endif

#endif // LED_SINGLE_H
