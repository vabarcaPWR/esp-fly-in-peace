#include "ms5611.h"

static esp_err_t ms5611_fake_init(void)
{
    return ESP_OK;
}

static esp_err_t ms5611_fake_read(data_baro_t *out)
{
    if (!out)
        return ESP_ERR_INVALID_ARG;

    out->pressure_pa = 101325;
    out->temperature_mc = 25000;
    out->timestamp_us = 0;
    return ESP_OK;
}

static const char *ms5611_fake_get_name(void)
{
    return "MS5611";
}

const sensor_baro_t *get_ms5611_sensor(void)
{
    static sensor_baro_t sensor = {
        .init = ms5611_fake_init,
        .read = ms5611_fake_read,
        .get_name = ms5611_fake_get_name,
    };

    return &sensor;
}
