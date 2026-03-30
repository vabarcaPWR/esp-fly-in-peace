#include "i2c_bus.h"

#include <stdbool.h>

#include "esp_log.h"
#include "sdkconfig.h"

#ifndef CONFIG_SENSOR_I2C_INTERNAL_PULLUP
#define CONFIG_SENSOR_I2C_INTERNAL_PULLUP 0
#endif

static const char *TAG = "i2c_bus";

#if __has_include("driver/i2c_master.h")

static bool bus_initialized = false;
static i2c_master_bus_handle_t bus_handle = NULL;

esp_err_t sensor_i2c_bus_init(void)
{
    if (bus_initialized)
        return ESP_OK;

    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = CONFIG_SENSOR_I2C_SCL_GPIO,
        .sda_io_num = CONFIG_SENSOR_I2C_SDA_GPIO,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags.enable_internal_pullup = true,
    };

    esp_err_t result = i2c_new_master_bus(&bus_config, &bus_handle);
    if (result != ESP_OK)
        return result;

    bus_initialized = true;
    ESP_LOGI(TAG, "I2C bus ready (SDA=%d SCL=%d)", CONFIG_SENSOR_I2C_SDA_GPIO, CONFIG_SENSOR_I2C_SCL_GPIO);

    for (uint8_t addr = 0x08; addr < 0x78; addr++)
    {
        if (i2c_master_probe(bus_handle, addr, 50) == ESP_OK)
            ESP_LOGI(TAG, "I2C device found at 0x%02x", addr);
    }

    return ESP_OK;
}

i2c_master_bus_handle_t sensor_i2c_bus_get_handle(void)
{
    return bus_handle;
}

#else

esp_err_t sensor_i2c_bus_init(void)
{
    ESP_LOGE(TAG, "I2C master API unavailable");
    return ESP_ERR_NOT_SUPPORTED;
}

#endif
