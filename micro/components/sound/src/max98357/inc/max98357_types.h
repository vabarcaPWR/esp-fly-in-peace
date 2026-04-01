#ifndef MAX98357_TYPES_H
#define MAX98357_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define SYNTH_SINE_TABLE_SIZE 256

    typedef struct synth_state_s
    {
        int16_t sine_table[SYNTH_SINE_TABLE_SIZE];
        uint32_t phase_accumulator;
        uint32_t sample_rate;
    } synth_state_t;

#ifdef __cplusplus
}
#endif

#endif // MAX98357_TYPES_H
