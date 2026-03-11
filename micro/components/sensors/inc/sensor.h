#ifndef SENSOR_H
#define SENSOR_H

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct baro_data_s
    {
        int32_t pressure_pa;
        int32_t temperature_mc;
        int64_t timestamp_us;
    } baro_data_t;

    typedef struct imu_data_s
    {
    } imu_data_t;

    typedef struct baro_sensor_s
    {
        esp_err_t (*init)(void);
        esp_err_t (*read)(baro_data_t *out);
        const char *(*get_name)(void);
    } baro_sensor_t;

    typedef struct imu_sensor_s
    {
        esp_err_t (*init)(void);
        esp_err_t (*read)(imu_data_t *out);
        const char *(*get_name)(void);
    } imu_sensor_t;

    const baro_sensor_t *get_baro_sensor(const char *sensor_name);
    const imu_sensor_t *get_imu_sensor(const char *sensor_name);

#ifdef __cplusplus
}
#endif

#endif // SENSOR_H
