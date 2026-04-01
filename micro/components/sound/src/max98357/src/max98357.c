#include "max98357.h"
#include "max98357_hardware.h"
#include "max98357_model.h"
#include "tone_model.h"
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MAX98357_SAMPLE_RATE 16000
#define MAX98357_UPDATE_PERIOD_MS 50
#define MAX98357_SAMPLES_PER_UPDATE (MAX98357_SAMPLE_RATE * MAX98357_UPDATE_PERIOD_MS / 1000)
#define MAX98357_FREQ_SMOOTH_MAX_STEP 50
#define MAX98357_STARTUP_VOLUME_PCT 30
#define FADE_SAMPLES 64

typedef enum beep_phase_e
{
    BEEP_PHASE_ON = 0,
    BEEP_PHASE_OFF,
} beep_phase_e;

typedef struct startup_tone_step_s
{
    uint16_t freq_hz;
    uint16_t duration_ms;
    uint16_t gap_ms;
} startup_tone_step_t;

static const startup_tone_step_t s_startup_sequence[] = {
    {523, 80, 30},
    {659, 80, 30},
    {784, 120, 0},
};

#define STARTUP_SEQUENCE_LEN (sizeof(s_startup_sequence) / sizeof(s_startup_sequence[0]))
#define STARTUP_SAMPLES_PER_MS (MAX98357_SAMPLE_RATE / 1000)

static synth_state_t s_synth;
static tone_config_t s_config;
static int16_t s_pcm_buf[MAX98357_SAMPLES_PER_UPDATE];
static beep_phase_e s_beep_phase;
static uint16_t s_beep_elapsed_ms;
static uint16_t s_current_freq_hz;
static bool s_muted;
static bool s_was_playing;

static int16_t clamp_step(int16_t delta, int16_t max_step)
{
    if (delta > max_step)
        return max_step;
    if (delta < -max_step)
        return -max_step;
    return delta;
}

static uint16_t smooth_frequency(uint16_t target_freq, uint16_t current_freq)
{
    int16_t delta = (int16_t)target_freq - (int16_t)current_freq;
    int16_t step = clamp_step(delta, MAX98357_FREQ_SMOOTH_MAX_STEP);
    return (uint16_t)((int16_t)current_freq + step);
}

static void apply_fade_in(int16_t *buf, size_t total_samples)
{
    size_t n = FADE_SAMPLES < total_samples ? FADE_SAMPLES : total_samples;
    for (size_t i = 0; i < n; i++)
        buf[i] = (int16_t)((int32_t)buf[i] * (int32_t)i / (int32_t)n);
}

static void apply_fade_out(int16_t *buf, size_t total_samples)
{
    size_t n = FADE_SAMPLES < total_samples ? FADE_SAMPLES : total_samples;
    size_t start = total_samples - n;
    for (size_t i = 0; i < n; i++)
        buf[start + i] = (int16_t)((int32_t)buf[start + i] * (int32_t)(n - i) / (int32_t)n);
}

static esp_err_t max98357_init(void)
{
    const tone_config_t *defaults = tone_config_get_defaults();
    s_config = *defaults;
    s_beep_phase = BEEP_PHASE_ON;
    s_beep_elapsed_ms = 0;
    s_current_freq_hz = 0;
    s_muted = s_config.muted;
    s_was_playing = false;

    esp_err_t ret = max98357_mdl_init(&s_synth, MAX98357_SAMPLE_RATE);
    if (ret != ESP_OK)
        return ret;

    max98357_gpio_cfg_t gpio_cfg = {
        .bclk = CONFIG_MAX98357_BCLK_GPIO,
        .ws = CONFIG_MAX98357_WS_GPIO,
        .dout = CONFIG_MAX98357_DOUT_GPIO,
        .sd = CONFIG_MAX98357_SD_GPIO,
    };

    return max98357_hw_init(&gpio_cfg);
}

static void write_tone(uint16_t freq_hz, uint8_t volume_pct)
{
    max98357_mdl_fill_tone(&s_synth, s_pcm_buf, MAX98357_SAMPLES_PER_UPDATE, freq_hz, volume_pct);
    if (!s_was_playing)
        apply_fade_in(s_pcm_buf, MAX98357_SAMPLES_PER_UPDATE);
    s_was_playing = true;
    max98357_hw_write(s_pcm_buf, MAX98357_SAMPLES_PER_UPDATE);
}

