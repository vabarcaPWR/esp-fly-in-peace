#include "mpu6050.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static esp_err_t mpu6050_init(void)
{
    return ESP_OK;
}

static esp_err_t mpu6050_read(data_imu_t *out)
{
    if (!out)
        return ESP_ERR_INVALID_ARG;

    return ESP_OK;
}

static const char *mpu6050_get_name(void)
{
    return "MPU6050";
}

const sensor_imu_t *get_mpu6050_sensor(void)
{
    static const sensor_imu_t sensor = {
        .init = mpu6050_init,
        .read = mpu6050_read,
        .get_name = mpu6050_get_name,
    };

    return &sensor;
}
