#include "ms5611.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#if __has_include("driver/i2c_master.h")
#include "driver/i2c_master.h"
#define MS5611_HAS_I2C_MASTER_API 1
#else
#define MS5611_HAS_I2C_MASTER_API 0
#endif

#ifndef CONFIG_SENSOR_I2C_INTERNAL_PULLUP
#define CONFIG_SENSOR_I2C_INTERNAL_PULLUP 0
#endif

#ifndef CONFIG_SENSOR_I2C_ALLOW_PD
#define CONFIG_SENSOR_I2C_ALLOW_PD 0
#endif

#if MS5611_HAS_I2C_MASTER_API
#define MS5611_I2C_PORT I2C_NUM_0
#else
#define MS5611_I2C_PORT 0
#endif
#define MS5611_RESET_COMMAND 0x1E
#define MS5611_ADC_READ_COMMAND 0x00
#define MS5611_PROM_BASE_COMMAND 0xA0
#define MS5611_CONVERT_D1_OSR_4096 0x48
#define MS5611_CONVERT_D2_OSR_4096 0x58
#define MS5611_RESET_DELAY_MS 3U
#define MS5611_CONVERSION_DELAY_MS 10U

static const char *TAG = "ms5611";

typedef struct ms5611_context_s
{
    bool i2c_ready;
    bool calibration_ready;
#if MS5611_HAS_I2C_MASTER_API
    i2c_master_bus_handle_t i2c_bus_handle;
    i2c_master_dev_handle_t i2c_dev_handle;
#endif
    uint16_t prom[8];
} ms5611_context_t;

static ms5611_context_t ms5611_context = {
    .i2c_ready = false,
    .calibration_ready = false,
#if MS5611_HAS_I2C_MASTER_API
    .i2c_bus_handle = NULL,
    .i2c_dev_handle = NULL,
#endif
    .prom = {0},
};

#if MS5611_HAS_I2C_MASTER_API

static esp_err_t ms5611_i2c_write_command(uint8_t command)
{
    if (!ms5611_context.i2c_dev_handle)
    {
        return ESP_ERR_INVALID_STATE;
    }

    return i2c_master_transmit(ms5611_context.i2c_dev_handle, &command, sizeof(command), 100);
}

