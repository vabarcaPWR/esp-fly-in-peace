#ifndef BLE_NUS_MODEL_H
#define BLE_NUS_MODEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#include "ble_nus.h"

typedef struct ble_nus_model_snapshot_s
{
    bool initialized;
    bool connected;
    bool notify_enabled;
    uint16_t conn_handle;
    uint16_t tx_value_handle;
    uint16_t mtu;
    uint16_t adv_interval_ms;
    uint8_t own_addr_type;
    ble_nus_rx_cb_t rx_callback;
    ble_nus_state_cb_t state_callback;
} ble_nus_model_snapshot_t;

esp_err_t ble_nus_model_init(const char *device_name, uint16_t adv_interval_ms, uint16_t default_adv_interval_ms,
                             uint16_t default_mtu, size_t max_device_name_len);
void ble_nus_model_reset(uint16_t default_adv_interval_ms, uint16_t default_mtu);

bool ble_nus_model_is_initialized(void);
void ble_nus_model_set_initialized(bool initialized);

bool ble_nus_model_is_connected(void);
const char *ble_nus_model_get_device_name(void);
uint16_t ble_nus_model_get_adv_interval_ms(void);

void ble_nus_model_set_conn_state(bool connected, uint16_t conn_handle, uint16_t default_conn_handle,
                                  uint16_t default_mtu);
void ble_nus_model_set_notify_enabled(bool enabled);
void ble_nus_model_set_mtu(uint16_t mtu);
void ble_nus_model_set_tx_value_handle(uint16_t value_handle);
uint16_t ble_nus_model_get_tx_value_handle(void);
void ble_nus_model_set_own_addr_type(uint8_t own_addr_type);
uint8_t ble_nus_model_get_own_addr_type(void);

void ble_nus_model_set_rx_callback(ble_nus_rx_cb_t callback);
void ble_nus_model_set_state_callback(ble_nus_state_cb_t callback);
ble_nus_rx_cb_t ble_nus_model_get_rx_callback(void);
ble_nus_state_cb_t ble_nus_model_get_state_callback(void);

bool ble_nus_model_get_snapshot(ble_nus_model_snapshot_t *snapshot);

#endif