static void write_silence(void)
{
    if (s_was_playing && s_current_freq_hz > 0)
    {
        max98357_mdl_fill_tone(&s_synth, s_pcm_buf, FADE_SAMPLES, s_current_freq_hz, s_config.volume_pct);
        apply_fade_out(s_pcm_buf, FADE_SAMPLES);
        memset(&s_pcm_buf[FADE_SAMPLES], 0,
               (MAX98357_SAMPLES_PER_UPDATE - FADE_SAMPLES) * sizeof(int16_t));
    }
    else
    {
        max98357_mdl_fill_silence(s_pcm_buf, MAX98357_SAMPLES_PER_UPDATE);
    }
    s_was_playing = false;
    max98357_hw_write(s_pcm_buf, MAX98357_SAMPLES_PER_UPDATE);
}

static void handle_continuous_tone(uint16_t freq_hz, uint8_t duty_pct)
{
    s_current_freq_hz = smooth_frequency(freq_hz, s_current_freq_hz);
    write_tone(s_current_freq_hz, s_config.volume_pct);
}

static void handle_beeping_tone(const tone_output_t *tone)
{
    s_beep_elapsed_ms += MAX98357_UPDATE_PERIOD_MS;

    uint16_t on_duration_ms = (uint16_t)((uint32_t)tone->cycle_ms * tone->duty_pct / 100);
    uint16_t off_duration_ms = tone->cycle_ms - on_duration_ms;

    if (s_beep_phase == BEEP_PHASE_ON)
    {
        s_current_freq_hz = smooth_frequency(tone->freq_hz, s_current_freq_hz);
        write_tone(s_current_freq_hz, s_config.volume_pct);

        if (s_beep_elapsed_ms >= on_duration_ms)
        {
            s_beep_phase = BEEP_PHASE_OFF;
            s_beep_elapsed_ms = 0;
        }
    }
    else
    {
        write_silence();

        if (s_beep_elapsed_ms >= off_duration_ms)
        {
            s_beep_phase = BEEP_PHASE_ON;
            s_beep_elapsed_ms = 0;
        }
    }
}

static esp_err_t max98357_update(double vario_cms)
{
    if (s_muted)
    {
        write_silence();
        return ESP_OK;
    }

    float vario_ms = (float)(vario_cms / 100.0);

    tone_output_t tone = {0};
    tone_model_compute(&s_config.curve, &s_config.thresholds, s_config.pre_lift_enabled, vario_ms, &tone);

    if (tone.zone == TONE_ZONE_SILENCE)
    {
        write_silence();
        s_current_freq_hz = 0;
        s_beep_phase = BEEP_PHASE_ON;
        s_beep_elapsed_ms = 0;
        return ESP_OK;
    }

    if (tone.zone == TONE_ZONE_SINK)
    {
        handle_continuous_tone(tone.freq_hz, tone.duty_pct);
        return ESP_OK;
    }

    handle_beeping_tone(&tone);
    return ESP_OK;
}

static esp_err_t max98357_play_startup(void)
{
    int16_t buf[STARTUP_SAMPLES_PER_MS * 10];
    synth_state_t startup_synth;
    max98357_mdl_init(&startup_synth, MAX98357_SAMPLE_RATE);

    for (size_t i = 0; i < STARTUP_SEQUENCE_LEN; i++)
    {
        uint16_t remaining_ms = s_startup_sequence[i].duration_ms;
        while (remaining_ms > 0)
        {
            uint16_t chunk_ms = remaining_ms > 10 ? 10 : remaining_ms;
            size_t chunk_samples = (size_t)chunk_ms * STARTUP_SAMPLES_PER_MS;
            max98357_mdl_fill_tone(&startup_synth, buf, chunk_samples, s_startup_sequence[i].freq_hz,
                            MAX98357_STARTUP_VOLUME_PCT);
            max98357_hw_write(buf, chunk_samples);
            remaining_ms -= chunk_ms;
        }

        if (s_startup_sequence[i].gap_ms > 0)
        {
            uint16_t gap_remaining = s_startup_sequence[i].gap_ms;
            while (gap_remaining > 0)
            {
                uint16_t chunk_ms = gap_remaining > 10 ? 10 : gap_remaining;
                size_t chunk_samples = (size_t)chunk_ms * STARTUP_SAMPLES_PER_MS;
                max98357_mdl_fill_silence(buf, chunk_samples);
                max98357_hw_write(buf, chunk_samples);
                gap_remaining -= chunk_ms;
            }
        }
    }

    return ESP_OK;
}

static const char *max98357_get_name(void)
{
    return "max98357";
}

static const sound_generator_t s_max98357_generator = {
    .init = max98357_init,
    .update = max98357_update,
    .play_startup = max98357_play_startup,
    .get_name = max98357_get_name,
};

const sound_generator_t *get_max98357_sound_generator(void)
{
    return &s_max98357_generator;
}
