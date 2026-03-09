#include "sensor_hal.h"

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"

#if defined(CONFIG_SENSOR_MS5611)
#include "sensor_ms5611.h"
#elif defined(CONFIG_SENSOR_BMP390)
// #include "sensor_bmp390.h"
#else
#error "No sensor driver selected. Run idf.py menuconfig → Component config → Sensor driver."
#endif

#define SENSOR_HAL_I2C_TIMEOUT_MS 100

static const char *TAG = "sensor_hal";

#if defined(CONFIG_SENSOR_MS5611)
static sensor_ms5611_t s_ms5611;
#endif

static i2c_master_bus_handle_t s_bus_handle;
static bool s_initialized;

static esp_err_t init_i2c_bus(void)
{
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = CONFIG_SENSOR_I2C_SDA_GPIO,
        .scl_io_num = CONFIG_SENSOR_I2C_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
#ifdef CONFIG_SENSOR_I2C_INTERNAL_PULLUP
        .flags.enable_internal_pullup = true,
#endif
#ifdef CONFIG_SENSOR_I2C_ALLOW_PD
        .flags.allow_pd = true,
#endif
    };

    esp_err_t ret = i2c_new_master_bus(&bus_cfg, &s_bus_handle);
    if (ESP_OK != ret)
        ESP_LOGE(TAG, "I2C bus init failed: %s", esp_err_to_name(ret));

    return ret;
}

esp_err_t sensor_hal_init(void)
{
    if (s_initialized)
        return ESP_ERR_INVALID_STATE;

    esp_err_t ret = init_i2c_bus();
    if (ESP_OK != ret)
        return ret;

#if defined(CONFIG_SENSOR_MS5611)
    sensor_ms5611_cfg_t ms5611_cfg = {
        .bus_handle = s_bus_handle,
        .i2c_addr = CONFIG_SENSOR_I2C_ADDR,
        .osr_index = SENSOR_MS5611_OSR_4096,
    };
    ret = sensor_ms5611_init(&s_ms5611, &ms5611_cfg);
    if (ESP_OK != ret)
        return ret;
#endif

    ESP_LOGI(TAG, "Sensor HAL initialized: %s (I2C addr 0x%02X, SDA=%d, SCL=%d)", sensor_hal_get_name(),
             CONFIG_SENSOR_I2C_ADDR, CONFIG_SENSOR_I2C_SDA_GPIO, CONFIG_SENSOR_I2C_SCL_GPIO);

    s_initialized = true;
    return ESP_OK;
}

esp_err_t sensor_hal_read(sensor_data_t *out)
{
    if (!out)
        return ESP_ERR_INVALID_ARG;

    if (!s_initialized)
        return ESP_ERR_INVALID_STATE;

#if defined(CONFIG_SENSOR_MS5611)
    return sensor_ms5611_read(&s_ms5611, out);
#else
    out->pressure_pa = 0;
    out->temperature_mc = 0;
    out->timestamp_us = esp_timer_get_time();
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

i2c_master_bus_handle_t sensor_hal_get_i2c_bus_handle(void)
{
    return s_bus_handle;
}

const char *sensor_hal_get_name(void)
{
#if defined(CONFIG_SENSOR_MS5611)
    return "MS5611";
#elif defined(CONFIG_SENSOR_BMP390)
    return "BMP390";
#endif
}
