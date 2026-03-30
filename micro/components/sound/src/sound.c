#include "sound.h"

#ifdef CONFIG_SENSOR_PIEZO
#include "piezo.h"
#endif

#include <string.h>

const sound_generator_t *get_sound_generator(const char *sensor_name)
{
    if (!sensor_name)
        return NULL;

#ifdef CONFIG_SENSOR_PIEZO
    if (!strcmp(sensor_name, "piezo"))
        return get_piezo_sound_generator();
#endif

    return NULL;
}
