#include <sensor.h>
#include <ms5611/ms5611.h>


sensor_t *get_sensor(const char *sensor_name)
{
    if (!strcmp(sensor_name, "ms5611"))
        return get_ms5611_sensor();

    return NULL;
}