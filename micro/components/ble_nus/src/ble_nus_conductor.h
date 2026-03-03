#ifndef BLE_NUS_CONDUCTOR_H
#define BLE_NUS_CONDUCTOR_H

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#include "ble_nus.h"

esp_err_t ble_nus_conductor_init(const ble_nus_cfg_t *cfg);
esp_err_t ble_nus_conductor_send(const uint8_t *data, uint16_t len);
bool ble_nus_conductor_is_connected(void);
void ble_nus_conductor_register_rx_callback(ble_nus_rx_cb_t callback);
void ble_nus_conductor_register_state_callback(ble_nus_state_cb_t callback);

#endif
