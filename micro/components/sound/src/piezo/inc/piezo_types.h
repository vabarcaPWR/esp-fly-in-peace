#ifndef PIEZO_TYPES_H
#define PIEZO_TYPES_H

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define PIEZO_MAX_TONE_POINTS 12

    typedef enum piezo_tone_zone_e
    {
        PIEZO_ZONE_SILENCE = 0,
        PIEZO_ZONE_PRE_LIFT,
        PIEZO_ZONE_CLIMB,
        PIEZO_ZONE_SINK,
    } piezo_tone_zone_e;

    typedef struct piezo_tone_point_s
    {
        float vario_ms;
        uint16_t freq_hz;
        uint16_t cycle_ms;
        uint8_t duty_pct;
    } piezo_tone_point_t;

    typedef struct piezo_tone_curve_s
    {
        piezo_tone_point_t points[PIEZO_MAX_TONE_POINTS];
        uint8_t count;
    } piezo_tone_curve_t;

    typedef struct piezo_tone_thresholds_s
    {
        float climb_on_ms;
        float climb_off_ms;
        float sink_on_ms;
        float sink_off_ms;
    } piezo_tone_thresholds_t;

    typedef struct piezo_tone_output_s
    {
        uint16_t freq_hz;
        uint16_t cycle_ms;
        uint8_t duty_pct;
        piezo_tone_zone_e zone;
    } piezo_tone_output_t;

    typedef struct piezo_tone_config_s
    {
        piezo_tone_curve_t curve;
        piezo_tone_thresholds_t thresholds;
        bool pre_lift_enabled;
        bool muted;
        uint8_t volume_pct;
    } piezo_tone_config_t;

#ifdef __cplusplus
}
#endif

#endif // PIEZO_TYPES_H
