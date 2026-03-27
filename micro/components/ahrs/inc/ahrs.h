#ifndef AHRS_H
#define AHRS_H

#include "esp_err.h"
#include "sensor.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define AHRS_DEFAULT_BETA 0.1f
#define AHRS_DEFAULT_SAMPLE_RATE 100.0f
#define AHRS_GRAVITY_MS2 9.80665f

    typedef struct ahrs_cfg_s
    {
        float beta;
        float sample_rate_hz;
    } ahrs_cfg_t;

    typedef struct ahrs_state_s
    {
        float q[4];
        float r[3][3];
        bool initialized;
    } ahrs_state_t;

    /**
     * @brief Initialise the AHRS state to identity orientation.
     *
     * @param[out] state  AHRS state to initialise.
     * @param[in]  cfg    Configuration (beta, sample rate).
     * @return ESP_OK on success, ESP_ERR_INVALID_ARG if any pointer is NULL.
     */
    esp_err_t ahrs_init(ahrs_state_t *state, const ahrs_cfg_t *cfg);

    /**
     * @brief Run one Madgwick filter iteration.
     *
     * @param[in,out] state  Current AHRS state (quaternion + rotation matrix).
     * @param[in]     cfg    Filter configuration.
     * @param[in]     imu    IMU sample (accel + gyro).
     * @return ESP_OK on success, ESP_ERR_INVALID_ARG on NULL pointer,
     *         ESP_ERR_INVALID_STATE if not initialised.
     */
    esp_err_t ahrs_update(ahrs_state_t *state, const ahrs_cfg_t *cfg, const data_imu_t *imu);

    /**
     * @brief Reset the AHRS state back to identity orientation.
     *
     * @param[out] state  AHRS state to reset.
     * @return ESP_OK on success, ESP_ERR_INVALID_ARG if state is NULL.
     */
    esp_err_t ahrs_reset(ahrs_state_t *state);

    /**
     * @brief Compute gravity-compensated vertical acceleration in m/s².
     *
     * @param[in]  state              Initialised AHRS state.
     * @param[in]  imu                Current IMU reading.
     * @param[out] vertical_accel_ms2 Resulting vertical acceleration (positive = up).
     * @return ESP_OK on success, ESP_ERR_INVALID_ARG on NULL pointer,
     *         ESP_ERR_INVALID_STATE if not initialised.
     */
    esp_err_t ahrs_get_vertical_accel(const ahrs_state_t *state, const data_imu_t *imu, float *vertical_accel_ms2);

#ifdef __cplusplus
}
#endif

#endif // AHRS_H