static esp_err_t ms5611_i2c_read_prom_word(uint8_t index, uint16_t *word)
{
    if (!word || (index > 7U))
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t command = (uint8_t)(MS5611_PROM_BASE_COMMAND + (index * 2U));
    uint8_t raw[2] = {0};
    if (!ms5611_context.i2c_dev_handle)
    {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t result =
        i2c_master_transmit_receive(ms5611_context.i2c_dev_handle, &command, sizeof(command), raw, sizeof(raw), 100);
    if (result != ESP_OK)
    {
        return result;
    }

    *word = (uint16_t)(((uint16_t)raw[0] << 8U) | raw[1]);
    return ESP_OK;
}

static esp_err_t ms5611_i2c_read_adc_24b(int32_t *value)
{
    if (!value)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t command = MS5611_ADC_READ_COMMAND;
    uint8_t raw[3] = {0};
    if (!ms5611_context.i2c_dev_handle)
    {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t result =
        i2c_master_transmit_receive(ms5611_context.i2c_dev_handle, &command, sizeof(command), raw, sizeof(raw), 100);
    if (result != ESP_OK)
    {
        return result;
    }

    *value = ((int32_t)raw[0] << 16U) | ((int32_t)raw[1] << 8U) | raw[2];
    return ESP_OK;
}

static esp_err_t ms5611_ensure_i2c_ready(void)
{
    if (ms5611_context.i2c_ready)
    {
        return ESP_OK;
    }

    i2c_master_bus_config_t bus_config = {
        .i2c_port = MS5611_I2C_PORT,
        .sda_io_num = CONFIG_SENSOR_I2C_SDA_GPIO,
        .scl_io_num = CONFIG_SENSOR_I2C_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 1,
        .flags.enable_internal_pullup = CONFIG_SENSOR_I2C_INTERNAL_PULLUP,
        .flags.allow_pd = CONFIG_SENSOR_I2C_ALLOW_PD,
    };

    esp_err_t result = i2c_new_master_bus(&bus_config, &ms5611_context.i2c_bus_handle);
    if (result != ESP_OK)
    {
        return result;
    }

    i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = CONFIG_SENSOR_I2C_ADDR,
        .scl_speed_hz = CONFIG_SENSOR_I2C_FREQ_HZ,
        .scl_wait_us = 0,
        .flags.disable_ack_check = 0,
    };

    result = i2c_master_bus_add_device(ms5611_context.i2c_bus_handle, &device_config, &ms5611_context.i2c_dev_handle);
    if (result != ESP_OK)
    {
        return result;
    }

    ms5611_context.i2c_ready = true;
    return ESP_OK;
}

static esp_err_t ms5611_read_calibration_prom(void)
{
    for (uint8_t i = 0; i < 8U; i++)
    {
        esp_err_t result = ms5611_i2c_read_prom_word(i, &ms5611_context.prom[i]);
        if (result != ESP_OK)
        {
            return result;
        }
    }

    bool all_zero = true;
    for (uint8_t i = 1; i <= 6U; i++)
    {
        if (ms5611_context.prom[i])
        {
            all_zero = false;
            break;
        }
    }

    if (all_zero)
    {
        return ESP_FAIL;
    }

    ms5611_context.calibration_ready = true;
    return ESP_OK;
}

static esp_err_t ms5611_start_conversion_and_read(uint8_t conversion_command, int32_t *adc_value)
{
    if (!adc_value)
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t result = ms5611_i2c_write_command(conversion_command);
    if (result != ESP_OK)
    {
        return result;
    }

    vTaskDelay(pdMS_TO_TICKS(MS5611_CONVERSION_DELAY_MS));
    return ms5611_i2c_read_adc_24b(adc_value);
}

static esp_err_t ms5611_compensate(int32_t d1, int32_t d2, int32_t *pressure_pa, int32_t *temperature_mc)
{
    if (!pressure_pa || !temperature_mc)
    {
        return ESP_ERR_INVALID_ARG;
    }

    int64_t c1 = ms5611_context.prom[1];
    int64_t c2 = ms5611_context.prom[2];
    int64_t c3 = ms5611_context.prom[3];
    int64_t c4 = ms5611_context.prom[4];
    int64_t c5 = ms5611_context.prom[5];
    int64_t c6 = ms5611_context.prom[6];

    int64_t d_t = (int64_t)d2 - (c5 << 8U);
    int64_t temp = 2000 + ((d_t * c6) >> 23U);
    int64_t off = (c2 << 16U) + ((c4 * d_t) >> 7U);
    int64_t sens = (c1 << 15U) + ((c3 * d_t) >> 8U);
    int64_t t2 = 0;
    int64_t off2 = 0;
    int64_t sens2 = 0;

    if (temp < 2000)
    {
        int64_t temp_diff = temp - 2000;
        int64_t temp_diff_sq = temp_diff * temp_diff;
        t2 = (d_t * d_t) >> 31U;
        off2 = (5 * temp_diff_sq) >> 1U;
        sens2 = (5 * temp_diff_sq) >> 2U;

        if (temp < -1500)
        {
            int64_t very_low_diff = temp + 1500;
            int64_t very_low_diff_sq = very_low_diff * very_low_diff;
            off2 += 7 * very_low_diff_sq;
            sens2 += (11 * very_low_diff_sq) >> 1U;
        }
    }

    temp -= t2;
    off -= off2;
    sens -= sens2;

    int64_t pressure = (((int64_t)d1 * sens) >> 21U) - off;
    pressure >>= 15U;

    *pressure_pa = (int32_t)pressure;
    *temperature_mc = (int32_t)(temp * 10);
    return ESP_OK;
}

esp_err_t ms5611_init(void)
{
    esp_err_t result = ms5611_ensure_i2c_ready();
    if (result != ESP_OK)
    {
        return result;
    }

    result = ms5611_i2c_write_command(MS5611_RESET_COMMAND);
    if (result != ESP_OK)
    {
        return result;
    }

    vTaskDelay(pdMS_TO_TICKS(MS5611_RESET_DELAY_MS));
    result = ms5611_read_calibration_prom();
    if (result != ESP_OK)
    {
        return result;
    }

    ESP_LOGI(TAG, "PROM C1=%u C2=%u C3=%u C4=%u C5=%u C6=%u", (unsigned int)ms5611_context.prom[1],
             (unsigned int)ms5611_context.prom[2], (unsigned int)ms5611_context.prom[3],
             (unsigned int)ms5611_context.prom[4], (unsigned int)ms5611_context.prom[5],
             (unsigned int)ms5611_context.prom[6]);
    return ESP_OK;
}

esp_err_t ms5611_read(data_baro_t *out)
{
    if (!out)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (!ms5611_context.calibration_ready)
    {
        return ESP_ERR_INVALID_STATE;
    }

    int32_t d1 = 0;
    int32_t d2 = 0;
    esp_err_t result = ms5611_start_conversion_and_read(MS5611_CONVERT_D1_OSR_4096, &d1);
    if (result != ESP_OK)
    {
        return result;
    }

    result = ms5611_start_conversion_and_read(MS5611_CONVERT_D2_OSR_4096, &d2);
    if (result != ESP_OK)
    {
        return result;
    }

    result = ms5611_compensate(d1, d2, &out->pressure_pa, &out->temperature_mc);
    if (result != ESP_OK)
    {
        return result;
    }

    out->timestamp_us = esp_timer_get_time();
    return ESP_OK;
}

#else

static esp_err_t ms5611_ensure_i2c_ready(void)
{
    ms5611_context.i2c_ready = false;
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t ms5611_init(void)
{
    esp_err_t result = ms5611_ensure_i2c_ready();
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "MS5611 I2C driver headers unavailable in this component build");
        return result;
    }

    return ESP_OK;
}

esp_err_t ms5611_read(data_baro_t *out)
{
    if (!out)
    {
        return ESP_ERR_INVALID_ARG;
    }

    return ESP_ERR_INVALID_STATE;
}

#endif

const char *ms5611_get_name(void)
{
    return "MS5611";
}

const sensor_baro_t *get_ms5611_sensor(void)
{
    static sensor_baro_t sensor = {
        .init = ms5611_init,
        .read = ms5611_read,
        .get_name = ms5611_get_name,
    };

    return &sensor;
}
