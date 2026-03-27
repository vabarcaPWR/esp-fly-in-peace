#include "ahrs.h"

#include <math.h>
#include <string.h>

static void quaternion_to_rotation_matrix(const float q[4], float r[3][3])
{
    float q0 = q[0], q1 = q[1], q2 = q[2], q3 = q[3];
    float q0q0 = q0 * q0, q1q1 = q1 * q1, q2q2 = q2 * q2, q3q3 = q3 * q3;
    float q0q1 = q0 * q1, q0q2 = q0 * q2, q0q3 = q0 * q3;
    float q1q2 = q1 * q2, q1q3 = q1 * q3, q2q3 = q2 * q3;

    r[0][0] = q0q0 + q1q1 - q2q2 - q3q3;
    r[0][1] = 2.0f * (q1q2 + q0q3);
    r[0][2] = 2.0f * (q1q3 - q0q2);
    r[1][0] = 2.0f * (q1q2 - q0q3);
    r[1][1] = q0q0 - q1q1 + q2q2 - q3q3;
    r[1][2] = 2.0f * (q2q3 + q0q1);
    r[2][0] = 2.0f * (q1q3 + q0q2);
    r[2][1] = 2.0f * (q2q3 - q0q1);
    r[2][2] = q0q0 - q1q1 - q2q2 + q3q3;
}

static void set_identity_quaternion(ahrs_state_t *state)
{
    state->q[0] = 1.0f;
    state->q[1] = 0.0f;
    state->q[2] = 0.0f;
    state->q[3] = 0.0f;
    quaternion_to_rotation_matrix(state->q, state->r);
}

esp_err_t ahrs_init(ahrs_state_t *state, const ahrs_cfg_t *cfg)
{
    if (!state || !cfg)
        return ESP_ERR_INVALID_ARG;

    memset(state, 0, sizeof(*state));
    set_identity_quaternion(state);
    state->initialized = true;
    return ESP_OK;
}

esp_err_t ahrs_reset(ahrs_state_t *state)
{
    if (!state)
        return ESP_ERR_INVALID_ARG;

    memset(state, 0, sizeof(*state));
    set_identity_quaternion(state);
    state->initialized = false;
    return ESP_OK;
}

esp_err_t ahrs_update(ahrs_state_t *state, const ahrs_cfg_t *cfg, const data_imu_t *imu)
{
    if (!state || !cfg || !imu)
        return ESP_ERR_INVALID_ARG;

    if (!state->initialized)
        return ESP_ERR_INVALID_STATE;

    float q0 = state->q[0], q1 = state->q[1], q2 = state->q[2], q3 = state->q[3];
    float gx = imu->gyro_x, gy = imu->gyro_y, gz = imu->gyro_z;
    float ax = imu->accel_x, ay = imu->accel_y, az = imu->accel_z;

    float accel_norm = sqrtf(ax * ax + ay * ay + az * az);
    if (accel_norm < 1e-6f)
        return ESP_OK;

    float inv_norm = 1.0f / accel_norm;
    ax *= -inv_norm;
    ay *= -inv_norm;
    az *= -inv_norm;

    float _2q0 = 2.0f * q0, _2q1 = 2.0f * q1, _2q2 = 2.0f * q2, _2q3 = 2.0f * q3;
    float _4q0 = 4.0f * q0, _4q1 = 4.0f * q1, _4q2 = 4.0f * q2;
    float _8q1 = 8.0f * q1, _8q2 = 8.0f * q2;
    float q0q0 = q0 * q0, q1q1 = q1 * q1, q2q2 = q2 * q2, q3q3 = q3 * q3;

    float s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
    float s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * q1 - _2q0 * ay - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
    float s2 = 4.0f * q0q0 * q2 + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
    float s3 = 4.0f * q1q1 * q3 - _2q1 * ax + 4.0f * q2q2 * q3 - _2q2 * ay;

    float grad_norm = sqrtf(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3);
    if (grad_norm > 1e-6f)
    {
        float inv_grad = 1.0f / grad_norm;
        s0 *= inv_grad;
        s1 *= inv_grad;
        s2 *= inv_grad;
        s3 *= inv_grad;
    }

    float dt = 1.0f / cfg->sample_rate_hz;
    float beta = cfg->beta;

    float qDot0 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz) - beta * s0;
    float qDot1 = 0.5f * (q0 * gx + q2 * gz - q3 * gy) - beta * s1;
    float qDot2 = 0.5f * (q0 * gy - q1 * gz + q3 * gx) - beta * s2;
    float qDot3 = 0.5f * (q0 * gz + q1 * gy - q2 * gx) - beta * s3;

    q0 += qDot0 * dt;
    q1 += qDot1 * dt;
    q2 += qDot2 * dt;
    q3 += qDot3 * dt;

    float qnorm = sqrtf(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    float inv_qnorm = 1.0f / qnorm;
    state->q[0] = q0 * inv_qnorm;
    state->q[1] = q1 * inv_qnorm;
    state->q[2] = q2 * inv_qnorm;
    state->q[3] = q3 * inv_qnorm;

    quaternion_to_rotation_matrix(state->q, state->r);
    return ESP_OK;
}

esp_err_t ahrs_get_vertical_accel(const ahrs_state_t *state, const data_imu_t *imu, float *vertical_accel_ms2)
{
    if (!state || !imu || !vertical_accel_ms2)
        return ESP_ERR_INVALID_ARG;

    if (!state->initialized)
        return ESP_ERR_INVALID_STATE;

    float a_down = state->r[2][0] * imu->accel_x + state->r[2][1] * imu->accel_y + state->r[2][2] * imu->accel_z;
    *vertical_accel_ms2 = -(a_down + AHRS_GRAVITY_MS2);
    return ESP_OK;
}
