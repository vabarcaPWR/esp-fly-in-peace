#include "piezo.h"
#include "piezo_hardware.h"
#include "tone_model.h"
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#define PIEZO_FREQ_SMOOTH_MAX_STEP 50
#define PIEZO_BEEP_UPDATE_RESOLUTION_MS 50
#define PIEZO_STARTUP_DUTY_PCT 50

typedef enum beep_phase_e
{
    BEEP_PHASE_ON = 0,
    BEEP_PHASE_OFF,
} beep_phase_e;

typedef struct piezo_context_s
{
    tone_config_t config;
    SemaphoreHandle_t config_mutex;
    beep_phase_e beep_phase;
    uint16_t beep_elapsed_ms;
    uint16_t current_freq_hz;
    bool muted;
} piezo_context_t;

static piezo_context_t s_self;

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
    s_self.config_mutex = xSemaphoreCreateMutex();
    if (!s_self.config_mutex)
        return ESP_ERR_NO_MEM;

    const tone_config_t *defaults = tone_config_get_defaults();
    s_self.config = *defaults;
    s_self.beep_phase = BEEP_PHASE_ON;
    s_self.beep_elapsed_ms = 0;
    s_self.current_freq_hz = 0;
    s_self.muted = s_self.config.muted;

#ifdef CONFIG_PIEZO_GPIO
    return piezo_hw_init(CONFIG_PIEZO_GPIO);
#else
    return piezo_hw_init(5);
#endif
}

static void handle_continuous_tone(uint16_t freq_hz, uint8_t duty_pct)
{
    s_self.current_freq_hz = smooth_frequency(freq_hz, s_self.current_freq_hz);
    piezo_hw_set_tone(s_self.current_freq_hz, duty_pct);
}

static void handle_beeping_tone(const tone_output_t *tone)
{
    s_self.beep_elapsed_ms += PIEZO_BEEP_UPDATE_RESOLUTION_MS;

    uint16_t on_duration_ms = (uint16_t)((uint32_t)tone->cycle_ms * tone->duty_pct / 100);
    uint16_t off_duration_ms = tone->cycle_ms - on_duration_ms;

    if (s_self.beep_phase == BEEP_PHASE_ON)
    {
        s_self.current_freq_hz = smooth_frequency(tone->freq_hz, s_self.current_freq_hz);
        piezo_hw_set_tone(s_self.current_freq_hz, tone->duty_pct);

        if (s_self.beep_elapsed_ms >= on_duration_ms)
        {
            s_self.beep_phase = BEEP_PHASE_OFF;
            s_self.beep_elapsed_ms = 0;
        }
    }
    else
    {
        piezo_hw_mute();

        if (s_self.beep_elapsed_ms >= off_duration_ms)
        {
            s_self.beep_phase = BEEP_PHASE_ON;
            s_self.beep_elapsed_ms = 0;
        }
    }
}

static esp_err_t piezo_update(double vario_cms)
{
    if (s_self.muted)
    {
        piezo_hw_mute();
        return ESP_OK;
    }

    float vario_ms = (float)(vario_cms / 100.0);

    tone_output_t tone = {0};
    tone_model_compute(&s_self.config.curve, &s_self.config.thresholds, s_self.config.pre_lift_enabled, vario_ms,
                       &tone);

    if (tone.zone == TONE_ZONE_SILENCE)
    {
        piezo_hw_mute();
        s_self.current_freq_hz = 0;
        s_self.beep_phase = BEEP_PHASE_ON;
        s_self.beep_elapsed_ms = 0;
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

static esp_err_t piezo_set_config(const tone_config_t *cfg)
{
    if (!cfg)
        return ESP_ERR_INVALID_ARG;

    esp_err_t ret = tone_config_validate(cfg);
    if (ret != ESP_OK)
        return ret;

    xSemaphoreTake(s_self.config_mutex, portMAX_DELAY);
    s_self.config = *cfg;
    s_self.muted = cfg->muted;
    xSemaphoreGive(s_self.config_mutex);
    return ESP_OK;
}

static esp_err_t piezo_get_config(tone_config_t *cfg)
{
    if (!cfg)
        return ESP_ERR_INVALID_ARG;

    xSemaphoreTake(s_self.config_mutex, portMAX_DELAY);
    *cfg = s_self.config;
    xSemaphoreGive(s_self.config_mutex);
    return ESP_OK;
}

static const sound_generator_t s_piezo_generator = {
    .init = piezo_init,
    .update = piezo_update,
    .play_startup = piezo_play_startup,
    .get_name = piezo_get_name,
    .set_config = piezo_set_config,
    .get_config = piezo_get_config,
};

const sound_generator_t *get_piezo_sound_generator(void)
{
    return &s_piezo_generator;
}
