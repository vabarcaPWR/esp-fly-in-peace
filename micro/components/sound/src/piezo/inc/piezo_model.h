#ifndef PIEZO_MODEL_H
#define PIEZO_MODEL_H

#include "piezo_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Compute tone output (freq, cycle, duty, zone) from vertical speed.
     * Pure function — no side effects, no ESP-IDF dependencies.
     */
    void piezo_model_compute(const piezo_tone_curve_t *curve, const piezo_tone_thresholds_t *thresh,
                             bool pre_lift_enabled, float vario_ms, piezo_tone_output_t *out);

    /**
     * Returns pointer to static default comfort-first tone config.
     * This config will be exposed via BLE Config Service in Phase 11.
     * Add piezo_tone_config_t to device_config_t when implementing Phase 11.
     */
    const piezo_tone_config_t *piezo_config_get_defaults(void);

    /**
     * Validates all config fields with range checks.
     * Returns ESP_OK if valid, ESP_ERR_INVALID_ARG otherwise.
     */
    esp_err_t piezo_config_validate(const piezo_tone_config_t *config);

#ifdef __cplusplus
}
#endif

#endif // PIEZO_MODEL_H
