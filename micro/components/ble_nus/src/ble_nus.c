#include "ble_nus.h"

#include "ble_nus_conductor.h"

esp_err_t ble_nus_init(const ble_nus_cfg_t *cfg)
{
    return ble_nus_conductor_init(cfg);
}

esp_err_t ble_nus_send(const uint8_t *data, uint16_t len)
{
    return ble_nus_conductor_send(data, len);
}

bool ble_nus_is_connected(void)
{
    return ble_nus_conductor_is_connected();
}

void ble_nus_register_rx_callback(ble_nus_rx_cb_t callback)
{
    ble_nus_conductor_register_rx_callback(callback);
}

void ble_nus_register_state_callback(ble_nus_state_cb_t callback)
{
    ble_nus_conductor_register_state_callback(callback);
}
