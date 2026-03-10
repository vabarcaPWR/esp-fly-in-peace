#include "sensor_ms5611_hardware.h"

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sensor_ms5611_model.h"

#define I2C_TIMEOUT_MS 100
#define MS5611_CMD_RESET 0x1E
#define MS5611_CMD_PROM_READ 0xA0
#define MS5611_CMD_ADC_READ 0x00
#define MS5611_CMD_CONVERT_D1 0x40
#define MS5611_CMD_CONVERT_D2 0x50
#define MS5611_RESET_WAIT_MS 10
#define MS5611_PROM_WORDS 8

static const char *TAG = "ms5611_hw";

static const uint8_t OSR_WAIT_MS[] = {1, 2, 3, 5, 10};

static esp_err_t transmit_with_retry(i2c_master_dev_handle_t dev, const uint8_t *data, size_t len)
{
    esp_err_t ret = i2c_master_transmit(dev, data, len, I2C_TIMEOUT_MS);
    if (ESP_OK == ret)
        return ESP_OK;
    return i2c_master_transmit(dev, data, len, I2C_TIMEOUT_MS);
}

static esp_err_t transmit_receive_with_retry(i2c_master_dev_handle_t dev, const uint8_t *tx, size_t tx_len, uint8_t *rx,
                                             size_t rx_len)
{
    esp_err_t ret = i2c_master_transmit_receive(dev, tx, tx_len, rx, rx_len, I2C_TIMEOUT_MS);
    if (ESP_OK == ret)
        return ESP_OK;
    return i2c_master_transmit_receive(dev, tx, tx_len, rx, rx_len, I2C_TIMEOUT_MS);
}

esp_err_t sensor_ms5611_hardware_init(sensor_ms5611_t *self, const sensor_ms5611_cfg_t *cfg)
{
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = cfg->i2c_addr,
        .scl_speed_hz = 400000,
    };

    esp_err_t ret = i2c_master_bus_add_device(cfg->bus_handle, &dev_cfg, &self->dev);
    if (ESP_OK != ret)
        ESP_LOGE(TAG, "i2c_master_bus_add_device failed: %s", esp_err_to_name(ret));

    self->osr_index = cfg->osr_index;
    return ret;
}

esp_err_t sensor_ms5611_hardware_reset_and_read_prom(sensor_ms5611_t *self)
{
    uint8_t reset_cmd = MS5611_CMD_RESET;
    esp_err_t ret = transmit_with_retry(self->dev, &reset_cmd, 1);
    if (ESP_OK != ret)
    {
        ESP_LOGE(TAG, "reset failed: %s", esp_err_to_name(ret));
        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(MS5611_RESET_WAIT_MS));

    uint16_t prom[MS5611_PROM_WORDS];
    for (int i = 0; i < MS5611_PROM_WORDS; i++)
    {
        uint8_t cmd = (uint8_t)(MS5611_CMD_PROM_READ + i * 2);
        uint8_t buf[2];
        ret = transmit_receive_with_retry(self->dev, &cmd, 1, buf, 2);
        if (ESP_OK != ret)
        {
            ESP_LOGE(TAG, "PROM read W%d failed: %s", i, esp_err_to_name(ret));
            return ret;
        }
        prom[i] = (uint16_t)((buf[0] << 8) | buf[1]);
    }

    uint8_t crc_stored = (uint8_t)(prom[7] & 0x000F);
    uint8_t crc_computed = sensor_ms5611_model_crc4(prom);
    if (crc_stored != crc_computed)
    {
        ESP_LOGE(TAG, "PROM CRC mismatch: stored=0x%X computed=0x%X", crc_stored, crc_computed);
        return ESP_ERR_INVALID_CRC;
    }

    for (int i = 0; i < 6; i++)
        self->calibration[i] = prom[i + 1];

    ESP_LOGI(TAG, "PROM OK: C1=%u C2=%u C3=%u C4=%u C5=%u C6=%u", self->calibration[0], self->calibration[1],
             self->calibration[2], self->calibration[3], self->calibration[4], self->calibration[5]);
    return ESP_OK;
}

static esp_err_t start_conversion_and_wait(sensor_ms5611_t *self, uint8_t base_cmd)
{
    uint8_t cmd = (uint8_t)(base_cmd + self->osr_index * 2);
    esp_err_t ret = transmit_with_retry(self->dev, &cmd, 1);
    if (ESP_OK != ret)
        return ret;

    vTaskDelay(pdMS_TO_TICKS(OSR_WAIT_MS[self->osr_index]));
    return ESP_OK;
}

static esp_err_t read_adc(sensor_ms5611_t *self, uint32_t *result)
{
    uint8_t cmd = MS5611_CMD_ADC_READ;
    uint8_t buf[3];
    esp_err_t ret = transmit_receive_with_retry(self->dev, &cmd, 1, buf, 3);
    if (ESP_OK != ret)
        return ret;

    *result = ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | buf[2];
    return ESP_OK;
}

esp_err_t sensor_ms5611_hardware_read_raw(sensor_ms5611_t *self, uint32_t *d1, uint32_t *d2)
{
    esp_err_t ret = start_conversion_and_wait(self, MS5611_CMD_CONVERT_D1);
    if (ESP_OK != ret)
        return ret;

    ret = read_adc(self, d1);
    if (ESP_OK != ret)
        return ret;

    ret = start_conversion_and_wait(self, MS5611_CMD_CONVERT_D2);
    if (ESP_OK != ret)
        return ret;

    return read_adc(self, d2);
}
