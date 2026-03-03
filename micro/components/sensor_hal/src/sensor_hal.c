#include "sensor_hal.h"

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"

#if defined(CONFIG_SENSOR_MS5611)
// #include "sensor_ms5611.h"
#elif defined(CONFIG_SENSOR_BMP390)
// #include "sensor_bmp390.h"
#else
#error "No sensor driver selected. Run idf.py menuconfig → Component config → Sensor driver."
#endif

#define SENSOR_HAL_I2C_TIMEOUT_MS 100

static const char *TAG = "sensor_hal";

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
        .flags.enable_internal_pullup = true,
    };

    esp_err_t ret = i2c_new_master_bus(&bus_cfg, &s_bus_handle);
    if (ESP_OK != ret)
        ESP_LOGE(TAG, "I2C bus init failed: %s", esp_err_to_name(ret));

    return ret;
}

static esp_err_t deinit_i2c_bus(void)
{
    esp_err_t ret = i2c_del_master_bus(s_bus_handle);
    if (ESP_OK != ret)
        ESP_LOGE(TAG, "I2C bus deinit failed: %s", esp_err_to_name(ret));

    s_bus_handle = NULL;
    return ret;
}

esp_err_t sensor_hal_init(void)
{
    if (s_initialized)
        return ESP_ERR_INVALID_STATE;

    esp_err_t ret = init_i2c_bus();
    if (ESP_OK != ret)
        return ret;

    // TODO(Phase 6/7): call sensor_ms5611_init() or sensor_bmp390_init() here
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

    // TODO(Phase 6/7): call sensor_ms5611_read() or sensor_bmp390_read() here
    out->pressure_pa = 0;
    out->temperature_mc = 0;
    out->timestamp_us = esp_timer_get_time();

    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t sensor_hal_deinit(void)
{
    if (!s_initialized)
        return ESP_ERR_INVALID_STATE;

    // TODO(Phase 6/7): call sensor_ms5611_deinit() or sensor_bmp390_deinit() here

    esp_err_t ret = deinit_i2c_bus();
    s_initialized = false;

    ESP_LOGI(TAG, "Sensor HAL deinitialized");
    return ret;
}

const char *sensor_hal_get_name(void)
{
#if defined(CONFIG_SENSOR_MS5611)
    return "MS5611";
#elif defined(CONFIG_SENSOR_BMP390)
    return "BMP390";
#endif
}
