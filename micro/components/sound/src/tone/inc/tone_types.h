#ifndef TONE_TYPES_H
#define TONE_TYPES_H

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define TONE_MAX_POINTS 12

    typedef enum tone_zone_e
    {
        TONE_ZONE_SILENCE = 0,
        TONE_ZONE_PRE_LIFT,
        TONE_ZONE_CLIMB,
        TONE_ZONE_SINK,
    } tone_zone_e;

    typedef struct tone_point_s
    {
        float vario_ms;
        uint16_t freq_hz;
        uint16_t cycle_ms;
        uint8_t duty_pct;
    } tone_point_t;

    typedef struct tone_curve_s
    {
        tone_point_t points[TONE_MAX_POINTS];
        uint8_t count;
    } tone_curve_t;

    typedef struct tone_thresholds_s
    {
        float climb_on_ms;
        float climb_off_ms;
        float sink_on_ms;
        float sink_off_ms;
    } tone_thresholds_t;

    typedef struct tone_output_s
    {
        uint16_t freq_hz;
        uint16_t cycle_ms;
        uint8_t duty_pct;
        tone_zone_e zone;
    } tone_output_t;

    typedef struct tone_config_s
    {
        tone_curve_t curve;
        tone_thresholds_t thresholds;
        bool pre_lift_enabled;
        bool muted;
        uint8_t volume_pct;
    } tone_config_t;

#ifdef __cplusplus
}
#endif

#endif // TONE_TYPES_H
