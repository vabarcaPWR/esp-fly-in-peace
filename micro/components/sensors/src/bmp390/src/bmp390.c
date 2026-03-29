#include "bmp390.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_bus.h"
#include "sdkconfig.h"

#if __has_include("driver/i2c_master.h")
#include "driver/i2c_master.h"
#define BMP390_HAS_I2C_MASTER_API 1
#else
#define BMP390_HAS_I2C_MASTER_API 0
#endif

#define BMP390_CHIP_ID_VALUE 0x60
#define BMP390_REG_CHIP_ID 0x00
#define BMP390_REG_DATA 0x04
#define BMP390_REG_PWR_CTRL 0x1B
#define BMP390_REG_OSR 0x1C
#define BMP390_REG_CALIB_DATA 0x31
#define BMP390_REG_CMD 0x7E

#define BMP390_REG_CONFIG 0x1F

#define BMP390_SOFT_RESET_CMD 0xB6
#define BMP390_FORCED_MODE 0x13
#define BMP390_OSR_P32X_T2X 0x0D
#define BMP390_IIR_BYPASS 0x00
#define BMP390_CALIB_LEN 21U
#define BMP390_DATA_LEN 6U
#define BMP390_RESET_DELAY_MS 5U
#define BMP390_MEAS_DELAY_MS 75U

static const char *TAG = "bmp390";

typedef struct bmp390_context_s
{
    bool i2c_ready;
    bool calibration_ready;
#if BMP390_HAS_I2C_MASTER_API
    i2c_master_dev_handle_t i2c_dev_handle;
#endif
    bmp390_calib_t calib;
} bmp390_context_t;

static bmp390_context_t ctx = {
    .i2c_ready = false,
    .calibration_ready = false,
#if BMP390_HAS_I2C_MASTER_API
    .i2c_dev_handle = NULL,
#endif
};

#if BMP390_HAS_I2C_MASTER_API

static esp_err_t bmp390_i2c_write_reg(uint8_t reg, uint8_t value)
{
    if (!ctx.i2c_dev_handle)
        return ESP_ERR_INVALID_STATE;

    uint8_t buf[2] = {reg, value};
    return i2c_master_transmit(ctx.i2c_dev_handle, buf, sizeof(buf), 100);
}

static esp_err_t bmp390_i2c_read_reg(uint8_t reg, uint8_t *data, size_t len)
{
    if (!data)
        return ESP_ERR_INVALID_ARG;
    if (!ctx.i2c_dev_handle)
        return ESP_ERR_INVALID_STATE;

    return i2c_master_transmit_receive(ctx.i2c_dev_handle, &reg, 1, data, len, 100);
}

static esp_err_t bmp390_ensure_i2c_ready(void)
{
    if (ctx.i2c_ready)
        return ESP_OK;

    i2c_master_bus_handle_t bus = sensor_i2c_bus_get_handle();
    if (!bus)
        return ESP_ERR_INVALID_STATE;

    i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = CONFIG_SENSOR_I2C_ADDR,
        .scl_speed_hz = CONFIG_SENSOR_I2C_FREQ_HZ,
        .scl_wait_us = 0,
        .flags.disable_ack_check = 0,
    };

    esp_err_t result = i2c_master_bus_add_device(bus, &device_config, &ctx.i2c_dev_handle);
    if (result != ESP_OK)
        return result;

    ctx.i2c_ready = true;
    return ESP_OK;
}

static esp_err_t bmp390_verify_chip_id(void)
{
    uint8_t chip_id = 0;
    esp_err_t result = bmp390_i2c_read_reg(BMP390_REG_CHIP_ID, &chip_id, 1);
    if (result != ESP_OK)
        return result;

    if (chip_id != BMP390_CHIP_ID_VALUE)
    {
        ESP_LOGE(TAG, "unexpected chip ID: 0x%02X (expected 0x%02X)", chip_id, BMP390_CHIP_ID_VALUE);
        return ESP_ERR_NOT_FOUND;
    }

    return ESP_OK;
}

