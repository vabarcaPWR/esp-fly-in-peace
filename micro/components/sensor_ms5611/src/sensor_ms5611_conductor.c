#include "sensor_ms5611.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "sensor_ms5611_hardware.h"
#include "sensor_ms5611_model.h"

static const char *TAG = "ms5611";

esp_err_t sensor_ms5611_init(sensor_ms5611_t *self, const sensor_ms5611_cfg_t *cfg)
{
    esp_err_t ret = sensor_ms5611_model_validate_init_args(self, cfg);
    if (ESP_OK != ret)
        return ret;

    ret = sensor_ms5611_hardware_init(self, cfg);
    if (ESP_OK != ret)
        return ret;

    ret = sensor_ms5611_hardware_reset_and_read_prom(self);
    if (ESP_OK != ret)
    {
        sensor_ms5611_hardware_deinit(self);
        return ret;
    }

    ESP_LOGI(TAG, "MS5611 ready (OSR=%u)", self->osr_index);
    return ESP_OK;
}

esp_err_t sensor_ms5611_read(sensor_ms5611_t *self, sensor_data_t *out)
{
    if (!self || !out)
        return ESP_ERR_INVALID_ARG;

    uint32_t d1, d2;
    esp_err_t ret = sensor_ms5611_hardware_read_raw(self, &d1, &d2);
    if (ESP_OK != ret)
        return ret;

    sensor_ms5611_model_compensate(self->calibration, d1, d2, &out->pressure_pa, &out->temperature_mc);
    out->timestamp_us = esp_timer_get_time();
    return ESP_OK;
}

esp_err_t sensor_ms5611_deinit(sensor_ms5611_t *self)
{
    if (!self)
        return ESP_ERR_INVALID_ARG;
    return sensor_ms5611_hardware_deinit(self);
}
