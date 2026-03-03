#ifndef BLE_NUS_HARDWARE_H
#define BLE_NUS_HARDWARE_H

#include <stdint.h>

#include "esp_err.h"

esp_err_t ble_nus_hardware_start(void);
esp_err_t ble_nus_hardware_send(const uint8_t *data, uint16_t len, uint16_t att_overhead);

#endif
