# BLE Protocol Specification — ESP Fly-in-Peace

> Last updated: 2026-02-13

## Overview

The ESP Fly-in-Peace device exposes two BLE GATT services:

1. **SPP Service** — Serial Port Profile emulation for streaming LK8EX1 flight data (XCTrack compatible)
2. **Config Service** — Device configuration read/write

## 1. SPP Service

**Service UUID**: `0000abf0-0000-1000-8000-00805f9b34fb`

| Characteristic | UUID | Properties | Description |
|---------------|------|------------|-------------|
| SPP TX | `0000abf1-0000-1000-8000-00805f9b34fb` | Notify | LK8EX1 data stream (device → client) |
| SPP RX | `0000abf2-0000-1000-8000-00805f9b34fb` | Write | Commands to device (client → device) |

### SPP TX (Notify)

- Sends LK8EX1 sentences at 4 Hz when a client is subscribed.
- Data encoding: UTF-8 string.
- Each notification contains one complete `$LK8EX1,...*XX\r\n` sentence.
- Notifications are only sent when at least one client has enabled the CCCD (Client Characteristic Configuration Descriptor).

### SPP RX (Write)

- Receives commands from the client.
- Data encoding: UTF-8 string.
- Reserved for future use (firmware commands, calibration triggers, etc.).

## 2. Config Service

**Service UUID**: `0000abf1-0000-1000-8000-00805f9b34fb`

> **Note**: The Config Service UUID and SPP TX UUID share the same value (`0000abf1-...`). This is intentional in the current placeholder spec but **must be reviewed** before implementation to avoid ambiguity. Consider changing Config Service UUID to `0000abc0-0000-1000-8000-00805f9b34fb` or similar.

| Characteristic | UUID | Properties | Description |
|---------------|------|------------|-------------|
| Device Info | `0000abf3-0000-1000-8000-00805f9b34fb` | Read | Device info (JSON) |
| Config Read | `0000abf4-0000-1000-8000-00805f9b34fb` | Read | Current config (JSON) |
| Config Write | `0000abf5-0000-1000-8000-00805f9b34fb` | Write | Update config (JSON) |

### Device Info (Read)

Returns device information as a JSON string:

```json
{
  "name": "EFIP",
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
  "device_name": "EFIP",
  "wifi_enabled": false
}
```

| Field | Type | Range | Default | Description |
|-------|------|-------|---------|-------------|
| `sensor_rate` | int | 1-100 | 10 | Sensor read rate (Hz) |
| `ble_tx_rate` | int | 1-50 | 4 | BLE send rate (Hz) |
| `kalman_q` | float | 0.001-10.0 | 0.01 | Kalman process noise |
| `kalman_r` | float | 0.01-100.0 | 0.5 | Kalman measurement noise |
| `device_name` | string | 1-20 chars | "EFIP" | BLE device name |
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

## 3. BLE Parameters

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Advertising name | "EFIP" (configurable) | Short, recognizable |
| Advertising interval | 1000 ms | Low power when not connected |
| Connection interval | 30-50 ms | Fast enough for 4 Hz data |
| Slave latency | 0 | No skipping, needed for consistent 4 Hz |
| Supervision timeout | 4000 ms | Allows recovery from brief interference |
| MTU | 256 bytes | Enough for LK8EX1 (~50 bytes) + config JSON |
| TX Power | 0 dBm | Balance range vs power |

## 4. Connection Flow

```
Client (App/XCTrack)                    Device (ESP32-C3)
       │                                       │
       │──── Scan (filter by SPP svc UUID) ───►│
       │◄─── Advertisement (EFIP) ─────────────│
       │                                       │
       │──── Connect ─────────────────────────►│
       │◄─── Connection established ───────────│
       │                                       │
       │──── MTU exchange (request 256) ──────►│
       │◄─── MTU response ────────────────────│
       │                                       │
       │──── Discover services ───────────────►│
       │◄─── Service list (SPP + Config) ─────│
       │                                       │
       │──── Subscribe to SPP TX (CCCD) ──────►│
       │◄─── LK8EX1 notifications (4 Hz) ─────│
       │◄─── LK8EX1 notifications (4 Hz) ─────│
       │◄─── ... ─────────────────────────────│
       │                                       │
       │──── [Optional] Read Device Info ─────►│
       │◄─── Device info JSON ────────────────│
       │                                       │
       │──── [Optional] Read Config ──────────►│
       │◄─── Config JSON ────────────────────│
       │                                       │
       │──── [Optional] Write Config ─────────►│
       │◄─── Write ack ──────────────────────│
       │                                       │
       │──── Disconnect ──────────────────────►│
       │◄─── Disconnection event ─────────────│
       │                                       │
       │     (Device restarts advertising)     │
```

## 5. XCTrack Compatibility Notes

- XCTrack uses BLE SPP (Serial Port Profile emulation over GATT).
- XCTrack expects to receive NMEA-like sentences via BLE notifications.
- The SPP service UUID and characteristic UUIDs may need adjustment based on XCTrack's specific expectations — verify with XCTrack documentation or community forums.
- LK8EX1 is the standard sentence format for external vario sensors in XCTrack.

## 6. UUID Summary

| Name | UUID |
|------|------|
| SPP Service | `0000abf0-0000-1000-8000-00805f9b34fb` |
| SPP TX | `0000abf1-0000-1000-8000-00805f9b34fb` |
| SPP RX | `0000abf2-0000-1000-8000-00805f9b34fb` |
| Config Service | `0000abf1-0000-1000-8000-00805f9b34fb` (**review needed**) |
| Device Info | `0000abf3-0000-1000-8000-00805f9b34fb` |
| Config Read | `0000abf4-0000-1000-8000-00805f9b34fb` |
| Config Write | `0000abf5-0000-1000-8000-00805f9b34fb` |

> **Action item**: Resolve the UUID collision between SPP TX and Config Service before implementation.

---

*End of BLE Protocol Specification*
