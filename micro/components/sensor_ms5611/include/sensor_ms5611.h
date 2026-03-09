#ifndef SENSOR_MS5611_H
#define SENSOR_MS5611_H

#include <stdint.h>

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "sensor_hal.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /** @brief OSR index constants for MS5611 ADC oversampling ratio. */
#define SENSOR_MS5611_OSR_256 0U
#define SENSOR_MS5611_OSR_512 1U
#define SENSOR_MS5611_OSR_1024 2U
#define SENSOR_MS5611_OSR_2048 3U
#define SENSOR_MS5611_OSR_4096 4U

    /** @brief MS5611 driver instance. Must be zero-initialised before use. */
    typedef struct sensor_ms5611_s
    {
        uint16_t calibration[6];     /**< PROM calibration coefficients C1–C6. */
        uint8_t osr_index;           /**< Oversampling ratio index 0–4. */
        i2c_master_dev_handle_t dev; /**< ESP-IDF I2C device handle. */
    } sensor_ms5611_t;

    /** @brief Configuration passed to sensor_ms5611_init(). */
    typedef struct sensor_ms5611_cfg_s
    {
        i2c_master_bus_handle_t bus_handle; /**< I2C master bus handle from sensor_hal. */
        uint8_t i2c_addr;                   /**< Device I2C address (0x77 or 0x76). */
        uint8_t osr_index;                  /**< OSR index; use SENSOR_MS5611_OSR_* constants. */
    } sensor_ms5611_cfg_t;

    /**
     * @brief Initialise the MS5611 driver: register I2C device, reset sensor, read and
     *        validate PROM calibration coefficients.
     *
     * @param self  Driver instance. Must not be NULL.
     * @param cfg   Configuration. Must not be NULL.
     * @return ESP_OK on success, ESP_ERR_INVALID_ARG if pointers are NULL,
     *         ESP_ERR_INVALID_CRC if PROM CRC fails, or an I2C error code.
     */
    esp_err_t sensor_ms5611_init(sensor_ms5611_t *self, const sensor_ms5611_cfg_t *cfg);

    /**
     * @brief Read compensated pressure and temperature from the MS5611.
     *
     * Performs two ADC conversion cycles (D1 + D2) and applies the datasheet
     * second-order temperature compensation algorithm.
     *
     * @param self  Initialised driver instance. Must not be NULL.
     * @param out   Output data. Must not be NULL.
     * @return ESP_OK on success, or an I2C / argument error code.
     */
    esp_err_t sensor_ms5611_read(sensor_ms5611_t *self, sensor_data_t *out);

#ifdef __cplusplus
}
#endif

#endif // SENSOR_MS5611_H