static esp_err_t bmp390_soft_reset(void)
{
    esp_err_t result = bmp390_i2c_write_reg(BMP390_REG_CMD, BMP390_SOFT_RESET_CMD);
    if (result != ESP_OK)
        return result;

    vTaskDelay(pdMS_TO_TICKS(BMP390_RESET_DELAY_MS));
    return ESP_OK;
}

static esp_err_t bmp390_read_calibration(void)
{
    uint8_t raw[BMP390_CALIB_LEN] = {0};
    esp_err_t result = bmp390_i2c_read_reg(BMP390_REG_CALIB_DATA, raw, BMP390_CALIB_LEN);
    if (result != ESP_OK)
        return result;

    bmp390_parse_calib(raw, &ctx.calib);
    ctx.calibration_ready = true;
    return ESP_OK;
}

esp_err_t bmp390_init(void)
{
    esp_err_t result = bmp390_ensure_i2c_ready();
    if (result != ESP_OK)
        return result;

    result = bmp390_verify_chip_id();
    if (result != ESP_OK)
        return result;

    result = bmp390_soft_reset();
    if (result != ESP_OK)
        return result;

    result = bmp390_i2c_write_reg(BMP390_REG_OSR, BMP390_OSR_P32X_T2X);
    if (result != ESP_OK)
        return result;

    result = bmp390_i2c_write_reg(BMP390_REG_CONFIG, BMP390_IIR_BYPASS);
    if (result != ESP_OK)
        return result;

    result = bmp390_read_calibration();
    if (result != ESP_OK)
        return result;

    ESP_LOGI(TAG, "par_t1=%.1f par_t2=%.10f par_p5=%.1f par_p6=%.4f", ctx.calib.par_t1, ctx.calib.par_t2,
             ctx.calib.par_p5, ctx.calib.par_p6);
    return ESP_OK;
}

esp_err_t bmp390_read(data_baro_t *out)
{
    if (!out)
        return ESP_ERR_INVALID_ARG;
    if (!ctx.calibration_ready)
        return ESP_ERR_INVALID_STATE;

    esp_err_t result = bmp390_i2c_write_reg(BMP390_REG_PWR_CTRL, BMP390_FORCED_MODE);
    if (result != ESP_OK)
        return result;

    vTaskDelay(pdMS_TO_TICKS(BMP390_MEAS_DELAY_MS));

    uint8_t data[BMP390_DATA_LEN] = {0};
    result = bmp390_i2c_read_reg(BMP390_REG_DATA, data, BMP390_DATA_LEN);
    if (result != ESP_OK)
        return result;

    uint32_t raw_press = (uint32_t)data[0] | ((uint32_t)data[1] << 8U) | ((uint32_t)data[2] << 16U);
    uint32_t raw_temp = (uint32_t)data[3] | ((uint32_t)data[4] << 8U) | ((uint32_t)data[5] << 16U);

    result = bmp390_compensate(raw_press, raw_temp, &ctx.calib, &out->pressure_pa, &out->temperature_mc);
    if (result != ESP_OK)
        return result;

    out->timestamp_us = esp_timer_get_time();
    return ESP_OK;
}

#else

esp_err_t bmp390_init(void)
{
    ESP_LOGE(TAG, "BMP390 I2C driver headers unavailable in this build");
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bmp390_read(data_baro_t *out)
{
    if (!out)
        return ESP_ERR_INVALID_ARG;
    return ESP_ERR_INVALID_STATE;
}

#endif

static const char *bmp390_get_name(void)
{
    return "BMP390";
}

const sensor_baro_t *get_bmp390_sensor(void)
{
    static sensor_baro_t sensor = {
        .init = bmp390_init,
        .read = bmp390_read,
        .get_name = bmp390_get_name,
    };

    return &sensor;
}
