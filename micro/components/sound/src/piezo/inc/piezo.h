#ifndef PIEZO_H
#define PIEZO_H

#include "piezo_types.h"
#include "sound.h"

#ifdef __cplusplus
extern "C"
{
#endif

    const sound_generator_t *get_piezo_sound_generator(void);

#ifdef __cplusplus
}
#endif

#endif // PIEZO_H
