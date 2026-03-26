#include "mpu6050.h"

#include <stdbool.h>

static bool mpu6050_fake_initialized = false;

static esp_err_t mpu6050_fake_init(void)
{
    mpu6050_fake_initialized = true;
    return ESP_OK;
}

static esp_err_t mpu6050_fake_read(data_imu_t *out)
{
    if (!out)
        return ESP_ERR_INVALID_ARG;

    if (!mpu6050_fake_initialized)
        return ESP_ERR_INVALID_STATE;

    out->accel_x = 0.0f;
    out->accel_y = 0.0f;
    out->accel_z = -9.80665f;
    out->gyro_x = 0.0f;
    out->gyro_y = 0.0f;
    out->gyro_z = 0.0f;
    out->timestamp_us = 0;
    return ESP_OK;
}

static const char *mpu6050_fake_get_name(void)
{
    return "MPU6050";
}

const sensor_imu_t *get_mpu6050_sensor(void)
{
    static const sensor_imu_t sensor = {
        .init = mpu6050_fake_init,
        .read = mpu6050_fake_read,
        .get_name = mpu6050_fake_get_name,
    };

    return &sensor;
}
