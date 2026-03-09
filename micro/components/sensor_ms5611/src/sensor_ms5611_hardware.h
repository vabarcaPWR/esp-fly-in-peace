#ifndef SENSOR_MS5611_HARDWARE_H
#define SENSOR_MS5611_HARDWARE_H

#include "sensor_ms5611.h"

#ifdef __cplusplus
extern "C"
{
#endif

    esp_err_t sensor_ms5611_hardware_init(sensor_ms5611_t *self, const sensor_ms5611_cfg_t *cfg);
    esp_err_t sensor_ms5611_hardware_reset_and_read_prom(sensor_ms5611_t *self);
    esp_err_t sensor_ms5611_hardware_read_raw(sensor_ms5611_t *self, uint32_t *d1, uint32_t *d2);
    esp_err_t sensor_ms5611_hardware_deinit(sensor_ms5611_t *self);

#ifdef __cplusplus
}
#endif

#endif // SENSOR_MS5611_HARDWARE_H
