#include <stdbool.h>
#include <string.h>

#ifdef CONFIG_LED_SINGLE
#include "led_single/led_single.h"
#endif

#ifdef CONFIG_LED_RGB
#include "led_rgb/led_rgb.h"
#endif

const led_t *get_led(const char *led_name)
{
    if (!led_name)
        return NULL;

#ifdef CONFIG_LED_SINGLE
    if (!strcmp(led_name, "single"))
        return get_single_led();
#endif

#ifdef CONFIG_LED_RGB
    if (!strcmp(led_name, "rgb"))
        return get_rgb_led();
#endif

    return NULL;
}
