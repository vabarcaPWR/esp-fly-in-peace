#include "led_single/led_single.h"

#include <stdbool.h>
#include <string.h>

const led_t *get_led(const char *led_name)
{
    if (!led_name)
        return NULL;

    if (!strcmp(led_name, "single"))
        return get_single_led();

    return NULL;
}
