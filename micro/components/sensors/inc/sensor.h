#ifndef SENSOR_H
#define SENSOR_H

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct data_baro_s
    {
        int32_t pressure_pa;
        int32_t temperature_mc;
        int64_t timestamp_us;
    } data_baro_t;

    typedef struct data_imu_s
    {
        float accel_x;
        float accel_y;
        float accel_z;
        float gyro_x;
        float gyro_y;
        float gyro_z;
        int64_t timestamp_us;
    } data_imu_t;

    typedef struct sensor_baro_s
    {
        esp_err_t (*init)(void);
        esp_err_t (*read)(data_baro_t *out);
        const char *(*get_name)(void);
    } sensor_baro_t;

    typedef struct imu_sensor_s
    {
        esp_err_t (*init)(void);
        esp_err_t (*read)(data_imu_t *out);
        const char *(*get_name)(void);
    } sensor_imu_t;

    const sensor_baro_t *get_baro_sensor(const char *sensor_name);
    const sensor_imu_t *get_imu_sensor(const char *sensor_name);

#ifdef __cplusplus
}
#endif

#endif // SENSOR_H
