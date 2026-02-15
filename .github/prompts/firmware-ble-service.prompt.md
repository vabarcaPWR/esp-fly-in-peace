# BLE Service Implementation

Create or modify a BLE service using NimBLE for the esp-fly-in-peace firmware.

## Input

- Service name: {{SERVICE_NAME}}
- Service UUID: {{UUID}}
- Characteristics:
  {{CHARACTERISTICS_LIST}}
  (format: `name | UUID | properties (R/W/N) | description`)

## Context

- BLE stack: **NimBLE** (not Bluedroid)
- Primary service: **Nordic UART Service (NUS)**
  - Service UUID: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
  - TX characteristic (notify): `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`
  - RX characteristic (write): `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`

## Requirements

1. **GATT service registration**: Register with NimBLE GATT server using `ble_gatts_count_cfg()` and `ble_gatts_add_svcs()`.

2. **Characteristic access callbacks**: Implement `ble_gatt_access_fn` for each characteristic.

3. **Thread safety**: Use FreeRTOS mutex or critical sections for shared data accessed from BLE callbacks and application tasks.

4. **Connection management**:
   - GAP event handler for connect/disconnect/MTU exchange
   - Track connection handle for notifications
   - Log state changes with `ESP_LOGx`

5. **Notification sending**:
   - `esp_err_t ble_nus_send(const uint8_t *data, uint16_t len)` — send via TX notify
   - Check connection state before sending
   - Handle MTU fragmentation if needed

6. **Advertising**:
   - Include NUS service UUID in advertisement
   - Configurable device name (from NVS config)
   - Advertising interval: 100–200 ms (configurable)
   - Auto-restart advertising on disconnect

7. **Apply `.clang-format`** after generating all files.

## Config Service (Separate GATT — Hybrid Architecture)

Device configuration uses a **separate GATT service**, NOT the NUS RX/TX path.

- **Config Service UUID**: `0000ABC0-0000-1000-8000-00805F9B34FB`
- **Device Info** (Read): `0000ABC1-...` — returns JSON `{"name", "fw", "bat"}`
- **Config Read** (Read): `0000ABC2-...` — returns current config as JSON
- **Config Write** (Write): `0000ABC3-...` — accepts partial JSON config updates

See `docs/architecture/ble_protocol.md` for the full specification.
