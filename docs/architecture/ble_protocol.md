# BLE Protocol Specification — ESP Fly-in-Peace

> Last updated: 2025-07-11

## Overview

The ESP Fly-in-Peace device exposes two BLE GATT services via the `ble_nus` component
(`micro/components/ble_nus/`). This component encapsulates all NimBLE stack management,
GAP advertising, and GATT service registration.

1. **NUS (Nordic UART Service)** — Streams LK8EX1 flight data via notifications (XCTrack compatible)
2. **Config Service** — Device configuration read/write (separate GATT service)

This **hybrid architecture** cleanly separates the real-time data streaming path (NUS) from the device configuration path (Config Service).

## 1. NUS — Nordic UART Service

**Service UUID**: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`

| Characteristic | UUID | Properties | Description |
|---------------|------|------------|-------------|
| NUS RX | `6E400002-B5A3-F393-E0A9-E50E24DCCA9E` | Write | Data to device (client → device) |
| NUS TX | `6E400003-B5A3-F393-E0A9-E50E24DCCA9E` | Notify | LK8EX1 data stream (device → client) |

### NUS TX (Notify)

- Sends LK8EX1 sentences at 4 Hz when a client is subscribed.
- Data encoding: UTF-8 string.
- Each notification contains one complete `$LK8EX1,...*XX\r\n` sentence.
- Notifications are only sent when at least one client has enabled the CCCD (Client Characteristic Configuration Descriptor).

### NUS RX (Write)

- Receives data from the client.
- Data encoding: UTF-8 string.
- Reserved for future use (firmware commands, etc.).
- **Note**: Altitude calibration is handled through the Config Service (see §2 Config Write — Calibrate action), not via NUS RX.

## 2. Config Service

**Service UUID**: `0000ABC0-0000-1000-8000-00805F9B34FB`

| Characteristic | UUID | Properties | Description |
|---------------|------|------------|-------------|
| Device Info | `0000ABC1-0000-1000-8000-00805F9B34FB` | Read | Device info (JSON) |
| Config Read | `0000ABC2-0000-1000-8000-00805F9B34FB` | Read | Current config (JSON) |
| Config Write | `0000ABC3-0000-1000-8000-00805F9B34FB` | Write | Update config (JSON) |

### Device Info (Read)

Returns device information as a JSON string:

```json
{
  "name": "FlyInPeace",
  "fw": "1.0.0",
  "bat": 3700
}
```

| Field | Type | Description |
|-------|------|-------------|
| `name` | string | Device name |
| `fw` | string | Firmware version (semver) |
| `bat` | int | Battery voltage in mV (999 = unknown) |

### Config Read (Read)

Returns current device configuration as a JSON string:

```json
{
  "sensor_rate": 10,
  "ble_tx_rate": 4,
  "kalman_q": 0.01,
  "kalman_r": 0.5,
  "reference_pressure_pa": 101325.0,
  "device_name": "FlyInPeace",
  "wifi_enabled": false
}
```

| Field | Type | Range | Default | Description |
|-------|------|-------|---------|-------------|
| `sensor_rate` | int | 1-100 | 10 | Sensor read rate (Hz) |
| `ble_tx_rate` | int | 1-50 | 4 | BLE send rate (Hz) |
| `kalman_q` | float | 0.001-10.0 | 0.01 | Kalman process noise |
| `kalman_r` | float | 0.01-100.0 | 0.5 | Kalman measurement noise |
| `reference_pressure_pa` | float | 80000.0-120000.0 | 101325.0 | Reference sea-level pressure — QNH (Pa) |
| `device_name` | string | 1-20 chars | "FlyInPeace" | BLE device name |
| `wifi_enabled` | bool | — | false | WiFi service state |

### Config Write (Write)

Accepts a JSON string with configuration updates. Partial updates are supported — only include fields to change:

```json
{
  "sensor_rate": 20,
  "kalman_q": 0.05
}
```

The device validates all fields before applying. Invalid values are rejected and the write returns an error.

#### Calibrate Action

To calibrate the barometric altimeter, the client sends a JSON payload with the `action` field set to `"calibrate"` and the known altitude in meters:

```json
{
  "action": "calibrate",
  "altitude_m": 452.0
}
```

| Field | Type | Range | Description |
|-------|------|-------|-------------|
| `action` | string | `"calibrate"` | Action identifier |
| `altitude_m` | float | -500.0 — 10000.0 | Known altitude at current position (m) |

**Flow**: Config Write → `CONFIG_REQUEST_CALIBRATE` → config_task → calibration_queue → sensor_task → `kalman_filter_calibrate()`. The sensor_task computes the new reference pressure (QNH) using the inverse barometric formula and persists it to NVS.

**Response**: The write returns success (`0x00`) immediately after queuing the calibration request. The actual calibration is applied asynchronously within the next sensor_task cycle (≤ 100 ms). The updated `reference_pressure_pa` value can be verified via Config Read.

## 3. BLE Parameters

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Advertising name | "FlyInPeace" (configurable via NVS) | Descriptive, recognizable |
| Advertising interval | 100–200 ms | Fast discovery, optimizable for power |
| Connection interval | 15–30 ms | Reliable 4 Hz data at low power |
| Slave latency | 0 | No skipping, needed for consistent 4 Hz |
| Supervision timeout | 4000 ms | Allows recovery from brief interference |
| MTU | 256 bytes | Enough for LK8EX1 (~50 bytes) + config JSON |
| TX Power | 0 dBm | Balance range vs power |

## 4. Connection Flow

```
Client (App/XCTrack)                    Device (ESP32-C3)
       │                                       │
       │──── Scan (filter by NUS svc UUID) ───►│
       │◄─── Advertisement (FlyInPeace) ───────│
       │                                       │
       │──── Connect ─────────────────────────►│
       │◄─── Connection established ───────────│
       │                                       │
       │──── MTU exchange (request 256) ──────►│
       │◄─── MTU response ────────────────────│
       │                                       │
       │──── Discover services ───────────────►│
       │◄─── Service list (NUS + Config) ─────│
       │                                       │
       │──── Subscribe to NUS TX (CCCD) ──────►│
       │◄─── LK8EX1 notifications (4 Hz) ─────│
       │◄─── LK8EX1 notifications (4 Hz) ─────│
       │◄─── ... ─────────────────────────────│
       │                                       │
       │──── [Optional] Read Device Info ─────►│  (Config Service)
       │◄─── Device info JSON ────────────────│
       │                                       │
       │──── [Optional] Read Config ──────────►│  (Config Service)
       │◄─── Config JSON ────────────────────│
       │                                       │
       │──── [Optional] Write Config ─────────►│  (Config Service)
       │◄─── Write ack ──────────────────────│
       │                                       │
       │──── [Optional] Calibrate altitude ───►│  (Config Write + action)
       │◄─── Write ack ──────────────────────│
       │                                       │
       │──── Disconnect ──────────────────────►│
       │◄─── Disconnection event ─────────────│
       │                                       │
       │     (Device restarts advertising)     │
