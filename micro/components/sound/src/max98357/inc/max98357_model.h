#ifndef MAX98357_MODEL_H
#define MAX98357_MODEL_H

#include "esp_err.h"
#include "max98357_types.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Initialize the synth state: compute sine lookup table and reset phase.
     */
    esp_err_t max98357_mdl_init(synth_state_t *state, uint32_t sample_rate);

    /**
     * Fill buffer with sine wave at given frequency and volume.
     * Phase accumulator maintains continuity across calls.
     */
    void max98357_mdl_fill_tone(synth_state_t *state, int16_t *buf, size_t samples, uint16_t freq_hz, uint8_t volume_pct);

    /**
     * Fill buffer with silence (all zeros).
     */
    void max98357_mdl_fill_silence(int16_t *buf, size_t samples);

#ifdef __cplusplus
}
#endif

#endif // MAX98357_MODEL_H
