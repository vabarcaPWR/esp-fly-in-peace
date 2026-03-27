#ifndef EKF_H
#define EKF_H

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define EKF_DEFAULT_Q_ALTITUDE 0.1f
#define EKF_DEFAULT_Q_VARIO 0.5f
#define EKF_DEFAULT_Q_ACCEL_BIAS 0.001f
#define EKF_DEFAULT_R_ALTITUDE 0.5f
#define EKF_DEFAULT_REFERENCE_PA 101325.0f
#define EKF_BARO_FORMULA_COEFF 44330.0f
#define EKF_BARO_FORMULA_EXP 0.1903f
#define EKF_BARO_INV_EXP 5.255f
#define EKF_INNOVATION_GATE_SIGMA 5.0f
#define EKF_ALTITUDE_MIN_M (-500.0f)
#define EKF_ALTITUDE_MAX_M 10000.0f
#define EKF_PRESSURE_MIN_PA 20000.0f
#define EKF_PRESSURE_MAX_PA 120000.0f

    /**
     * @brief EKF tuning configuration.
     */
    typedef struct ekf_cfg_s
    {
        float q_altitude;
        float q_vario;
        float q_accel_bias;
        float r_altitude;
        float reference_pressure_pa;
    } ekf_cfg_t;

    /**
     * @brief EKF internal state vector and covariance.
     */
    typedef struct ekf_state_s
    {
        float altitude_m;
        float vario_ms;
        float accel_bias_ms2;
        float p[3][3];
        int64_t last_predict_us;
        int64_t last_baro_us;
        bool initialized;
    } ekf_state_t;

    /**
     * @brief Initialize EKF state to zero with identity-scaled covariance.
     *
     * @param[out] state  EKF state to initialize.
     * @param[in]  cfg    Configuration with process/measurement noise parameters.
     * @return ESP_OK on success, ESP_ERR_INVALID_ARG if any pointer is NULL.
     */
    esp_err_t ekf_init(ekf_state_t *state, const ekf_cfg_t *cfg);

    /**
     * @brief Propagate the state estimate forward using vertical acceleration.
     *
     * @param[in,out] state              EKF state.
     * @param[in]     cfg                Configuration.
     * @param[in]     vertical_accel_ms2 Vertical acceleration in m/s² (positive = up).
     * @param[in]     timestamp_us       Current timestamp in microseconds.
     * @return ESP_OK on success, ESP_ERR_INVALID_ARG if pointers are NULL,
     *         ESP_ERR_INVALID_STATE if not initialized.
     */
    esp_err_t ekf_predict(ekf_state_t *state, const ekf_cfg_t *cfg, float vertical_accel_ms2, int64_t timestamp_us);

    /**
     * @brief Update the state estimate with a barometric pressure measurement.
     *
     * Converts pressure to altitude and applies innovation gating before fusing.
     *
     * @param[in,out] state        EKF state.
     * @param[in]     cfg          Configuration.
     * @param[in]     pressure_pa  Barometric pressure in Pascals.
     * @param[in]     timestamp_us Current timestamp in microseconds.
     * @return ESP_OK on success, ESP_ERR_INVALID_ARG if pointers are NULL.
     */
    esp_err_t ekf_update_baro(ekf_state_t *state, const ekf_cfg_t *cfg, float pressure_pa, int64_t timestamp_us);

    /**
     * @brief Reset EKF state to initial conditions (zeroed state, uninitialized).
     *
     * @param[out] state EKF state to reset.
     * @return ESP_OK on success, ESP_ERR_INVALID_ARG if state is NULL.
     */
    esp_err_t ekf_reset(ekf_state_t *state);

    /**
     * @brief Calibrate reference pressure from a known altitude and current pressure.
     *
     * Resets filter state after calibration.
     *
     * @param[in,out] cfg                  Configuration (reference_pressure_pa is updated).
     * @param[in,out] state                EKF state (reset after calibration).
     * @param[in]     known_altitude_m     Known altitude at the current location.
     * @param[in]     current_pressure_pa  Current barometric pressure in Pascals.
     * @return ESP_OK on success, ESP_ERR_INVALID_ARG if pointers are NULL or values out of range.
     */
    esp_err_t ekf_calibrate(ekf_cfg_t *cfg, ekf_state_t *state, float known_altitude_m, float current_pressure_pa);

    /**
     * @brief Convert barometric pressure to altitude using the hypsometric formula.
     *
     * Pure function with no side effects.
     *
     * @param[in] pressure_pa            Measured pressure in Pascals.
     * @param[in] reference_pressure_pa  Sea-level reference pressure in Pascals.
     * @return Altitude in meters.
     */
    float ekf_pressure_to_altitude(float pressure_pa, float reference_pressure_pa);

#ifdef __cplusplus
}
#endif

#endif // EKF_H
