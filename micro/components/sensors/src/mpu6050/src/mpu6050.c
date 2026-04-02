#include "mpu6050.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_bus.h"

#if __has_include("driver/i2c_master.h")
#include "driver/i2c_master.h"
#define MPU6050_HAS_I2C_MASTER_API 1
#else
#define MPU6050_HAS_I2C_MASTER_API 0
#endif

#define MPU6050_I2C_ADDR 0x68
#define MPU6050_I2C_FREQ_HZ 400000
#define MPU6050_WHO_AM_I_REG 0x75
#define MPU6050_WHO_AM_I_EXPECTED 0x68
#define MPU6050_PWR_MGMT_1_REG 0x6B
#define MPU6050_SMPLRT_DIV_REG 0x19
#define MPU6050_CONFIG_REG 0x1A
#define MPU6050_GYRO_CONFIG_REG 0x1B
#define MPU6050_ACCEL_CONFIG_REG 0x1C
#define MPU6050_ACCEL_XOUT_H_REG 0x3B
#define MPU6050_BURST_READ_LEN 14U
#define MPU6050_ACCEL_SCALE (9.80665f / 16384.0f)
#define MPU6050_GYRO_SCALE (1.0f / 131.0f * 0.0174533f)
#define MPU6050_I2C_TIMEOUT_MS 100
#define MPU6050_WAKEUP_DELAY_MS 100U

static const char *TAG = "mpu6050";

typedef struct mpu6050_context_s
{
    bool i2c_ready;
    bool initialized;
#if MPU6050_HAS_I2C_MASTER_API
    i2c_master_dev_handle_t i2c_dev_handle;
#endif
} mpu6050_context_t;

static mpu6050_context_t s_self = {
    .i2c_ready = false,
    .initialized = false,
#if MPU6050_HAS_I2C_MASTER_API
    .i2c_dev_handle = NULL,
#endif
};

#if MPU6050_HAS_I2C_MASTER_API

static esp_err_t mpu6050_write_register(uint8_t reg, uint8_t value)
{
    if (!s_self.i2c_dev_handle)
        return ESP_ERR_INVALID_STATE;

    uint8_t buf[2] = {reg, value};
    return i2c_master_transmit(s_self.i2c_dev_handle, buf, sizeof(buf), MPU6050_I2C_TIMEOUT_MS);
}

static esp_err_t mpu6050_read_register(uint8_t reg, uint8_t *data, size_t len)
{
    if (!s_self.i2c_dev_handle || !data)
        return ESP_ERR_INVALID_STATE;

    return i2c_master_transmit_receive(s_self.i2c_dev_handle, &reg, 1, data, len, MPU6050_I2C_TIMEOUT_MS);
}

static esp_err_t mpu6050_i2c_init(void)
{
    i2c_master_bus_handle_t bus = sensor_i2c_bus_get_handle();
    if (!bus)
        return ESP_ERR_INVALID_STATE;

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = MPU6050_I2C_ADDR,
        .scl_speed_hz = MPU6050_I2C_FREQ_HZ,
    };

    return i2c_master_bus_add_device(bus, &dev_config, &s_self.i2c_dev_handle);
}

static int16_t mpu6050_raw_from_bytes(uint8_t high, uint8_t low)
{
    return (int16_t)((uint16_t)high << 8U | low);
}

static esp_err_t mpu6050_init(void)
{
    if (s_self.initialized)
        return ESP_OK;

    esp_err_t result = mpu6050_i2c_init();
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C init failed: 0x%x", result);
        return result;
    }

    s_self.i2c_ready = true;

    uint8_t who_am_i = 0;
    result = mpu6050_read_register(MPU6050_WHO_AM_I_REG, &who_am_i, 1);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "WHO_AM_I read failed: 0x%x", result);
        return result;
    }

    if (who_am_i != MPU6050_WHO_AM_I_EXPECTED)
    {
        ESP_LOGE(TAG, "WHO_AM_I mismatch: got 0x%02x, expected 0x%02x", who_am_i, MPU6050_WHO_AM_I_EXPECTED);
        return ESP_ERR_NOT_FOUND;
    }

    result = mpu6050_write_register(MPU6050_PWR_MGMT_1_REG, 0x00);
    if (result != ESP_OK)
        return result;

    vTaskDelay(pdMS_TO_TICKS(MPU6050_WAKEUP_DELAY_MS));

    result = mpu6050_write_register(MPU6050_SMPLRT_DIV_REG, 9);
    if (result != ESP_OK)
        return result;

    result = mpu6050_write_register(MPU6050_CONFIG_REG, 0x03);
    if (result != ESP_OK)
        return result;

    result = mpu6050_write_register(MPU6050_GYRO_CONFIG_REG, 0x00);
    if (result != ESP_OK)
        return result;

    result = mpu6050_write_register(MPU6050_ACCEL_CONFIG_REG, 0x00);
    if (result != ESP_OK)
        return result;

    s_self.initialized = true;
    ESP_LOGI(TAG, "MPU6050 initialized (WHO_AM_I=0x%02x, addr=0x%02x)", who_am_i, MPU6050_I2C_ADDR);
    return ESP_OK;
}

static esp_err_t mpu6050_read(data_imu_t *out)
{
    if (!out)
        return ESP_ERR_INVALID_ARG;

    if (!s_self.initialized)
        return ESP_ERR_INVALID_STATE;

    uint8_t raw[MPU6050_BURST_READ_LEN];
    esp_err_t result = mpu6050_read_register(MPU6050_ACCEL_XOUT_H_REG, raw, MPU6050_BURST_READ_LEN);
    if (result != ESP_OK)
        return result;

    out->accel_x = mpu6050_raw_from_bytes(raw[0], raw[1]) * MPU6050_ACCEL_SCALE;
    out->accel_y = mpu6050_raw_from_bytes(raw[2], raw[3]) * MPU6050_ACCEL_SCALE;
    out->accel_z = mpu6050_raw_from_bytes(raw[4], raw[5]) * MPU6050_ACCEL_SCALE;
    out->gyro_x = mpu6050_raw_from_bytes(raw[8], raw[9]) * MPU6050_GYRO_SCALE;
    out->gyro_y = mpu6050_raw_from_bytes(raw[10], raw[11]) * MPU6050_GYRO_SCALE;
    out->gyro_z = mpu6050_raw_from_bytes(raw[12], raw[13]) * MPU6050_GYRO_SCALE;
    out->timestamp_us = esp_timer_get_time();
    return ESP_OK;
}

#endif

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
