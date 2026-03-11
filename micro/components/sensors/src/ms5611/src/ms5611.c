#include "ms5611.h"
#include <stddef.h>

esp_err_t ms5611_init(void)
{
    return ESP_OK;
}

esp_err_t ms5611_read(baro_data_t *out)
{
    return ESP_OK;
}

const char *ms5611_get_name(void)
{
    return "MS5611";
}

const baro_sensor_t *get_ms5611_sensor(void)
{
    static baro_sensor_t sensor = {.get_name = ms5611_get_name, .init = ms5611_init, .read = ms5611_read};
    return &sensor;
}
