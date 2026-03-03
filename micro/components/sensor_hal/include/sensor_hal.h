#ifndef SENSOR_HAL_H
#define SENSOR_HAL_H

#include <stdint.h>

#include "driver/i2c_master.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /** @brief Sensor output data common to all drivers. */
    typedef struct sensor_data_s
    {
        int32_t pressure_pa;    /**< Pressure in Pascals (e.g. 101325). */
        int32_t temperature_mc; /**< Temperature in milli-Celsius (e.g. 23500 = 23.5 °C). */
        int64_t timestamp_us;   /**< Microsecond timestamp from esp_timer_get_time(). */
    } sensor_data_t;

    /**
     * @brief Initialize the selected sensor driver and the I2C bus.
     * @return ESP_OK on success, or an error code.
     */
    esp_err_t sensor_hal_init(void);

    /**
     * @brief Read compensated pressure and temperature from the sensor.
     *
     * Blocks for the sensor's conversion time (~10-20 ms depending on driver/OSR).
     *
     * @param[out] out  Populated with compensated values on success; unchanged on error.
     * @return ESP_OK on success, or an error code.
     */
    esp_err_t sensor_hal_read(sensor_data_t *out);

    /**
     * @brief Return a human-readable name of the active sensor driver.
     * @return Pointer to a static string, e.g. "MS5611" or "BMP390".
     */
    const char *sensor_hal_get_name(void);

    /**
     * @brief Return the I2C master bus handle managed by the HAL.
     *
     * Use this handle to add sensor devices via i2c_master_bus_add_device().
     * Only valid after sensor_hal_init() returns ESP_OK.
     *
     * @return I2C bus handle, or NULL if not initialized.
     */
    i2c_master_bus_handle_t sensor_hal_get_i2c_bus_handle(void);

#ifdef __cplusplus
}
#endif

#endif // SENSOR_HAL_H
