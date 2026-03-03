#ifndef BLE_NUS_H
#define BLE_NUS_H

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define BLE_NUS_DEFAULT_DEVICE_NAME "FlyInPeace"
#define BLE_NUS_DEFAULT_ADV_INTERVAL_MS 100U

    /** Callback invoked when NUS RX data is written by a client. */
    typedef void (*ble_nus_rx_cb_t)(const uint8_t *data, uint16_t len);

    /** Callback invoked on BLE connection state changes. */
    typedef void (*ble_nus_state_cb_t)(bool connected, uint16_t conn_handle);

    /** BLE NUS initialization configuration. */
    typedef struct ble_nus_cfg_s
    {
        const char *device_name;
        uint16_t adv_interval_ms;
    } ble_nus_cfg_t;

    /** Initialize NimBLE, register NUS service, and start advertising. */
    esp_err_t ble_nus_init(const ble_nus_cfg_t *cfg);

    /** Send payload over NUS TX notifications. */
    esp_err_t ble_nus_send(const uint8_t *data, uint16_t len);

    /** Return true if a BLE client is currently connected. */
    bool ble_nus_is_connected(void);

    /** Register RX callback. Pass NULL to unregister. */
    void ble_nus_register_rx_callback(ble_nus_rx_cb_t callback);

    /** Register connection-state callback. Pass NULL to unregister. */
    void ble_nus_register_state_callback(ble_nus_state_cb_t callback);

#ifdef __cplusplus
}
#endif

#endif
