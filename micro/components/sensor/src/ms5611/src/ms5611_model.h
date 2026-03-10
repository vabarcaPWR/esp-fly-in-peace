#ifndef SENSOR_MS5611_MODEL_H
#define SENSOR_MS5611_MODEL_H

#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Apply MS5611 second-order temperature compensation (per datasheet §4.1).
     *
     * Pure computation — no I2C calls. Safe to use on host for unit testing.
     *
     * @param calibration  Six PROM coefficients C1–C6 (must not be NULL).
     * @param d1           Raw 24-bit pressure ADC value.
     * @param d2           Raw 24-bit temperature ADC value.
     * @param pressure_pa  Output compensated pressure in Pascals.
     * @param temperature_mc Output compensated temperature in milli-Celsius
     *                       (e.g. 20070 = 20.07 °C).
     */
    void sensor_ms5611_model_compensate(const uint16_t calibration[6], uint32_t d1, uint32_t d2, int32_t *pressure_pa,
                                        int32_t *temperature_mc);

    /**
     * @brief Compute the 4-bit CRC of the MS5611 PROM (per datasheet §4.0).
     *
     * @param prom  Eight PROM words (W0–W7). Word 7 CRC nibble is cleared internally.
     * @return 4-bit CRC value.
     */
    uint8_t sensor_ms5611_model_crc4(uint16_t prom[8]);

    /**
     * @brief Validate that neither init pointer is NULL.
     *
     * @param self  Pointer to driver instance.
     * @param cfg   Pointer to configuration.
     * @return ESP_OK if both non-NULL, ESP_ERR_INVALID_ARG otherwise.
     */
    esp_err_t sensor_ms5611_model_validate_init_args(const void *self, const void *cfg);

#ifdef __cplusplus
}
#endif

#endif // SENSOR_MS5611_MODEL_H
