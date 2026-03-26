#include "sensor.h"
#include "ms5611.h"

#include <string.h>

const sensor_baro_t *get_baro_sensor(const char *sensor_name)
{
    if (!sensor_name)
        return NULL;

    if (!strcmp(sensor_name, "ms5611"))
        return get_ms5611_sensor();

    return NULL;
}

const sensor_imu_t *get_imu_sensor(const char *sensor_name)
{
    if (!sensor_name)
        return NULL;

    if (!strcmp(sensor_name, "MPU6050"))
        return get_mpu6050_sensor();

    return NULL;
}
