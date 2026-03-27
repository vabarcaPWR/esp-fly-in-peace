#include "sensor.h"

#ifdef CONFIG_SENSOR_MS5611
#include "ms5611.h"
#endif

#ifdef CONFIG_IMU_MPU6050
#include "mpu6050.h"
#endif

#include <string.h>

const sensor_baro_t *get_baro_sensor(const char *sensor_name)
{
    if (!sensor_name)
        return NULL;

#ifdef CONFIG_SENSOR_MS5611
    if (!strcmp(sensor_name, "ms5611"))
        return get_ms5611_sensor();
#endif

    return NULL;
}

const sensor_imu_t *get_imu_sensor(const char *sensor_name)
{
    if (!sensor_name)
        return NULL;

#ifdef CONFIG_IMU_MPU6050
    if (!strcmp(sensor_name, "MPU6050"))
        return get_mpu6050_sensor();
#endif

    return NULL;
}
