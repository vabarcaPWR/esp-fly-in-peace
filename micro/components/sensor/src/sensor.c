#include "sensor.h"
#include "ms5611.h"

#include <string.h>

const baro_sensor_t *get_baro_sensor(const char *sensor_name)
{
    if (!sensor_name)
        return NULL;

    if (!strcmp(sensor_name, "ms5611"))
        return get_ms5611_sensor();

    return NULL;
}

const imu_sensor_t *get_imu_sensor(const char *sensor_name)
{
    if (!sensor_name)
        return NULL;

    return NULL;
}
