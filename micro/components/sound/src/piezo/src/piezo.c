#include "piezo.h"
#include "piezo_hardware.h"
#include "tone_model.h"
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define PIEZO_FREQ_SMOOTH_MAX_STEP 50
#define PIEZO_BEEP_UPDATE_RESOLUTION_MS 50
#define PIEZO_STARTUP_DUTY_PCT 50

typedef enum beep_phase_e
{
    BEEP_PHASE_ON = 0,
    BEEP_PHASE_OFF,
} beep_phase_e;

static tone_config_t s_config;
static beep_phase_e s_beep_phase;
static uint16_t s_beep_elapsed_ms;
static uint16_t s_current_freq_hz;
static bool s_muted;

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
    int16_t step = clamp_step(delta, PIEZO_FREQ_SMOOTH_MAX_STEP);
    return (uint16_t)((int16_t)current_freq + step);
}

static esp_err_t piezo_init(void)
{
    const tone_config_t *defaults = tone_config_get_defaults();
    s_config = *defaults;
    s_beep_phase = BEEP_PHASE_ON;
    s_beep_elapsed_ms = 0;
    s_current_freq_hz = 0;
    s_muted = s_config.muted;

#ifdef CONFIG_PIEZO_GPIO
    return piezo_hw_init(CONFIG_PIEZO_GPIO);
#else
    return piezo_hw_init(5);
#endif
}

static void handle_continuous_tone(uint16_t freq_hz, uint8_t duty_pct)
{
    s_current_freq_hz = smooth_frequency(freq_hz, s_current_freq_hz);
    piezo_hw_set_tone(s_current_freq_hz, duty_pct);
}

static void handle_beeping_tone(const tone_output_t *tone)
{
    s_beep_elapsed_ms += PIEZO_BEEP_UPDATE_RESOLUTION_MS;

    uint16_t on_duration_ms = (uint16_t)((uint32_t)tone->cycle_ms * tone->duty_pct / 100);
    uint16_t off_duration_ms = tone->cycle_ms - on_duration_ms;

    if (s_beep_phase == BEEP_PHASE_ON)
    {
        s_current_freq_hz = smooth_frequency(tone->freq_hz, s_current_freq_hz);
        piezo_hw_set_tone(s_current_freq_hz, tone->duty_pct);

        if (s_beep_elapsed_ms >= on_duration_ms)
        {
            s_beep_phase = BEEP_PHASE_OFF;
            s_beep_elapsed_ms = 0;
        }
    }
    else
    {
        piezo_hw_mute();

        if (s_beep_elapsed_ms >= off_duration_ms)
        {
            s_beep_phase = BEEP_PHASE_ON;
            s_beep_elapsed_ms = 0;
        }
    }
}

static esp_err_t piezo_update(double vario_cms)
{
    if (s_muted)
    {
        piezo_hw_mute();
        return ESP_OK;
    }

    float vario_ms = (float)(vario_cms / 100.0);

    tone_output_t tone = {0};
    tone_model_compute(&s_config.curve, &s_config.thresholds, s_config.pre_lift_enabled, vario_ms, &tone);

    if (tone.zone == TONE_ZONE_SILENCE)
    {
        piezo_hw_mute();
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

static esp_err_t piezo_play_startup(void)
{
    for (size_t i = 0; i < STARTUP_SEQUENCE_LEN; i++)
    {
        piezo_hw_set_tone(s_startup_sequence[i].freq_hz, PIEZO_STARTUP_DUTY_PCT);
        vTaskDelay(pdMS_TO_TICKS(s_startup_sequence[i].duration_ms));

        piezo_hw_mute();
        if (s_startup_sequence[i].gap_ms > 0)
            vTaskDelay(pdMS_TO_TICKS(s_startup_sequence[i].gap_ms));
    }

    return ESP_OK;
}

static const char *piezo_get_name(void)
{
    return "piezo";
}

static const sound_generator_t s_piezo_generator = {
    .init = piezo_init,
    .update = piezo_update,
    .play_startup = piezo_play_startup,
    .get_name = piezo_get_name,
};

const sound_generator_t *get_piezo_sound_generator(void)
{
    return &s_piezo_generator;
}
