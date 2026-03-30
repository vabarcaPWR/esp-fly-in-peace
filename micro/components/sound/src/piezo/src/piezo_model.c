#include "piezo_model.h"
#include <math.h>
#include <stddef.h>

#define PIEZO_FREQ_MIN 100
#define PIEZO_FREQ_MAX 4000
#define PIEZO_PRE_LIFT_FREQ_HZ 400
#define PIEZO_PRE_LIFT_CYCLE_MS 1000
#define PIEZO_PRE_LIFT_DUTY_PCT 5

static float lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

static void interpolate_curve(const piezo_tone_curve_t *curve, float vario_ms, piezo_tone_output_t *out)
{
    if (curve->count == 0)
        return;

    if (vario_ms <= curve->points[0].vario_ms)
    {
        out->freq_hz = curve->points[0].freq_hz;
        out->cycle_ms = curve->points[0].cycle_ms;
        out->duty_pct = curve->points[0].duty_pct;
        return;
    }

    uint8_t last = curve->count - 1;
    if (vario_ms >= curve->points[last].vario_ms)
    {
        out->freq_hz = curve->points[last].freq_hz;
        out->cycle_ms = curve->points[last].cycle_ms;
        out->duty_pct = curve->points[last].duty_pct;
        return;
    }

    for (uint8_t i = 0; i < last; i++)
    {
        const piezo_tone_point_t *a = &curve->points[i];
        const piezo_tone_point_t *b = &curve->points[i + 1];

        if (vario_ms >= a->vario_ms && vario_ms <= b->vario_ms)
        {
            float range = b->vario_ms - a->vario_ms;
            float t = (range > 0.0f) ? (vario_ms - a->vario_ms) / range : 0.0f;

            out->freq_hz = (uint16_t)(lerp((float)a->freq_hz, (float)b->freq_hz, t) + 0.5f);
            out->cycle_ms = (uint16_t)(lerp((float)a->cycle_ms, (float)b->cycle_ms, t) + 0.5f);
            out->duty_pct = (uint8_t)(lerp((float)a->duty_pct, (float)b->duty_pct, t) + 0.5f);
            return;
        }
    }
}

void piezo_model_compute(const piezo_tone_curve_t *curve, const piezo_tone_thresholds_t *thresh, bool pre_lift_enabled,
                         float vario_ms, piezo_tone_output_t *out)
{
    if (!curve || !thresh || !out)
        return;

    out->freq_hz = 0;
    out->cycle_ms = 0;
    out->duty_pct = 0;
    out->zone = PIEZO_ZONE_SILENCE;

    if (vario_ms >= thresh->climb_on_ms)
    {
        out->zone = PIEZO_ZONE_CLIMB;
        interpolate_curve(curve, vario_ms, out);
    }
    else if (vario_ms <= thresh->sink_on_ms)
    {
        out->zone = PIEZO_ZONE_SINK;
        interpolate_curve(curve, vario_ms, out);
    }
    else if (pre_lift_enabled && vario_ms >= thresh->climb_off_ms && vario_ms < thresh->climb_on_ms)
    {
        out->zone = PIEZO_ZONE_PRE_LIFT;
        out->freq_hz = PIEZO_PRE_LIFT_FREQ_HZ;
        out->cycle_ms = PIEZO_PRE_LIFT_CYCLE_MS;
        out->duty_pct = PIEZO_PRE_LIFT_DUTY_PCT;
    }
}

static const piezo_tone_config_t s_default_config = {
    .curve =
        {
            .points =
                {
                    {-10.0f, 200, 0, 100},
                    {-2.0f, 280, 0, 100},
                    {0.0f, 400, 600, 30},
                    {0.5f, 550, 500, 35},
                    {1.0f, 700, 420, 40},
                    {2.0f, 950, 320, 45},
                    {3.0f, 1150, 250, 50},
                    {5.0f, 1400, 200, 55},
                    {8.0f, 1550, 180, 55},
                    {10.0f, 1600, 180, 55},
                },
            .count = 10,
        },
    .thresholds =
        {
            .climb_on_ms = 0.15f,
            .climb_off_ms = 0.05f,
            .sink_on_ms = -2.0f,
            .sink_off_ms = -1.5f,
        },
    .pre_lift_enabled = true,
    .muted = false,
    .volume_pct = 100,
};

const piezo_tone_config_t *piezo_config_get_defaults(void)
{
    return &s_default_config;
}

esp_err_t piezo_config_validate(const piezo_tone_config_t *config)
{
    if (!config)
        return ESP_ERR_INVALID_ARG;

    if (config->curve.count < 2 || config->curve.count > PIEZO_MAX_TONE_POINTS)
        return ESP_ERR_INVALID_ARG;

    for (uint8_t i = 0; i < config->curve.count; i++)
    {
        const piezo_tone_point_t *p = &config->curve.points[i];
        if (p->freq_hz > PIEZO_FREQ_MAX)
            return ESP_ERR_INVALID_ARG;
        if (p->duty_pct > 100)
            return ESP_ERR_INVALID_ARG;
    }

    for (uint8_t i = 1; i < config->curve.count; i++)
    {
        if (config->curve.points[i].vario_ms <= config->curve.points[i - 1].vario_ms)
            return ESP_ERR_INVALID_ARG;
    }

    if (config->thresholds.climb_on_ms <= config->thresholds.climb_off_ms)
        return ESP_ERR_INVALID_ARG;

    if (config->thresholds.sink_on_ms >= config->thresholds.sink_off_ms)
        return ESP_ERR_INVALID_ARG;

    if (config->volume_pct > 100)
        return ESP_ERR_INVALID_ARG;

    return ESP_OK;
}
