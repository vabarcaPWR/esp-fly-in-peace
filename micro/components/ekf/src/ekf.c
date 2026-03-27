#include "ekf.h"
#include <math.h>
#include <string.h>

float ekf_pressure_to_altitude(float pressure_pa, float reference_pressure_pa)
{
    return EKF_BARO_FORMULA_COEFF * (1.0f - powf(pressure_pa / reference_pressure_pa, EKF_BARO_FORMULA_EXP));
}

esp_err_t ekf_reset(ekf_state_t *state)
{
    if (!state)
    {
        return ESP_ERR_INVALID_ARG;
    }
    memset(state, 0, sizeof(*state));
    return ESP_OK;
}

esp_err_t ekf_init(ekf_state_t *state, const ekf_cfg_t *cfg)
{
    if (!state || !cfg)
    {
        return ESP_ERR_INVALID_ARG;
    }
    memset(state, 0, sizeof(*state));
    state->p[0][0] = 1.0f;
    state->p[1][1] = 1.0f;
    state->p[2][2] = 0.01f;
    return ESP_OK;
}

esp_err_t ekf_predict(ekf_state_t *state, const ekf_cfg_t *cfg, float vertical_accel_ms2, int64_t timestamp_us)
{
    if (!state || !cfg)
    {
        return ESP_ERR_INVALID_ARG;
    }
    if (!state->initialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    float dt = (float)(timestamp_us - state->last_predict_us) / 1e6f;
    if (dt <= 0.0f || dt > 1.0f)
    {
        return ESP_OK;
    }

    float accel_corrected = vertical_accel_ms2 - state->accel_bias_ms2;
    float half_dt_sq = 0.5f * accel_corrected * dt * dt;

    state->altitude_m += state->vario_ms * dt + half_dt_sq;
    state->vario_ms += accel_corrected * dt;

    float dt2 = dt * dt;
    float half_dt2 = 0.5f * dt2;

    float p[3][3];
    memcpy(p, state->p, sizeof(p));

    float fp[3][3];
    for (int i = 0; i < 3; i++)
    {
        fp[0][i] = p[0][i] + dt * p[1][i] - half_dt2 * p[2][i];
        fp[1][i] = p[1][i] - dt * p[2][i];
        fp[2][i] = p[2][i];
    }

    for (int i = 0; i < 3; i++)
    {
        state->p[i][0] = fp[i][0] + dt * fp[i][1] - half_dt2 * fp[i][2];
        state->p[i][1] = fp[i][1] - dt * fp[i][2];
        state->p[i][2] = fp[i][2];
    }

    state->p[0][0] += cfg->q_altitude * dt;
    state->p[1][1] += cfg->q_vario * dt;
    state->p[2][2] += cfg->q_accel_bias * dt;

    state->last_predict_us = timestamp_us;
    return ESP_OK;
}

esp_err_t ekf_update_baro(ekf_state_t *state, const ekf_cfg_t *cfg, float pressure_pa, int64_t timestamp_us)
{
    if (!state || !cfg)
    {
        return ESP_ERR_INVALID_ARG;
    }

    float baro_alt = ekf_pressure_to_altitude(pressure_pa, cfg->reference_pressure_pa);

    if (!state->initialized)
    {
        state->altitude_m = baro_alt;
        state->initialized = true;
        state->last_predict_us = timestamp_us;
        state->last_baro_us = timestamp_us;
        return ESP_OK;
    }

    float innovation = baro_alt - state->altitude_m;
    float innovation_variance = state->p[0][0] + cfg->r_altitude;

    float gate_threshold = EKF_INNOVATION_GATE_SIGMA * EKF_INNOVATION_GATE_SIGMA * innovation_variance;
    if (innovation * innovation > gate_threshold)
    {
        return ESP_OK;
    }

    float k[3];
    k[0] = state->p[0][0] / innovation_variance;
    k[1] = state->p[1][0] / innovation_variance;
    k[2] = state->p[2][0] / innovation_variance;

    state->altitude_m += k[0] * innovation;
    state->vario_ms += k[1] * innovation;
    state->accel_bias_ms2 += k[2] * innovation;

    float p0[3] = {state->p[0][0], state->p[0][1], state->p[0][2]};
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            state->p[i][j] -= k[i] * p0[j];
        }
    }

    state->last_baro_us = timestamp_us;
    return ESP_OK;
}

esp_err_t ekf_calibrate(ekf_cfg_t *cfg, ekf_state_t *state, float known_altitude_m, float current_pressure_pa)
{
    if (!cfg || !state)
    {
        return ESP_ERR_INVALID_ARG;
    }
    if (known_altitude_m < EKF_ALTITUDE_MIN_M || known_altitude_m > EKF_ALTITUDE_MAX_M)
    {
        return ESP_ERR_INVALID_ARG;
    }
    if (current_pressure_pa < EKF_PRESSURE_MIN_PA || current_pressure_pa > EKF_PRESSURE_MAX_PA)
    {
        return ESP_ERR_INVALID_ARG;
    }

    float base = 1.0f - known_altitude_m / EKF_BARO_FORMULA_COEFF;
    cfg->reference_pressure_pa = current_pressure_pa / powf(base, EKF_BARO_INV_EXP);

    return ekf_reset(state);
}
