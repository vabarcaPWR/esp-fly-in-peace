#include "sound.h"

#ifdef CONFIG_SOUND_PIEZO
#include "piezo.h"
#endif

#include <string.h>

const sound_generator_t *get_sound_generator(const char *name)
{
    if (!name)
        return NULL;

#ifdef CONFIG_SOUND_PIEZO
    if (!strcmp(name, "piezo"))
        return get_piezo_sound_generator();
#endif

    return NULL;
}
