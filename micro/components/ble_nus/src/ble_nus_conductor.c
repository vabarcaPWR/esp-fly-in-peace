#include "ble_nus_conductor.h"

#include <string.h>

#include "ble_nus_hardware.h"
#include "ble_nus_model.h"
#include "esp_log.h"
#include "host/ble_att.h"

#define BLE_NUS_MAX_DEVICE_NAME_LEN 20U
#define BLE_NUS_ATT_OVERHEAD 3U

static const char *TAG = "ble_nus";

esp_err_t ble_nus_conductor_init(const ble_nus_cfg_t *cfg)
{
    if (cfg == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (ble_nus_model_is_initialized())
    {
        return ESP_ERR_INVALID_STATE;
    }

    const char *requested_name = cfg->device_name != NULL ? cfg->device_name : BLE_NUS_DEFAULT_DEVICE_NAME;
    size_t device_name_len = strlen(requested_name);
    if (device_name_len == 0U || device_name_len > BLE_NUS_MAX_DEVICE_NAME_LEN)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint16_t adv_interval_ms = cfg->adv_interval_ms != 0U ? cfg->adv_interval_ms : BLE_NUS_DEFAULT_ADV_INTERVAL_MS;

    esp_err_t model_result = ble_nus_model_init(requested_name, adv_interval_ms, BLE_NUS_DEFAULT_ADV_INTERVAL_MS,
                                                BLE_ATT_MTU_DFLT, BLE_NUS_MAX_DEVICE_NAME_LEN);
    if (model_result != ESP_OK)
    {
        return model_result;
    }

    esp_err_t hardware_result = ble_nus_hardware_start();
    if (hardware_result != ESP_OK)
    {
        ble_nus_model_reset(BLE_NUS_DEFAULT_ADV_INTERVAL_MS, BLE_ATT_MTU_DFLT);
        return hardware_result;
    }

    ble_nus_model_set_initialized(true);
    ESP_LOGI(TAG, "BLE NUS initialized");
    return ESP_OK;
}

esp_err_t ble_nus_conductor_deinit(void)
{
    if (!ble_nus_model_is_initialized())
    {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t stop_result = ble_nus_hardware_stop();
    ble_nus_model_reset(BLE_NUS_DEFAULT_ADV_INTERVAL_MS, BLE_ATT_MTU_DFLT);
    if (stop_result != ESP_OK)
    {
        return stop_result;
    }

    ESP_LOGI(TAG, "BLE NUS deinitialized");
    return ESP_OK;
}

esp_err_t ble_nus_conductor_send(const uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0U)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (!ble_nus_model_is_initialized())
    {
        return ESP_ERR_INVALID_STATE;
    }

    return ble_nus_hardware_send(data, len, BLE_NUS_ATT_OVERHEAD);
}

bool ble_nus_conductor_is_connected(void)
{
    return ble_nus_model_is_connected();
}

void ble_nus_conductor_register_rx_callback(ble_nus_rx_cb_t callback)
{
    ble_nus_model_set_rx_callback(callback);
}

void ble_nus_conductor_register_state_callback(ble_nus_state_cb_t callback)
{
    ble_nus_model_set_state_callback(callback);
}
