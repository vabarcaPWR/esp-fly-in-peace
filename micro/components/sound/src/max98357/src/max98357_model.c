#include "max98357_model.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define PHASE_BITS 32
#define TABLE_MASK (SYNTH_SINE_TABLE_SIZE - 1)
#define TABLE_SHIFT (PHASE_BITS - 8)

esp_err_t max98357_mdl_init(synth_state_t *state, uint32_t sample_rate)
{
    if (!state)
        return ESP_ERR_INVALID_ARG;

    state->sample_rate = sample_rate;
    state->phase_accumulator = 0;

    for (int i = 0; i < SYNTH_SINE_TABLE_SIZE; i++)
    {
        double angle = 2.0 * M_PI * (double)i / (double)SYNTH_SINE_TABLE_SIZE;
        state->sine_table[i] = (int16_t)(32767.0 * sin(angle));
    }

    return ESP_OK;
}

void max98357_mdl_fill_tone(synth_state_t *state, int16_t *buf, size_t samples, uint16_t freq_hz, uint8_t volume_pct)
{
    if (!state || !buf)
        return;

    if (freq_hz == 0 || volume_pct == 0)
    {
        max98357_mdl_fill_silence(buf, samples);
        return;
    }

    uint32_t phase_inc = (uint32_t)(((uint64_t)freq_hz << PHASE_BITS) / state->sample_rate);
    int32_t vol = (int32_t)volume_pct;

    for (size_t i = 0; i < samples; i++)
    {
        uint8_t index = (uint8_t)(state->phase_accumulator >> TABLE_SHIFT);
        int32_t sample = (int32_t)state->sine_table[index & TABLE_MASK];
        buf[i] = (int16_t)(sample * vol / 100);
        state->phase_accumulator += phase_inc;
    }
}

void max98357_mdl_fill_silence(int16_t *buf, size_t samples)
{
    if (!buf)
        return;

    memset(buf, 0, samples * sizeof(int16_t));
}
