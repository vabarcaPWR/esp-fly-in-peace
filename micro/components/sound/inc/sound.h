#ifndef SOUND_H
#define SOUND_H

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct sound_generator_s
    {
        esp_err_t (*init)(void);
        esp_err_t (*update)(double vario_cms, double altitude_m);
        const char *(*get_name)(void);
    } sound_generator_t;

    const sound_generator_t *get_sound_generator(const char *sensor_name);

#ifdef __cplusplus
}
#endif

#endif // SOUND_H
