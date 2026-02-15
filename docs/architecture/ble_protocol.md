# BLE Protocol Specification — ESP Fly-in-Peace

> Last updated: 2026-02-15

## Overview

The ESP Fly-in-Peace device exposes two BLE GATT services:

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
- Reserved for future use (firmware commands, calibration triggers, etc.).

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

*End of BLE Protocol Specification*