```

## 5. XCTrack Compatibility Notes

- XCTrack connects via BLE and expects NMEA-like sentences via notifications.
- The NUS standard UUIDs (`6E400001-...`) are widely compatible with XCTrack and other flight instruments.
- LK8EX1 is the standard sentence format for external vario sensors in XCTrack.
- XCTrack only uses the NUS service — it ignores the Config Service.

## 6. UUID Summary

| Name | UUID |
|------|------|
| NUS Service | `6E400001-B5A3-F393-E0A9-E50E24DCCA9E` |
| NUS RX (write) | `6E400002-B5A3-F393-E0A9-E50E24DCCA9E` |
| NUS TX (notify) | `6E400003-B5A3-F393-E0A9-E50E24DCCA9E` |
| Config Service | `0000ABC0-0000-1000-8000-00805F9B34FB` |
| Device Info | `0000ABC1-0000-1000-8000-00805F9B34FB` |
| Config Read | `0000ABC2-0000-1000-8000-00805F9B34FB` |
| Config Write | `0000ABC3-0000-1000-8000-00805F9B34FB` |

---

## 7. Firmware C Interface Contract

The `ble_nus` component provides a single public header (`ble_nus.h`) that exposes the
following API. All BLE internals (GATT tables, GAP callbacks, NimBLE host task) are
encapsulated and not exposed.

### 7.1 Types

```c
/// Callback invoked when data is received on NUS RX characteristic
typedef void (*ble_nus_rx_cb_t)(const uint8_t *data, uint16_t len);

/// Callback invoked on BLE connection state changes
typedef void (*ble_nus_state_cb_t)(bool connected, uint16_t conn_handle);

/// BLE NUS initialization configuration
typedef struct ble_nus_cfg_s
{
    const char *device_name;       // Advertised device name (max 20 chars, default: "FlyInPeace")
    uint16_t    adv_interval_ms;   // Advertising interval in ms (default: 100)
} ble_nus_cfg_t;
```

### 7.2 Public API

```c
/// Initialize NimBLE stack, register NUS + Config GATT services, start advertising.
/// Must be called once from app_main() before any other ble_nus function.
esp_err_t ble_nus_init(const ble_nus_cfg_t *cfg);

/// Deinitialize BLE stack. Stops advertising, disconnects clients, frees resources.
esp_err_t ble_nus_deinit(void);

/// Send data via NUS TX notification.
/// Returns ESP_ERR_INVALID_STATE if not connected or CCCD not subscribed.
/// Fragments data if payload exceeds (MTU - 3). Thread-safe.
esp_err_t ble_nus_send(const uint8_t *data, uint16_t len);

/// Returns true if a BLE client is currently connected. Thread-safe (atomic read).
bool ble_nus_is_connected(void);

/// Register callback for NUS RX data (client → device writes).
/// Only one callback supported. Passing NULL unregisters.
void ble_nus_register_rx_callback(ble_nus_rx_cb_t callback);

/// Register callback for BLE connection state changes (connect/disconnect).
/// Only one callback supported. Passing NULL unregisters.
void ble_nus_register_state_callback(ble_nus_state_cb_t callback);
```

### 7.3 Error Codes

| Return Code | Condition |
|-------------|-----------|
| `ESP_OK` | Operation succeeded |
| `ESP_ERR_INVALID_ARG` | NULL pointer for required parameter |
| `ESP_ERR_INVALID_STATE` | `send()` called when not connected or not subscribed |
| `ESP_ERR_NO_MEM` | NimBLE memory pool exhausted |
| `ESP_FAIL` | NimBLE host or GATT registration failure |

### 7.4 Threading Model

- `ble_nus_init()` / `ble_nus_deinit()` — call from `app_main()` only (not thread-safe).
- `ble_nus_send()` — thread-safe, can be called from any FreeRTOS task.
- `ble_nus_is_connected()` — thread-safe (atomic read of `_Atomic bool`).
- Callbacks (`rx_cb`, `state_cb`) are invoked from the NimBLE host task context. Keep handlers short and non-blocking.

### 7.5 Config Service GATT Interface

The Config Service is registered internally by `ble_nus_init()`. It interacts with `config_manager` via a config request queue (see [firmware-architecture.md](firmware-architecture.md) §6.2).

| Operation | Trigger | Internal Action |
|-----------|---------|-----------------|
| Device Info read | Client reads `0xABC1` | Build JSON from fw version + battery |
| Config read | Client reads `0xABC2` | `config_manager_load()` → JSON response |
| Config write | Client writes `0xABC3` | Post `CONFIG_REQUEST_WRITE` to config queue || Calibrate altitude | Client writes `0xABC3` with `action: "calibrate"` | Post `CONFIG_REQUEST_CALIBRATE` to config queue → calibration_queue → sensor_task |
---

*End of BLE Protocol Specification*
