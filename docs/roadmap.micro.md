# Roadmap — Firmware (ESP32-C3)

> **Project**: esp-fly-in-peace  
> **Component**: Firmware (`micro/`)  
> **Target**: ESP32-C3-DevKitC-02 v1.1  
> **Master reference**: `.github/PRE-PROMPT.md`

---

## Summary Checklist

- [x] **Phase 0: Project Bootstrap**
  - [x] Task 0.1: Create ESP-IDF project skeleton
  - [x] Task 0.2: Configure `.clang-format`
  - [x] Task 0.3: Configure Ceedling for host-side unit tests
  - [x] Task 0.4: Create build/flash/test/monitor scripts
  - [x] Task 0.5: Configure `sdkconfig.defaults` for ESP32-C3 + NimBLE
  - [x] Task 0.6: Verify "Hello World" builds, flashes, and runs
  - [x] Task 0.7: Create ESP-IDF environment activation script
- [ ] **Phase 1: Software Architecture Design**
  - [ ] Task 1.1: System architecture document
  - [ ] Task 1.2: Component interface contracts
  - [ ] Task 1.3: FreeRTOS task model and data flow design
- [ ] **Phase 2: BLE NUS Service**
  - [ ] Task 2.1: NimBLE initialization and GAP configuration
  - [ ] Task 2.2: NUS GATT service registration
  - [ ] Task 2.3: TX notification (send data)
  - [ ] Task 2.4: RX write handler (reserved for future use)
  - [ ] Task 2.5: Connection state management
  - [ ] Task 2.6: Verify with nRF Connect
- [ ] **Phase 3: LK8EX1 Protocol**
  - [ ] Task 3.1: LK8EX1 data abstraction layer
  - [ ] Task 3.2: NMEA checksum calculator
  - [ ] Task 3.3: LK8EX1 sentence formatter
  - [ ] Task 3.4: Simulated data provider
  - [ ] Task 3.5: BLE + LK8EX1 integration (send simulated frames)
  - [ ] Task 3.6: Ceedling unit tests
- [ ] **Phase 4: LED Indicator**
  - [ ] Task 4.1: WS2812 driver via RMT peripheral
  - [ ] Task 4.2: LED state machine (red/green/blue patterns)
  - [ ] Task 4.3: Integration with BLE connection state
- [ ] **Phase 5: Hardware Abstraction**
  - [ ] Task 5.1: I2C bus driver wrapper
  - [ ] Task 5.2: Sensor HAL interface definition
- [ ] **Phase 6: MS5611 Sensor Driver**
  - [ ] Task 6.1: MS5611 PROM calibration read
  - [ ] Task 6.2: MS5611 raw pressure & temperature read
  - [ ] Task 6.3: MS5611 compensation math
  - [ ] Task 6.4: Ceedling unit tests for compensation
  - [ ] Task 6.5: Integration test on hardware
- [ ] **Phase 7: Kalman Filter**
  - [ ] Task 7.1: 2-state Kalman filter implementation
  - [ ] Task 7.2: Altitude calculation from pressure
  - [ ] Task 7.3: Vario (vertical speed) derivation
  - [ ] Task 7.4: Ceedling unit tests with synthetic data
- [ ] **Phase 8: Data Pipeline**
  - [ ] Task 8.1: Sensor reader task (10 Hz)
  - [ ] Task 8.2: Data processing task (Kalman + LK8EX1 formatting)
  - [ ] Task 8.3: BLE sender task (4 Hz)
  - [ ] Task 8.4: Inter-task communication (FreeRTOS queues)
  - [ ] Task 8.5: Replace simulated provider with real sensor data
  - [ ] Task 8.6: End-to-end data flow validation
- [ ] **Phase 9: NVS Configuration**
  - [ ] Task 9.1: Config schema definition
  - [ ] Task 9.2: NVS read/write with defaults
  - [ ] Task 9.3: BLE Config Service GATT (read/write characteristics)
  - [ ] Task 9.4: Ceedling unit tests for config parsing
- [ ] **Phase 10: Power Optimization**
  - [ ] Task 10.1: Light-sleep between sensor reads
  - [ ] Task 10.2: BLE connection interval optimization
  - [ ] Task 10.3: Peripheral power gating
  - [ ] Task 10.4: Power consumption measurement & logging
- [ ] **Phase 11: Integration & Validation**
  - [ ] Task 11.1: End-to-end test with XCTrack
  - [ ] Task 11.2: Long-duration stability test (8+ hours)
  - [ ] Task 11.3: Power consumption budget verification
  - [ ] Task 11.4: Edge case testing (BLE disconnect/reconnect, sensor errors)
- [ ] **Phase 12: Documentation & Cleanup**
  - [ ] Task 12.1: Firmware README with build/flash instructions
  - [ ] Task 12.2: Component API documentation
  - [ ] Task 12.3: Architecture diagram update (Mermaid)
  - [ ] Task 12.4: Code review pass (Boy Scout Rule)

---

## Phase 0: Project Bootstrap

**Objective**: Set up a working ESP-IDF project that compiles, flashes, and runs a "Hello World" on the ESP32-C3-DevKitC-02.  
**Estimated Duration**: 2–3 days  
**Dependencies**: None

---

### Task 0.1: Create ESP-IDF project skeleton

**Description**: Initialize the ESP-IDF project structure under `micro/` with the standard CMake layout. Create `main/main.c` with a minimal `app_main()` that logs a startup message.

**Acceptance Criteria**:
- [x] `micro/CMakeLists.txt` exists with correct `cmake_minimum_required` and `project()` calls
- [x] `micro/main/CMakeLists.txt` registers `main.c` as source
- [x] `micro/main/main.c` contains `app_main()` that logs `"esp-fly-in-peace firmware starting"` via `ESP_LOGI`
- [x] `micro/components/` directory exists (empty, ready for components)
- [x] Project compiles with `idf.py build` targeting `esp32c3`

**Validation**:
- Run `idf.py set-target esp32c3 && idf.py build` — build succeeds with 0 errors

**Files to create**:
- `micro/CMakeLists.txt`
- `micro/main/CMakeLists.txt`
- `micro/main/main.c`
- `micro/main/Kconfig.projbuild` (optional, for project-level menuconfig)

**Notes**: Use ESP-IDF v5.x project template. Set target to `esp32c3` in the top-level CMakeLists or via `idf.py set-target`.

---

### Task 0.2: Configure `.clang-format`

**Description**: Create the `.clang-format` file at `micro/.clang-format` with the project's coding style (Allman braces, 4-space indent, 120-char limit, pointer right-aligned).

**Acceptance Criteria**:
- [x] `micro/.clang-format` exists
- [x] Running `clang-format -style=file micro/main/main.c` produces output matching the style guide
- [x] Allman braces, 4-space indent, 120-char line limit, right-aligned pointers

**Validation**:
- Format `main.c` with `clang-format -i -style=file micro/main/main.c` and verify style compliance

**Files to create**:
- `micro/.clang-format`

**Notes**: Base style on `Microsoft` (which uses Allman braces), then customize specific settings per `PRE-PROMPT.md` §4.1.

---

### Task 0.3: Configure Ceedling for host-side unit tests

**Description**: Set up Ceedling in `micro/test/` for host-side unit testing. Configure `project.yml` to find source files in `micro/components/*/src/` and `micro/components/*/include/`.

**Acceptance Criteria**:
- [x] `micro/test/project.yml` exists with correct paths
- [x] `micro/test/support/` directory exists for mock helpers
- [x] Running `ceedling test:all` from `micro/test/` succeeds (even with zero tests)
- [x] A sample test file can be compiled and run

**Validation**:
- Create a trivial test (`test_sample.c`) that passes, run `ceedling test:all`, verify green output

**Files to create**:
- `micro/test/project.yml`
- `micro/test/support/.gitkeep`
- `micro/test/test_sample.c` (trivial, can be deleted after verification)

**Notes**: Ceedling must mock ESP-IDF headers. Create a `support/` directory with mock headers for `esp_err.h`, `esp_log.h` that provide the type definitions without ESP-IDF SDK. This allows testing pure business logic on the host.

---

### Task 0.4: Create build/flash/test/monitor scripts

**Description**: Create bash scripts in `micro/scripts/` for common development workflows. All scripts should accept `-p PORT` for serial port override (default: `/dev/ttyUSB0`).

**Acceptance Criteria**:
- [x] `build.sh` — runs `idf.py build`, exits non-zero on failure
- [x] `flash.sh` — runs `idf.py -p $PORT flash`, supports `-p` flag
- [x] `monitor.sh` — runs `idf.py -p $PORT monitor`, supports `-p` flag
- [x] `test.sh` — runs `cd ../test && ceedling test:all`
- [x] `all.sh` — chains build → flash → monitor
- [x] All scripts are executable (`chmod +x`)
- [x] All scripts print colored status messages (green=success, red=error)

**Validation**:
- Run `./scripts/build.sh` — build succeeds
- Run `./scripts/test.sh` — Ceedling tests pass

**Files to create**:
- `micro/scripts/build.sh`
- `micro/scripts/flash.sh`
- `micro/scripts/monitor.sh`
- `micro/scripts/test.sh`
- `micro/scripts/all.sh`

---

### Task 0.5: Configure `sdkconfig.defaults` for ESP32-C3 + NimBLE

**Description**: Create `sdkconfig.defaults` with optimal settings for the project: NimBLE (not Bluedroid), FreeRTOS tick rate, log level, partition table, flash size.

**Acceptance Criteria**:
- [x] `sdkconfig.defaults` exists at `micro/sdkconfig.defaults`
- [x] NimBLE is enabled, Bluedroid is disabled
- [x] FreeRTOS tick rate is 1000 Hz (1 ms resolution for timing)
- [x] Flash size set to 4 MB (DevKitC-02)
- [x] Log level default set to INFO
- [x] Power management enabled (`CONFIG_PM_ENABLE=y`)
- [x] Building with these defaults succeeds

**Validation**:
- Delete `sdkconfig`, run `idf.py build` — defaults are applied, build succeeds

**Files to create**:
- `micro/sdkconfig.defaults`

**Notes**:
Key config entries:
```
CONFIG_BT_ENABLED=y
CONFIG_BT_NIMBLE_ENABLED=y
CONFIG_BT_BLUEDROID_ENABLED=n 
CONFIG_FREERTOS_HZ=1000
CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y
CONFIG_PM_ENABLE=y
CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_160=y
```

---

### Task 0.6: Verify "Hello World" builds, flashes, and runs

**Description**: End-to-end verification: build the firmware, flash it to the DevKitC-02, and verify the startup log message appears in the serial monitor.

**Acceptance Criteria**:
- [x] `idf.py build` succeeds with 0 errors, 0 warnings (except SDK warnings)
- [x] `idf.py flash` succeeds
- [x] Serial monitor shows `"esp-fly-in-peace firmware starting"` log message
- [x] No crash or reboot loops

**Validation**:
- Run `./scripts/all.sh` — observe startup message in monitor output

**Files to modify**:
- None (verification only)

**Notes**: This is the gate for Phase 0. Do not proceed to Phase 1 until this passes.

---

### Task 0.7: Create ESP-IDF environment activation script

**Description**: Create a sourceable shell script `micro/scripts/env.sh` that activates or deactivates the ESP-IDF environment (`idf.py`, toolchain, Python venv). This allows using the build/flash/monitor scripts from any terminal without manually sourcing ESP-IDF's `export.sh`.

**Usage**:
- Activate: `source ./scripts/env.sh` or `. ./scripts/env.sh`
- Deactivate: `idf_deactivate`

**Acceptance Criteria**:
- [x] `micro/scripts/env.sh` exists and is sourceable (not executable directly)
- [x] Sourcing it activates the ESP-IDF environment (adds `idf.py` to PATH)
- [x] Defines an `idf_deactivate` function that restores the original PATH/environment
- [x] Prints a colored status message indicating activation/deactivation
- [x] Is idempotent — sourcing twice does not duplicate PATH entries
- [x] Detects ESP-IDF installation path automatically or uses `IDF_PATH` if set
- [x] All existing scripts (`build.sh`, `flash.sh`, etc.) work after sourcing

**Validation**:
- Open a fresh terminal, `source ./scripts/env.sh`, run `idf.py --version`
- Run `idf_deactivate`, verify `idf.py` is no longer in PATH
- Source again, run `./scripts/build.sh` — build succeeds

**Files to create**:
- `micro/scripts/env.sh`

**Notes**:
- ESP-IDF v5.5.2 is installed at `~/.espressif/v5.5.2/esp-idf/`.
- The script should source `$IDF_PATH/export.sh` internally.
- Must be sourced (`. env.sh`), not executed (`./env.sh`), to modify the calling shell's environment.

---

## Phase 1: Software Architecture Design

**Objective**: Define the system architecture and component interfaces.  
**Estimated Duration**: 2–3 days  
**Dependencies**: Phase 0 complete

---

### Task 1.1: System architecture document

**Description**: Create a document that outlines the overall system architecture, including tasks, queues, components, and data flow.

**Acceptance Criteria**:
- [ ] Document exists with clear structure and diagrams
- [ ] Covers all components and their interactions
- [ ] Includes timing diagrams and resource allocation

**Validation**:
- Review: architecture is clear and comprehensive

**Files to create**:
- `docs/architecture/firmware-architecture.md`

---

### Task 1.2: Component interface contracts

**Description**: Define the interfaces that all components must implement.

**Acceptance Criteria**:
- [ ] Interface contracts defined in `docs/architecture/ble_protocol.md`
- [ ] Each component has a brief description in its header

**Validation**:
- Review: contracts are clear and sufficient

**Files to create**:
- `docs/architecture/ble_protocol.md`

---

### Task 1.3: FreeRTOS task model and data flow design

**Description**: Define the FreeRTOS task model and data flow.

**Acceptance Criteria**:
- [ ] Task model defined in `docs/architecture/firmware-architecture.md`
- [ ] Data flow diagrams included

**Validation**:
- Review: model is clear and realistic

**Files to create**:
- `docs/architecture/firmware-architecture.md`

---

## Phase 2: BLE NUS Service

**Objective**: Implement Bluetooth Low Energy with NimBLE, advertising the Nordic UART Service, and supporting TX notifications and RX writes.  
**Estimated Duration**: 3–4 days  
**Dependencies**: Phase 0 (NimBLE config in sdkconfig.defaults)

---

### Task 2.1: NimBLE initialization and GAP configuration

**Description**: Create the `ble_nus` component. Initialize the NimBLE host stack and configure GAP parameters (device name, advertising parameters, connection parameters).

**Acceptance Criteria**:
- [ ] Component `ble_nus` created in `micro/components/ble_nus/`
- [ ] NimBLE host stack initialized and running
- [ ] Device name configurable (default: `"FlyInPeace"`)
- [ ] GAP event handler processes: connect, disconnect, MTU exchange, connection update
- [ ] Advertising starts automatically at boot
- [ ] Advertising restarts after disconnect
- [ ] Advertising interval: 100 ms (configurable)
- [ ] Logs connection/disconnection events

**Validation**:
- Flash firmware, scan with nRF Connect on phone — device `"FlyInPeace"` appears

**Files to create**:
- `micro/components/ble_nus/CMakeLists.txt`
- `micro/components/ble_nus/include/ble_nus.h`
- `micro/components/ble_nus/src/ble_nus.c`
- `micro/components/ble_nus/src/ble_nus_types.h`

**Notes**:
- NimBLE requires a host task — use `nimble_port_freertos_init()`.
- Set `BLE_GAP_EVENT_CONNECT`, `BLE_GAP_EVENT_DISCONNECT`, `BLE_GAP_EVENT_MTU` handlers.

---

### Task 2.2: NUS GATT service registration

**Description**: Register the Nordic UART Service (NUS) with NimBLE's GATT server, including TX (notify) and RX (write) characteristics.

**Acceptance Criteria**:
- [ ] NUS service registered with UUID `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- [ ] TX characteristic (UUID `6E400003-...`) with notify property
- [ ] RX characteristic (UUID `6E400002-...`) with write property
- [ ] CCCD (Client Characteristic Configuration Descriptor) for TX notifications
- [ ] Access callback handles read/write/subscribe operations

**Validation**:
- Connect with nRF Connect — NUS service visible with both characteristics

**Files to modify**:
- `micro/components/ble_nus/src/ble_nus.c`

---

### Task 2.3: TX notification (send data)

**Description**: Implement the function to send data via BLE NUS TX characteristic notifications.

**Acceptance Criteria**:
- [ ] Public API: `esp_err_t ble_nus_send(const uint8_t *data, uint16_t len)`
- [ ] Checks if a client is connected and subscribed to notifications
- [ ] Respects MTU size — fragments data if needed
- [ ] Returns `ESP_ERR_INVALID_STATE` if not connected
- [ ] Thread-safe (can be called from any task)

**Validation**:
- Send test string via `ble_nus_send()`, verify receipt in nRF Connect UART terminal

**Files to modify**:
- `micro/components/ble_nus/include/ble_nus.h`
- `micro/components/ble_nus/src/ble_nus.c`

---

### Task 2.4: RX write handler (reserved for future use)

**Description**: Implement the NUS RX characteristic write handler. Reserved for future use (firmware commands, calibration triggers). Config is handled via the separate Config Service GATT.

**Acceptance Criteria**:
- [ ] RX data received via GATT write callback
- [ ] Data passed to a registered callback function (decoupled from BLE internals)
- [ ] API: `ble_nus_register_rx_callback(ble_nus_rx_cb_t callback)`
- [ ] Callback receives `(const uint8_t *data, uint16_t len)`
- [ ] Handles partial writes / fragmented data

**Validation**:
- Send text from nRF Connect UART, verify it arrives in the registered callback

**Files to modify**:
- `micro/components/ble_nus/include/ble_nus.h`
- `micro/components/ble_nus/src/ble_nus.c`

---

### Task 2.5: Connection state management

**Description**: Expose BLE connection state so other components (LED, data pipeline) can react to connections/disconnections.

**Acceptance Criteria**:
- [ ] API: `bool ble_nus_is_connected(void)` — returns current connection state
- [ ] API: `ble_nus_register_state_callback(ble_nus_state_cb_t cb)` — notified on connect/disconnect
- [ ] State callback receives: connected (true/false) + connection handle
- [ ] Thread-safe state access

**Validation**:
- Connect/disconnect from nRF Connect, verify state callbacks fire and `is_connected()` updates

**Files to modify**:
- `micro/components/ble_nus/include/ble_nus.h`
- `micro/components/ble_nus/src/ble_nus.c`

---

### Task 2.6: Verify with nRF Connect

**Description**: Full BLE NUS validation: send simulated LK8EX1 sentences and verify they are received correctly.

**Acceptance Criteria**:
- [ ] Device advertises and is visible in nRF Connect
- [ ] Connection succeeds, NUS service and characteristics visible
- [ ] TX notifications received correctly (full LK8EX1 sentence)
- [ ] RX writes received correctly in firmware callback
- [ ] XCTrack detects the device as a sensor source (if available for testing)
- [ ] Reconnection works after disconnect

**Validation**:
- Test with nRF Connect (mandatory)
- Test with XCTrack (if available — nice to have at this stage)

**Files to modify**:
- `micro/main/main.c` (temporary: send simulated LK8EX1 every 250 ms)

---

## Phase 3: LK8EX1 Protocol

**Objective**: Implement the LK8EX1 NMEA sentence formatter with checksum calculation.  
**Estimated Duration**: 1–2 days  
**Dependencies**: Phase 3 (for understanding the data fields)

---

### Task 3.1: LK8EX1 data abstraction layer

**Description**: Create the `lk8ex1` component that formats flight data into an LK8EX1 NMEA sentence string.

**Acceptance Criteria**:
- [ ] Component `lk8ex1` created in `micro/components/lk8ex1/`
- [ ] Input struct: `lk8ex1_data_t` with fields: `pressure_pa`, `altitude_m` (or 99999), `vario_cm_s`, `temperature_dc` (°C×10), `battery_mv` (or 999)
- [ ] Output function: `lk8ex1_format(data, buffer, buffer_size)` → `esp_err_t`
- [ ] Sentence format: `$LK8EX1,pressure,altitude,vario,temperature,battery*XX\r\n`
- [ ] Checksum: XOR of chars between `$` and `*` (exclusive), 2-digit uppercase hex
- [ ] Buffer size check to prevent overflow
- [ ] Pure C, no ESP-IDF dependencies (testable on host)

**Validation**:
- Unit tests verify correct output for known inputs

**Files to create**:
- `micro/components/lk8ex1/CMakeLists.txt`
- `micro/components/lk8ex1/include/lk8ex1.h`
- `micro/components/lk8ex1/src/lk8ex1.c`

---

### Task 3.2: NMEA checksum calculator

**Description**: Implement a reusable NMEA checksum function (XOR of characters between `$` and `*`).

**Acceptance Criteria**:
- [ ] Function: `uint8_t nmea_checksum(const char *sentence)` — computes XOR checksum
- [ ] Validation function: `bool nmea_validate_checksum(const char *sentence)` — checks existing checksum
- [ ] Handles edge cases: null input, missing `$` or `*`

**Validation**:
- Unit tests with known NMEA sentences

**Files to modify**:
- `micro/components/lk8ex1/src/lk8ex1.c`

**Notes**: The checksum function can also be used by the app-side parser for validation.

---

### Task 3.3: LK8EX1 sentence formatter

**Description**: Create the `lk8ex1` component that formats flight data into an LK8EX1 NMEA sentence string.

**Acceptance Criteria**:
- [ ] Component `lk8ex1` created in `micro/components/lk8ex1/`
- [ ] Input struct: `lk8ex1_data_t` with fields: `pressure_pa`, `altitude_m` (or 99999), `vario_cm_s`, `temperature_dc` (°C×10), `battery_mv` (or 999)
- [ ] Output function: `lk8ex1_format(data, buffer, buffer_size)` → `esp_err_t`
- [ ] Sentence format: `$LK8EX1,pressure,altitude,vario,temperature,battery*XX\r\n`
- [ ] Checksum: XOR of chars between `$` and `*` (exclusive), 2-digit uppercase hex
- [ ] Buffer size check to prevent overflow
- [ ] Pure C, no ESP-IDF dependencies (testable on host)

**Validation**:
- Unit tests verify correct output for known inputs

**Files to create**:
- `micro/components/lk8ex1/CMakeLists.txt`
- `micro/components/lk8ex1/include/lk8ex1.h`
- `micro/components/lk8ex1/src/lk8ex1.c`

---

### Task 3.4: Simulated data provider

**Description**: Provide simulated flight data for testing.

**Acceptance Criteria**:
- [ ] Function to generate random or patterned data
- [ ] Data rate: 10 Hz (matches sensor read rate)
- [ ] Data range: realistic (e.g., 30000–110000 Pa for pressure)
- [ ] Data format: matches LK8EX1 requirements

**Validation**:
- Verify with known test vectors from MS5611 datasheet (Application Note AN520)

**Files to modify**:
- `micro/components/lk8ex1/src/lk8ex1.c`

---

### Task 3.5: BLE + LK8EX1 integration (send simulated frames)

**Description**: Send simulated LK8EX1 frames over BLE.

**Acceptance Criteria**:
- [ ] Function to send simulated frames
- [ ] Frames sent at 4 Hz (matches BLE TX rate)
- [ ] Frames contain valid LK8EX1 data
- [ ] Frames are received correctly

**Validation**:
- Receive LK8EX1 sentences in nRF Connect at ~4 Hz rate

**Files to modify**:
- `micro/main/main.c` (temporary: send simulated LK8EX1 every 250 ms)

---

### Task 3.6: Ceedling unit tests

**Description**: Write unit tests for LK8EX1 formatting and checksum.

**Acceptance Criteria**:
- [ ] Test: format with typical values produces correct sentence
- [ ] Test: format with altitude=99999 (no GPS) works correctly
- [ ] Test: format with battery=999 (no battery readout) works correctly
- [ ] Test: checksum matches manual XOR calculation
- [ ] Test: validate checksum returns true for correct sentence
- [ ] Test: validate checksum returns false for corrupted sentence
- [ ] Test: format with null buffer returns error
- [ ] Test: format with insufficient buffer size returns error
- [ ] All tests pass in `ceedling test:all`

**Validation**:
- Run `./scripts/test.sh` — all tests green

**Files to create**:
- `micro/test/test_lk8ex1.c`

---

## Phase 4: LED Indicator

**Objective**: Drive the onboard WS2812 RGB LED to indicate device state (BLE connected/disconnected, WiFi stub).  
**Estimated Duration**: 1–2 days  
**Dependencies**: Phase 5 (BLE state callbacks)

---

### Task 4.1: WS2812 driver via RMT peripheral

**Description**: Create the `led_indicator` component that drives the WS2812 RGB LED using the ESP32-C3's RMT peripheral.

**Acceptance Criteria**:
- [ ] Component `led_indicator` created in `micro/components/led_indicator/`
- [ ] API: `led_indicator_init()`, `led_indicator_set_color(r, g, b)`, `led_indicator_off()`
- [ ] Uses RMT peripheral (ESP-IDF `led_strip` driver or direct RMT)
- [ ] GPIO8 (WS2812 data pin on DevKitC-02)
- [ ] Works correctly with WS2812 timing requirements

**Validation**:
- Flash firmware, LED lights up with specified color

**Files to create**:
- `micro/components/led_indicator/CMakeLists.txt`
- `micro/components/led_indicator/include/led_indicator.h`
- `micro/components/led_indicator/src/led_indicator.c`

**Notes**: ESP-IDF v5.x has a `led_strip` component — use it if available, or implement directly with RMT.

---

### Task 4.2: LED state machine (red/green/blue patterns)

**Description**: Implement a state machine that drives the LED with different blink patterns based on device state.

**Acceptance Criteria**:
- [ ] LED states: `DISCONNECTED` (red, 100ms on / 1900ms off), `CONNECTED` (green, 100ms on / 4900ms off), `WIFI_ACTIVE` (blue, steady)
- [ ] FreeRTOS task or software timer drives the blink pattern
- [ ] API: `led_indicator_set_state(led_state_e state)`
- [ ] State transitions are immediate (no waiting for current cycle to finish)
- [ ] Default state on boot: `DISCONNECTED` (red blink)

**Validation**:
- Boot without BLE connection → red blink (100ms/1900ms)
- Connect via BLE → green blink (100ms/4900ms)
- Disconnect → back to red blink

---

### Task 4.3: Integration with BLE connection state

**Description**: Register a BLE state callback to automatically change LED state on connect/disconnect.

**Acceptance Criteria**:
- [ ] BLE connect → LED state = `CONNECTED` (green)
- [ ] BLE disconnect → LED state = `DISCONNECTED` (red)
- [ ] Transition is immediate and visible

**Validation**:
- Connect/disconnect from phone, observe LED color changes

---

## Phase 5: Hardware Abstraction

**Objective**: Create a thin abstraction over I2C and define the sensor HAL interface that all sensor drivers will implement.  
**Estimated Duration**: 2–3 days  
**Dependencies**: Phase 0 complete

---

### Task 5.1: I2C bus driver wrapper

**Description**: Create the `i2c_bus` component — a thin wrapper around ESP-IDF's I2C master driver that simplifies common operations (write, read, write-then-read). This isolates sensor drivers from ESP-IDF's I2C API changes.

**Acceptance Criteria**:
- [ ] Component `i2c_bus` created in `micro/components/i2c_bus/`
- [ ] Public API: `i2c_bus_init()`, `i2c_bus_write()`, `i2c_bus_read()`, `i2c_bus_write_read()`, `i2c_bus_deinit()`
- [ ] All functions return `esp_err_t`
- [ ] Configurable I2C port, SDA/SCL pins, clock speed via config struct
- [ ] Header follows project conventions (include guard, `extern "C"`, Doxygen)
- [ ] Uses ESP-IDF's new I2C master driver (v5.x `i2c_master.h`)

**Validation**:
- Build succeeds with the new component
- (Optional) Quick test: init I2C bus, scan for devices, log found addresses

**Files to create**:
- `micro/components/i2c_bus/CMakeLists.txt`
- `micro/components/i2c_bus/include/i2c_bus.h`
- `micro/components/i2c_bus/src/i2c_bus.c`
- `micro/components/i2c_bus/src/i2c_bus_types.h`

**Notes**:
- ESP32-C3 has 1 I2C port (I2C_NUM_0).
- Default pins for DevKitC-02: SDA=GPIO4, SCL=GPIO5 (configurable).
- Use 100 kHz (standard mode) by default; MS5611 supports up to 400 kHz.

---

### Task 5.2: Sensor HAL interface definition

**Description**: Define the `sensor_hal` component — an abstract interface that all sensor drivers must implement. This allows the data pipeline to work with any sensor without knowing its specifics.

**Acceptance Criteria**:
- [ ] Component `sensor_hal` created in `micro/components/sensor_hal/`
- [ ] Interface struct `sensor_hal_interface_t` defined with function pointers:
  - `esp_err_t (*init)(void *ctx, const void *cfg)`
  - `esp_err_t (*read)(void *ctx)`
  - `void (*deinit)(void *ctx)`
  - `int32_t (*get_pressure_pa)(void *ctx)` — returns pressure in Pascals
  - `int32_t (*get_temperature_cc)(void *ctx)` — returns temperature in centi-Celsius (°C × 100)
- [ ] Public struct `sensor_hal_t` that wraps the interface + context pointer
- [ ] Convenience functions: `sensor_hal_init()`, `sensor_hal_read()`, etc. that dispatch through function pointers
- [ ] Header includes Doxygen documentation

**Validation**:
- Build succeeds
- Review: interface is generic enough for MS5611 and future BMP390

**Files to create**:
- `micro/components/sensor_hal/CMakeLists.txt`
- `micro/components/sensor_hal/include/sensor_hal.h`
- `micro/components/sensor_hal/src/sensor_hal.c`

**Notes**:
- The HAL uses `void *ctx` for the sensor instance and `void *cfg` for configuration to keep it type-agnostic.
- Each sensor driver will define its own `_t` struct and cast from `void *`.
- The pipeline task will only interact with `sensor_hal_t`, never with specific sensor types.

---

## Phase 6: MS5611 Sensor Driver

**Objective**: Implement a fully functional MS5611 barometric pressure sensor driver with calibration, compensation, and unit tests.  
**Estimated Duration**: 3–4 days  
**Dependencies**: Phase 1 complete

---

### Task 6.1: MS5611 PROM calibration read

**Description**: Implement reading the 6 factory calibration coefficients (C1–C6) from the MS5611's PROM via I2C. These are needed for pressure/temperature compensation.

**Acceptance Criteria**:
- [ ] Component `sensor_ms5611` created in `micro/components/sensor_ms5611/`
- [ ] `sensor_ms5611_init()` reads all 6 PROM coefficients via I2C
- [ ] Calibration data stored in driver struct
- [ ] Handles I2C errors (retry once, then return error)
- [ ] Validates PROM CRC if available
- [ ] Logs calibration values at INFO level on successful init

**Validation**:
- Flash to DevKitC-02 with MS5611 connected, verify calibration values in log output

**Files to create**:
- `micro/components/sensor_ms5611/CMakeLists.txt`
- `micro/components/sensor_ms5611/include/sensor_ms5611.h`
- `micro/components/sensor_ms5611/src/sensor_ms5611.c`
- `micro/components/sensor_ms5611/src/sensor_ms5611_types.h`

**Notes**:
- MS5611 I2C address: `0x77` (CSB low) or `0x76` (CSB high). Default: `0x77`.
- PROM read commands: `0xA0` to `0xAE` (8 words, 16-bit each; C1–C6 are words 1–6).
- Reset command: `0x1E` — send before PROM read.

---

### Task 6.2: MS5611 raw pressure & temperature read

**Description**: Implement the ADC conversion and raw data read for pressure (D1) and temperature (D2) from the MS5611.

**Acceptance Criteria**:
- [ ] Function to start ADC conversion (command + wait for conversion time)
- [ ] Function to read 24-bit ADC result
- [ ] Supports configurable OSR (Over-Sampling Ratio): 256, 512, 1024, 2048, 4096
- [ ] Default OSR: 4096 (highest precision, ~9.04 ms conversion time)
- [ ] Reads both D1 (pressure) and D2 (temperature) in sequence
- [ ] Handles I2C read errors

**Validation**:
- Flash to hardware, log raw D1 and D2 values, verify they are non-zero and in expected range

**Files to modify**:
- `micro/components/sensor_ms5611/src/sensor_ms5611.c`

**Notes**:
- Conversion commands: `0x40 + 2*OSR_index` (pressure), `0x50 + 2*OSR_index` (temperature)
- ADC read command: `0x00` — returns 3 bytes (24-bit value)
- Conversion time for OSR 4096: ~9.04 ms — use `vTaskDelay(pdMS_TO_TICKS(10))`
- Total read cycle for both P+T: ~20 ms → allows 10 Hz reads with margin

---

### Task 6.3: MS5611 compensation math

**Description**: Implement the second-order temperature compensation algorithm from the MS5611 datasheet to convert raw D1/D2 into calibrated pressure (Pa) and temperature (°C × 100).

**Acceptance Criteria**:
- [ ] Implements the full compensation algorithm per datasheet (including second-order for T < 20°C and T < -15°C)
- [ ] Output pressure in Pascals (int32_t)
- [ ] Output temperature in centi-Celsius (int32_t, e.g., 2350 = 23.50°C)
- [ ] Pure function (no side effects) for easy unit testing
- [ ] Uses 64-bit intermediate calculations to avoid overflow

**Validation**:
- Verify with known test vectors from MS5611 datasheet (Application Note AN520)

**Files to modify**:
- `micro/components/sensor_ms5611/src/sensor_ms5611.c`

**Notes**:
Datasheet test vector:
- C1=40127, C2=36924, C3=23317, C4=23282, C5=33464, C6=28312
- D1=9085466, D2=8569150
- Expected: TEMP=2007 (20.07°C), P=100009 (1000.09 mbar = 100009 Pa)

---

### Task 6.4: Ceedling unit tests for compensation

**Description**: Write comprehensive unit tests for the MS5611 compensation math using known test vectors.

**Acceptance Criteria**:
- [ ] Test file `micro/test/test_sensor_ms5611.c` exists
- [ ] Test: datasheet reference vector produces expected P and T values
- [ ] Test: second-order compensation activates for T < 20°C
- [ ] Test: second-order compensation activates for T < -15°C
- [ ] Test: init with null parameters returns error
- [ ] All tests pass with `ceedling test:all`

**Validation**:
- Run `./scripts/test.sh` — all tests green

**Files to create**:
- `micro/test/test_sensor_ms5611.c`

**Notes**: Mock the I2C layer with CMock. The compensation math should be in a static function that can be tested by calling the public `read` function with mocked I2C responses.

---

### Task 6.5: Integration test on hardware

**Description**: Run the MS5611 driver on actual hardware and verify readings are reasonable.

**Acceptance Criteria**:
- [ ] Pressure readings are in the range 30000–110000 Pa (300–1100 mbar)
- [ ] Temperature readings are in a reasonable range (e.g., 15–35°C indoors)
- [ ] Readings are stable (±10 Pa over 10 seconds at rest)
- [ ] 10 Hz read rate achieved without I2C errors
- [ ] Log output shows formatted pressure and temperature values

**Validation**:
- Flash firmware, observe sensor readings in serial monitor for 60 seconds
- Compare pressure reading with known altitude / weather station data

**Files to modify**:
- `micro/main/main.c` (temporary test loop, will be replaced by data pipeline)

---

## Phase 7: Kalman Filter

**Objective**: Implement a 2-state Kalman filter (altitude + vario) to smooth pressure readings and derive altitude and vertical speed.  
**Estimated Duration**: 2–3 days  
**Dependencies**: Phase 2 complete (compensation math for test data)

---

### Task 7.1: 2-state Kalman filter implementation

**Description**: Implement a 2-state Kalman filter for barometric altitude and vertical speed (vario). The state vector is `[altitude, vario]`. The filter takes pressure-derived altitude as input and outputs both smoothed altitude and estimated vertical speed.

**Acceptance Criteria**:
- [ ] Component `kalman_filter` created in `micro/components/kalman_filter/`
- [ ] State struct: `kalman_state_t` with 2-element state vector `[altitude, vario]`, 2x2 covariance matrix, process noise (Q), measurement noise (R)
- [ ] API: `kalman_init(state, Q, R, initial_altitude)`, `kalman_update(state, measured_altitude, dt)` → filtered altitude + vario
- [ ] Pure C, no ESP-IDF dependencies (fully testable on host)
- [ ] Configurable Q and R parameters for tuning

**Validation**:
- Unit tests with synthetic data show smoothing behavior and correct vario derivation

**Files to create**:
- `micro/components/kalman_filter/CMakeLists.txt`
- `micro/components/kalman_filter/include/kalman_filter.h`
- `micro/components/kalman_filter/src/kalman_filter.c`

**Notes**:
- State vector: `x = [altitude, vario]`. Prediction uses constant-velocity model.
- Measurement: altitude derived from pressure (only altitude is measured, vario is estimated).
- Reasonable starting values: Q=0.01, R=0.5 (tune with real sensor data).

---

### Task 7.2: Altitude calculation from pressure

**Description**: Implement the barometric altitude formula to convert filtered pressure to altitude in meters.

**Acceptance Criteria**:
- [ ] Function: `float altitude_from_pressure(int32_t pressure_pa, int32_t reference_pressure_pa)` → altitude in meters
- [ ] Uses the international barometric formula: `altitude = 44330 * (1 - (P/P0)^(1/5.255))`
- [ ] Reference pressure configurable (default: 101325 Pa = sea level)
- [ ] Pure function, no side effects

**Validation**:
- Known test: 101325 Pa at sea level → 0 m
- Known test: 89876 Pa → ~1000 m

**Files to modify**:
- `micro/components/kalman_filter/src/kalman_filter.c`
- `micro/components/kalman_filter/include/kalman_filter.h`

---

### Task 7.3: Vario (vertical speed) derivation

**Description**: Calculate vertical speed (vario) from the rate of change of filtered altitude.

**Acceptance Criteria**:
- [ ] Function computes vario as `(altitude_current - altitude_previous) / dt`
- [ ] Output in cm/s (integer, as required by LK8EX1)
- [ ] Handles first sample gracefully (vario = 0)
- [ ] Optionally applies a low-pass filter to vario to reduce noise

**Validation**:
- Synthetic test: constant altitude → vario = 0
- Synthetic test: linearly increasing altitude → constant positive vario

**Files to modify**:
- `micro/components/kalman_filter/src/kalman_filter.c`
- `micro/components/kalman_filter/include/kalman_filter.h`

---

### Task 7.4: Ceedling unit tests with synthetic data

**Description**: Write unit tests for the Kalman filter, altitude calculation, and vario derivation.

**Acceptance Criteria**:
- [ ] Test: Kalman filter converges to true value with noisy input
- [ ] Test: Kalman filter with constant input returns same value
- [ ] Test: altitude formula produces known results for known pressures
- [ ] Test: vario = 0 for constant altitude
- [ ] Test: vario correct sign for ascending/descending
- [ ] Test: init with null state returns error
- [ ] All tests pass in `ceedling test:all`

**Validation**:
- Run `./scripts/test.sh` — all tests green

**Files to create**:
- `micro/test/test_kalman_filter.c`

---

## Phase 4: LK8EX1 Protocol

**Objective**: Implement the LK8EX1 NMEA sentence formatter with checksum calculation.  
**Estimated Duration**: 1–2 days  
**Dependencies**: Phase 3 (for understanding the data fields)

---

### Task 3.1: LK8EX1 data abstraction layer

**Description**: Create the `lk8ex1` component that formats flight data into an LK8EX1 NMEA sentence string.

**Acceptance Criteria**:
- [ ] Component `lk8ex1` created in `micro/components/lk8ex1/`
- [ ] Input struct: `lk8ex1_data_t` with fields: `pressure_pa`, `altitude_m` (or 99999), `vario_cm_s`, `temperature_dc` (°C×10), `battery_mv` (or 999)
- [ ] Output function: `lk8ex1_format(data, buffer, buffer_size)` → `esp_err_t`
- [ ] Sentence format: `$LK8EX1,pressure,altitude,vario,temperature,battery*XX\r\n`
- [ ] Checksum: XOR of chars between `$` and `*` (exclusive), 2-digit uppercase hex
- [ ] Buffer size check to prevent overflow
- [ ] Pure C, no ESP-IDF dependencies (testable on host)

**Validation**:
- Unit tests verify correct output for known inputs

**Files to create**:
- `micro/components/lk8ex1/CMakeLists.txt`
- `micro/components/lk8ex1/include/lk8ex1.h`
- `micro/components/lk8ex1/src/lk8ex1.c`

---

### Task 3.2: NMEA checksum calculator

**Description**: Implement a reusable NMEA checksum function (XOR of characters between `$` and `*`).

**Acceptance Criteria**:
- [ ] Function: `uint8_t nmea_checksum(const char *sentence)` — computes XOR checksum
- [ ] Validation function: `bool nmea_validate_checksum(const char *sentence)` — checks existing checksum
- [ ] Handles edge cases: null input, missing `$` or `*`

**Validation**:
- Unit tests with known NMEA sentences

**Files to modify**:
- `micro/components/lk8ex1/src/lk8ex1.c`

**Notes**: The checksum function can also be used by the app-side parser for validation.

---

### Task 3.3: LK8EX1 sentence formatter

**Description**: Create the `lk8ex1` component that formats flight data into an LK8EX1 NMEA sentence string.

**Acceptance Criteria**:
- [ ] Component `lk8ex1` created in `micro/components/lk8ex1/`
- [ ] Input struct: `lk8ex1_data_t` with fields: `pressure_pa`, `altitude_m` (or 99999), `vario_cm_s`, `temperature_dc` (°C×10), `battery_mv` (or 999)
- [ ] Output function: `lk8ex1_format(data, buffer, buffer_size)` → `esp_err_t`
- [ ] Sentence format: `$LK8EX1,pressure,altitude,vario,temperature,battery*XX\r\n`
- [ ] Checksum: XOR of chars between `$` and `*` (exclusive), 2-digit uppercase hex
- [ ] Buffer size check to prevent overflow
- [ ] Pure C, no ESP-IDF dependencies (testable on host)

**Validation**:
- Unit tests verify correct output for known inputs

**Files to create**:
- `micro/components/lk8ex1/CMakeLists.txt`
- `micro/components/lk8ex1/include/lk8ex1.h`
- `micro/components/lk8ex1/src/lk8ex1.c`

---

### Task 3.4: Simulated data provider

**Description**: Provide simulated flight data for testing.

**Acceptance Criteria**:
- [ ] Function to generate random or patterned data
- [ ] Data rate: 10 Hz (matches sensor read rate)
- [ ] Data range: realistic (e.g., 30000–110000 Pa for pressure)
- [ ] Data format: matches LK8EX1 requirements

**Validation**:
- Verify with known test vectors from MS5611 datasheet (Application Note AN520)

**Files to modify**:
- `micro/components/lk8ex1/src/lk8ex1.c`

---

### Task 3.5: BLE + LK8EX1 integration (send simulated frames)

**Description**: Send simulated LK8EX1 frames over BLE.

**Acceptance Criteria**:
- [ ] Function to send simulated frames
- [ ] Frames sent at 4 Hz (matches BLE TX rate)
- [ ] Frames contain valid LK8EX1 data
- [ ] Frames are received correctly

**Validation**:
- Receive LK8EX1 sentences in nRF Connect at ~4 Hz rate

**Files to modify**:
- `micro/main/main.c` (temporary: send simulated LK8EX1 every 250 ms)

---

### Task 3.6: Ceedling unit tests

**Description**: Write unit tests for LK8EX1 formatting and checksum.

**Acceptance Criteria**:
- [ ] Test: format with typical values produces correct sentence
- [ ] Test: format with altitude=99999 (no GPS) works correctly
- [ ] Test: format with battery=999 (no battery readout) works correctly
- [ ] Test: checksum matches manual XOR calculation
- [ ] Test: validate checksum returns true for correct sentence
- [ ] Test: validate checksum returns false for corrupted sentence
- [ ] Test: format with null buffer returns error
- [ ] Test: format with insufficient buffer size returns error
- [ ] All tests pass in `ceedling test:all`

**Validation**:
- Run `./scripts/test.sh` — all tests green

**Files to create**:
- `micro/test/test_lk8ex1.c`

---

## Phase 4: LED Indicator

**Objective**: Drive the onboard WS2812 RGB LED to indicate device state (BLE connected/disconnected, WiFi stub).  
**Estimated Duration**: 1–2 days  
**Dependencies**: Phase 5 (BLE state callbacks)

---

### Task 4.1: WS2812 driver via RMT peripheral

**Description**: Create the `led_indicator` component that drives the WS2812 RGB LED using the ESP32-C3's RMT peripheral.

**Acceptance Criteria**:
- [ ] Component `led_indicator` created in `micro/components/led_indicator/`
- [ ] API: `led_indicator_init()`, `led_indicator_set_color(r, g, b)`, `led_indicator_off()`
- [ ] Uses RMT peripheral (ESP-IDF `led_strip` driver or direct RMT)
- [ ] GPIO8 (WS2812 data pin on DevKitC-02)
- [ ] Works correctly with WS2812 timing requirements

**Validation**:
- Flash firmware, LED lights up with specified color

**Files to create**:
- `micro/components/led_indicator/CMakeLists.txt`
- `micro/components/led_indicator/include/led_indicator.h`
- `micro/components/led_indicator/src/led_indicator.c`

**Notes**: ESP-IDF v5.x has a `led_strip` component — use it if available, or implement directly with RMT.

---

### Task 4.2: LED state machine (red/green/blue patterns)

**Description**: Implement a state machine that drives the LED with different blink patterns based on device state.

**Acceptance Criteria**:
- [ ] LED states: `DISCONNECTED` (red, 100ms on / 1900ms off), `CONNECTED` (green, 100ms on / 4900ms off), `WIFI_ACTIVE` (blue, steady)
- [ ] FreeRTOS task or software timer drives the blink pattern
- [ ] API: `led_indicator_set_state(led_state_e state)`
- [ ] State transitions are immediate (no waiting for current cycle to finish)
- [ ] Default state on boot: `DISCONNECTED` (red blink)

**Validation**:
- Boot without BLE connection → red blink (100ms/1900ms)
- Connect via BLE → green blink (100ms/4900ms)
- Disconnect → back to red blink

---

### Task 4.3: Integration with BLE connection state

**Description**: Register a BLE state callback to automatically change LED state on connect/disconnect.

**Acceptance Criteria**:
- [ ] BLE connect → LED state = `CONNECTED` (green)
- [ ] BLE disconnect → LED state = `DISCONNECTED` (red)
- [ ] Transition is immediate and visible

**Validation**:
- Connect/disconnect from phone, observe LED color changes

---

## Phase 5: Hardware Abstraction

**Objective**: Create a thin abstraction over I2C and define the sensor HAL interface that all sensor drivers will implement.  
**Estimated Duration**: 2–3 days  
**Dependencies**: Phase 0 complete

---

### Task 5.1: I2C bus driver wrapper

**Description**: Create the `i2c_bus` component — a thin wrapper around ESP-IDF's I2C master driver that simplifies common operations (write, read, write-then-read). This isolates sensor drivers from ESP-IDF's I2C API changes.

**Acceptance Criteria**:
- [ ] Component `i2c_bus` created in `micro/components/i2c_bus/`
- [ ] Public API: `i2c_bus_init()`, `i2c_bus_write()`, `i2c_bus_read()`, `i2c_bus_write_read()`, `i2c_bus_deinit()`
- [ ] All functions return `esp_err_t`
- [ ] Configurable I2C port, SDA/SCL pins, clock speed via config struct
- [ ] Header follows project conventions (include guard, `extern "C"`, Doxygen)
- [ ] Uses ESP-IDF's new I2C master driver (v5.x `i2c_master.h`)

**Validation**:
- Build succeeds with the new component
- (Optional) Quick test: init I2C bus, scan for devices, log found addresses

**Files to create**:
- `micro/components/i2c_bus/CMakeLists.txt`
- `micro/components/i2c_bus/include/i2c_bus.h`
- `micro/components/i2c_bus/src/i2c_bus.c`
- `micro/components/i2c_bus/src/i2c_bus_types.h`

**Notes**:
- ESP32-C3 has 1 I2C port (I2C_NUM_0).
- Default pins for DevKitC-02: SDA=GPIO4, SCL=GPIO5 (configurable).
- Use 100 kHz (standard mode) by default; MS5611 supports up to 400 kHz.

---

### Task 5.2: Sensor HAL interface definition

**Description**: Define the `sensor_hal` component — an abstract interface that all sensor drivers must implement. This allows the data pipeline to work with any sensor without knowing its specifics.

**Acceptance Criteria**:
- [ ] Component `sensor_hal` created in `micro/components/sensor_hal/`
- [ ] Interface struct `sensor_hal_interface_t` defined with function pointers:
  - `esp_err_t (*init)(void *ctx, const void *cfg)`
  - `esp_err_t (*read)(void *ctx)`
  - `void (*deinit)(void *ctx)`
  - `int32_t (*get_pressure_pa)(void *ctx)` — returns pressure in Pascals
  - `int32_t (*get_temperature_cc)(void *ctx)` — returns temperature in centi-Celsius (°C × 100)
- [ ] Public struct `sensor_hal_t` that wraps the interface + context pointer
- [ ] Convenience functions: `sensor_hal_init()`, `sensor_hal_read()`, etc. that dispatch through function pointers
- [ ] Header includes Doxygen documentation

**Validation**:
- Build succeeds
- Review: interface is generic enough for MS5611 and future BMP390

**Files to create**:
- `micro/components/sensor_hal/CMakeLists.txt`
- `micro/components/sensor_hal/include/sensor_hal.h`
- `micro/components/sensor_hal/src/sensor_hal.c`

**Notes**:
- The HAL uses `void *ctx` for the sensor instance and `void *cfg` for configuration to keep it type-agnostic.
- Each sensor driver will define its own `_t` struct and cast from `void *`.
- The pipeline task will only interact with `sensor_hal_t`, never with specific sensor types.

---

## Phase 6: MS5611 Sensor Driver

**Objective**: Implement a fully functional MS5611 barometric pressure sensor driver with calibration, compensation, and unit tests.  
**Estimated Duration**: 3–4 days  
**Dependencies**: Phase 1 complete

---

### Task 6.1: MS5611 PROM calibration read

**Description**: Implement reading the 6 factory calibration coefficients (C1–C6) from the MS5611's PROM via I2C. These are needed for pressure/temperature compensation.

**Acceptance Criteria**:
- [ ] Component `sensor_ms5611` created in `micro/components/sensor_ms5611/`
- [ ] `sensor_ms5611_init()` reads all 6 PROM coefficients via I2C
- [ ] Calibration data stored in driver struct
- [ ] Handles I2C errors (retry once, then return error)
- [ ] Validates PROM CRC if available
- [ ] Logs calibration values at INFO level on successful init

**Validation**:
- Flash to DevKitC-02 with MS5611 connected, verify calibration values in log output

**Files to create**:
- `micro/components/sensor_ms5611/CMakeLists.txt`
- `micro/components/sensor_ms5611/include/sensor_ms5611.h`
- `micro/components/sensor_ms5611/src/sensor_ms5611.c`
- `micro/components/sensor_ms5611/src/sensor_ms5611_types.h`

**Notes**:
- MS5611 I2C address: `0x77` (CSB low) or `0x76` (CSB high). Default: `0x77`.
- PROM read commands: `0xA0` to `0xAE` (8 words, 16-bit each; C1–C6 are words 1–6).
- Reset command: `0x1E` — send before PROM read.

---

### Task 6.2: MS5611 raw pressure & temperature read

**Description**: Implement the ADC conversion and raw data read for pressure (D1) and temperature (D2) from the MS5611.

**Acceptance Criteria**:
- [ ] Function to start ADC conversion (command + wait for conversion time)
- [ ] Function to read 24-bit ADC result
- [ ] Supports configurable OSR (Over-Sampling Ratio): 256, 512, 1024, 2048, 4096
- [ ] Default OSR: 4096 (highest precision, ~9.04 ms conversion time)
- [ ] Reads both D1 (pressure) and D2 (temperature) in sequence
- [ ] Handles I2C read errors

**Validation**:
- Flash to hardware, log raw D1 and D2 values, verify they are non-zero and in expected range

**Files to modify**:
- `micro/components/sensor_ms5611/src/sensor_ms5611.c`

**Notes**:
- Conversion commands: `0x40 + 2*OSR_index` (pressure), `0x50 + 2*OSR_index` (temperature)
- ADC read command: `0x00` — returns 3 bytes (24-bit value)
- Conversion time for OSR 4096: ~9.04 ms — use `vTaskDelay(pdMS_TO_TICKS(10))`
- Total read cycle for both P+T: ~20 ms → allows 10 Hz reads with margin

---

### Task 6.3: MS5611 compensation math

**Description**: Implement the second-order temperature compensation algorithm from the MS5611 datasheet to convert raw D1/D2 into calibrated pressure (Pa) and temperature (°C × 100).

**Acceptance Criteria**:
- [ ] Implements the full compensation algorithm per datasheet (including second-order for T < 20°C and T < -15°C)
- [ ] Output pressure in Pascals (int32_t)
- [ ] Output temperature in centi-Celsius (int32_t, e.g., 2350 = 23.50°C)
- [ ] Pure function (no side effects) for easy unit testing
- [ ] Uses 64-bit intermediate calculations to avoid overflow

**Validation**:
- Verify with known test vectors from MS5611 datasheet (Application Note AN520)

**Files to modify**:
- `micro/components/sensor_ms5611/src/sensor_ms5611.c`

**Notes**:
Datasheet test vector:
- C1=40127, C2=36924, C3=23317, C4=23282, C5=33464, C6=28312
- D1=9085466, D2=8569150
- Expected: TEMP=2007 (20.07°C), P=100009 (1000.09 mbar = 100009 Pa)

---

### Task 6.4: Ceedling unit tests for compensation

**Description**: Write comprehensive unit tests for the MS5611 compensation math using known test vectors.

**Acceptance Criteria**:
- [ ] Test file `micro/test/test_sensor_ms5611.c` exists
- [ ] Test: datasheet reference vector produces expected P and T values
- [ ] Test: second-order compensation activates for T < 20°C
- [ ] Test: second-order compensation activates for T < -15°C
- [ ] Test: init with null parameters returns error
- [ ] All tests pass with `ceedling test:all`

**Validation**:
- Run `./scripts/test.sh` — all tests green

**Files to create**:
- `micro/test/test_sensor_ms5611.c`

**Notes**: Mock the I2C layer with CMock. The compensation math should be in a static function that can be tested by calling the public `read` function with mocked I2C responses.

---

### Task 6.5: Integration test on hardware

**Description**: Run the MS5611 driver on actual hardware and verify readings are reasonable.

**Acceptance Criteria**:
- [ ] Pressure readings are in the range 30000–110000 Pa (300–1100 mbar)
- [ ] Temperature readings are in a reasonable range (e.g., 15–35°C indoors)
- [ ] Readings are stable (±10 Pa over 10 seconds at rest)
- [ ] 10 Hz read rate achieved without I2C errors
- [ ] Log output shows formatted pressure and temperature values

**Validation**:
- Flash firmware, observe sensor readings in serial monitor for 60 seconds
- Compare pressure reading with known altitude / weather station data

**Files to modify**:
- `micro/main/main.c` (temporary test loop, will be replaced by data pipeline)

---

## Phase 7: Kalman Filter

**Objective**: Implement a 2-state Kalman filter (altitude + vario) to smooth pressure readings and derive altitude and vertical speed.  
**Estimated Duration**: 2–3 days  
**Dependencies**: Phase 2 complete (compensation math for test data)

---

### Task 7.1: 2-state Kalman filter implementation

**Description**: Implement a 2-state Kalman filter for barometric altitude and vertical speed (vario). The state vector is `[altitude, vario]`. The filter takes pressure-derived altitude as input and outputs both smoothed altitude and estimated vertical speed.

**Acceptance Criteria**:
- [ ] Component `kalman_filter` created in `micro/components/kalman_filter/`
- [ ] State struct: `kalman_state_t` with 2-element state vector `[altitude, vario]`, 2x2 covariance matrix, process noise (Q), measurement noise (R)
- [ ] API: `kalman_init(state, Q, R, initial_altitude)`, `kalman_update(state, measured_altitude, dt)` → filtered altitude + vario
- [ ] Pure C, no ESP-IDF dependencies (fully testable on host)
- [ ] Configurable Q and R parameters for tuning

**Validation**:
- Unit tests with synthetic data show smoothing behavior and correct vario derivation

**Files to create**:
- `micro/components/kalman_filter/CMakeLists.txt`
- `micro/components/kalman_filter/include/kalman_filter.h`
- `micro/components/kalman_filter/src/kalman_filter.c`

**Notes**:
- State vector: `x = [altitude, vario]`. Prediction uses constant-velocity model.
- Measurement: altitude derived from pressure (only altitude is measured, vario is estimated).
- Reasonable starting values: Q=0.01, R=0.5 (tune with real sensor data).

---

### Task 7.2: Altitude calculation from pressure

**Description**: Implement the barometric altitude formula to convert filtered pressure to altitude in meters.

**Acceptance Criteria**:
- [ ] Function: `float altitude_from_pressure(int32_t pressure_pa, int32_t reference_pressure_pa)` → altitude in meters
- [ ] Uses the international barometric formula: `altitude = 44330 * (1 - (P/P0)^(1/5.255))`
- [ ] Reference pressure configurable (default: 101325 Pa = sea level)
- [ ] Pure function, no side effects

**Validation**:
- Known test: 101325 Pa at sea level → 0 m
- Known test: 89876 Pa → ~1000 m

**Files to modify**:
- `micro/components/kalman_filter/src/kalman_filter.c`
- `micro/components/kalman_filter/include/kalman_filter.h`

---

### Task 7.3: Vario (vertical speed) derivation

**Description**: Calculate vertical speed (vario) from the rate of change of filtered altitude.

**Acceptance Criteria**:
- [ ] Function computes vario as `(altitude_current - altitude_previous) / dt`
- [ ] Output in cm/s (integer, as required by LK8EX1)
- [ ] Handles first sample gracefully (vario = 0)
- [ ] Optionally applies a low-pass filter to vario to reduce noise

**Validation**:
- Synthetic test: constant altitude → vario = 0
- Synthetic test: linearly increasing altitude → constant positive vario

**Files to modify**:
- `micro/components/kalman_filter/src/kalman_filter.c`
- `micro/components/kalman_filter/include/kalman_filter.h`

---

### Task 7.4: Ceedling unit tests with synthetic data

**Description**: Write unit tests for the Kalman filter, altitude calculation, and vario derivation.

**Acceptance Criteria**:
- [ ] Test: Kalman filter converges to true value with noisy input
- [ ] Test: Kalman filter with constant input returns same value
- [ ] Test: altitude formula produces known results for known pressures
- [ ] Test: vario = 0 for constant altitude
- [ ] Test: vario correct sign for ascending/descending
- [ ] Test: init with null state returns error
- [ ] All tests pass in `ceedling test:all`

**Validation**:
- Run `./scripts/test.sh` — all tests green

**Files to create**:
- `micro/test/test_kalman_filter.c`

---

## Phase 4: LK8EX1 Protocol

**Objective**: Implement the LK8EX1 NMEA sentence formatter with checksum calculation.  
**Estimated Duration**: 1–2 days  
**Dependencies**: Phase 3 (for understanding the data fields)

---

### Task 3.1: LK8EX1 data abstraction layer

**Description**: Create the `lk8ex1` component that formats flight data into an LK8EX1 NMEA sentence string.

**Acceptance Criteria**:
- [ ] Component `lk8ex1` created in `micro/components/lk8ex1/`
- [ ] Input struct: `lk8ex1_data_t` with fields: `pressure_pa`, `altitude_m` (or 99999), `vario_cm_s`, `temperature_dc` (°C×10), `battery_mv` (or 999)
- [ ] Output function: `lk8ex1_format(data, buffer, buffer_size)` → `esp_err_t`
- [ ] Sentence format: `$LK8EX1,pressure,altitude,vario,temperature,battery*XX\r\n`
- [ ] Checksum: XOR of chars between `$` and `*` (exclusive), 2-digit uppercase hex
- [ ] Buffer size check to prevent overflow
- [ ] Pure C, no ESP-IDF dependencies (testable on host)

**Validation**:
- Unit tests verify correct output for known inputs

**Files to create**:
- `micro/components/lk8ex1/CMakeLists.txt`
- `micro/components/lk8ex1/include/lk8ex1.h`
- `micro/components/lk8ex1/src/lk8ex1.c`

---

### Task 3.2: NMEA checksum calculator

**Description**: Implement a reusable NMEA checksum function (XOR of characters between `$` and `*`).

**Acceptance Criteria**:
- [ ] Function: `uint8_t nmea_checksum(const char *sentence)` — computes XOR checksum
- [ ] Validation function: `bool nmea_validate_checksum(const char *sentence)` — checks existing checksum
- [ ] Handles edge cases: null input, missing `$` or `*`

**Validation**:
- Unit tests with known NMEA sentences

**Files to modify**:
- `micro/components/lk8ex1/src/lk8ex1.c`

**Notes**: The checksum function can also be used by the app-side parser for validation.

---

### Task 3.3: LK8EX1 sentence formatter

**Description**: Create the `lk8ex1` component that formats flight data into an LK8EX1 NMEA sentence string.

**Acceptance Criteria**:
- [ ] Component `lk8ex1` created in `micro/components/lk8ex1/`
- [ ] Input struct: `lk8ex1_data_t` with fields: `pressure_pa`, `altitude_m` (or 99999), `vario_cm_s`, `temperature_dc` (°C×10), `battery_mv` (or 999)
- [ ] Output function: `lk8ex1_format(data, buffer, buffer_size)` → `esp_err_t`
- [ ] Sentence format: `$LK8EX1,pressure,altitude,vario,temperature,battery*XX\r\n`
- [ ] Checksum: XOR of chars between `$` and `*` (exclusive), 2-digit uppercase hex
- [ ] Buffer size check to prevent overflow
- [ ] Pure C, no ESP-IDF dependencies (testable on host)

**Validation**:
- Unit tests verify correct output for known inputs

**Files to create**:
- `micro/components/lk8ex1/CMakeLists.txt`
- `micro/components/lk8ex1/include/lk8ex1.h`
- `micro/components/lk8ex1/src/lk8ex1.c`

---

### Task 3.4: Simulated data provider

**Description**: Provide simulated flight data for testing.

**Acceptance Criteria**:
- [ ] Function to generate random or patterned data
- [ ] Data rate: 10 Hz (matches sensor read rate)
- [ ] Data range: realistic (e.g., 30000–110000 Pa for pressure)
- [ ] Data format: matches LK8EX1 requirements

**Validation**:
- Verify with known test vectors from MS5611 datasheet (Application Note AN520)

**Files to modify**:
- `micro/components/lk8ex1/src/lk8ex1.c`

---

### Task 3.5: BLE + LK8EX1 integration (send simulated frames)

**Description**: Send simulated LK8EX1 frames over BLE.

**Acceptance Criteria**:
- [ ] Function to send simulated frames
- [ ] Frames sent at 4 Hz (matches BLE TX rate)
- [ ] Frames contain valid LK8EX1 data
- [ ] Frames are received correctly

**Validation**:
- Receive LK8EX1 sentences in nRF Connect at ~4 Hz rate

**Files to modify**:
- `micro/main/main.c` (temporary: send simulated LK8EX1 every 250 ms)

---

### Task 3.6: Ceedling unit tests

**Description**: Write unit tests for LK8EX1 formatting and checksum.

**Acceptance Criteria**:
- [ ] Test: format with typical values produces correct sentence
- [ ] Test: format with altitude=99999 (no GPS) works correctly
- [ ] Test: format with battery=999 (no battery readout) works correctly
- [ ] Test: checksum matches manual XOR calculation
- [ ] Test: validate checksum returns true for correct sentence
- [ ] Test: validate checksum returns false for corrupted sentence
- [ ] Test: format with null buffer returns error
- [ ] Test: format with insufficient buffer size returns error
- [ ] All tests pass in `ceedling test:all`

**Validation**:
- Run `./scripts/test.sh` — all tests green

**Files to create**:
- `micro/test/test_lk8ex1.c`

---

## Phase 4: LED Indicator

**Objective**: Drive the onboard WS2812 RGB LED to indicate device state (BLE connected/disconnected, WiFi stub).  
**Estimated Duration**: 1–2 days  
**Dependencies**: Phase 5 (BLE state callbacks)

---

### Task 4.1: WS2812 driver via RMT peripheral

**Description**: Create the `led_indicator` component that drives the WS2812 RGB LED using the ESP32-C3's RMT peripheral.

**Acceptance Criteria**:
- [ ] Component `led_indicator` created in `micro/components/led_indicator/`
- [ ] API: `led_indicator_init()`, `led_indicator_set_color(r, g, b)`, `led_indicator_off()`
- [ ] Uses RMT peripheral (ESP-IDF `led_strip` driver or direct RMT)
- [ ] GPIO8 (WS2812 data pin on DevKitC-02)
- [ ] Works correctly with WS2812 timing requirements

**Validation**:
- Flash firmware, LED lights up with specified color

**Files to create**:
- `micro/components/led_indicator/CMakeLists.txt`
- `micro/components/led_indicator/include/led_indicator.h`
- `micro/components/led_indicator/src/led_indicator.c`

**Notes**: ESP-IDF v5.x has a `led_strip` component — use it if available, or implement directly with RMT.

---

### Task 4.2: LED state machine (red/green/blue patterns)

**Description**: Implement a state machine that drives the LED with different blink patterns based on device state.

**Acceptance Criteria**:
- [ ] LED states: `DISCONNECTED` (red, 100ms on / 1900ms off), `CONNECTED` (green, 100ms on / 4900ms off), `WIFI_ACTIVE` (blue, steady)
- [ ] FreeRTOS task or software timer drives the blink pattern
- [ ] API: `led_indicator_set_state(led_state_e state)`
- [ ] State transitions are immediate (no waiting for current cycle to finish)
- [ ] Default state on boot: `DISCONNECTED` (red blink)

**Validation**:
- Boot without BLE connection → red blink (100ms/1900ms)
- Connect via BLE → green blink (100ms/4900ms)
- Disconnect → back to red blink

---

### Task 4.3: Integration with BLE connection state

**Description**: Register a BLE state callback to automatically change LED state on connect/disconnect.

**Acceptance Criteria**:
- [ ] BLE connect → LED state = `CONNECTED` (green)
- [ ] BLE disconnect → LED state = `DISCONNECTED` (red)
- [ ] Transition is immediate and visible

**Validation**:
- Connect/disconnect from phone, observe LED color changes

---

## Phase 5: Hardware Abstraction

**Objective**: Create a thin abstraction over I2C and define the sensor HAL interface that all sensor drivers will implement.  
**Estimated Duration**: 2–3 days  
**Dependencies**: Phase 0 complete

---

### Task 5.1: I2C bus driver wrapper

**Description**: Create the `i2c_bus` component — a thin wrapper around ESP-IDF's I2C master driver that simplifies common operations (write, read, write-then-read). This isolates sensor drivers from ESP-IDF's I2C API changes.

**Acceptance Criteria**:
- [ ] Component `i2c_bus` created in `micro/components/i2c_bus/`
- [ ] Public API: `i2c_bus_init()`, `i2c_bus_write()`, `i2c_bus_read()`, `i2c_bus_write_read()`, `i2c_bus_deinit()`
- [ ] All functions return `esp_err_t`
- [ ] Configurable I2C port, SDA/SCL pins, clock speed via config struct
- [ ] Header follows project conventions (include guard, `extern "C"`, Doxygen)
- [ ] Uses ESP-IDF's new I2C master driver (v5.x `i2c_master.h`)

**Validation**:
- Build succeeds with the new component
- (Optional) Quick test: init I2C bus, scan for devices, log found addresses

**Files to create**:
- `micro/components/i2c_bus/CMakeLists.txt`
- `micro/components/i2c_bus/include/i2c_bus.h`
- `micro/components/i2c_bus/src/i2c_bus.c`
- `micro/components/i2c_bus/src/i2c_bus_types.h`

**Notes**:
- ESP32-C3 has 1 I2C port (I2C_NUM_0).
- Default pins for DevKitC-02: SDA=GPIO4, SCL=GPIO5 (configurable).
- Use 100 kHz (standard mode) by default; MS5611 supports up to 400 kHz.

---

### Task 5.2: Sensor HAL interface definition

**Description**: Define the `sensor_hal` component — an abstract interface that all sensor drivers must implement. This allows the data pipeline to work with any sensor without knowing its specifics.

**Acceptance Criteria**:
- [ ] Component `sensor_hal` created in `micro/components/sensor_hal/`
- [ ] Interface struct `sensor_hal_interface_t` defined with function pointers:
  - `esp_err_t (*init)(void *ctx, const void *cfg)`
  - `esp_err_t (*read)(void *ctx)`
  - `void (*deinit)(void *ctx)`
  - `int32_t (*get_pressure_pa)(void *ctx)` — returns pressure in Pascals
  - `int32_t (*get_temperature_cc)(void *ctx)` — returns temperature in centi-Celsius (°C × 100)
- [ ] Public struct `sensor_hal_t` that wraps the interface + context pointer
- [ ] Convenience functions: `sensor_hal_init()`, `sensor_hal_read()`, etc. that dispatch through function pointers
- [ ] Header includes Doxygen documentation

**Validation**:
- Build succeeds
- Review: interface is generic enough for MS5611 and future BMP390

**Files to create**:
- `micro/components/sensor_hal/CMakeLists.txt`
- `micro/components/sensor_hal/include/sensor_hal.h`
- `micro/components/sensor_hal/src/sensor_hal.c`

**Notes**:
- The HAL uses `void *ctx` for the sensor instance and `void *cfg` for configuration to keep it type-agnostic.
- Each sensor driver will define its own `_t` struct and cast from `void *`.
- The pipeline task will only interact with `sensor_hal_t`, never with specific sensor types.

---

## Phase 6: MS5611 Sensor Driver

**Objective**: Implement a fully functional MS5611 barometric pressure sensor driver with calibration, compensation, and unit tests.  
**Estimated Duration**: 3–4 days  
**Dependencies**: Phase 1 complete

---

### Task 6.1: MS5611 PROM calibration read

**Description**: Implement reading the 6 factory calibration coefficients (C1–C6) from the MS5611's PROM via I2C. These are needed for pressure/temperature compensation.

**Acceptance Criteria**:
- [ ] Component `sensor_ms5611` created in `micro/components/sensor_ms5611/`
- [ ] `sensor_ms5611_init()` reads all 6 PROM coefficients via I2C
- [ ] Calibration data stored in driver struct
- [ ] Handles I2C errors (retry once, then return error)
- [ ] Validates PROM CRC if available
- [ ] Logs calibration values at INFO level on successful init

**Validation**:
- Flash to DevKitC-02 with MS5611 connected, verify calibration values in log output

**Files to create**:
- `micro/components/sensor_ms5611/CMakeLists.txt`
- `micro/components/sensor_ms5611/include/sensor_ms5611.h`
- `micro/components/sensor_ms5611/src/sensor_ms5611.c`
- `micro/components/sensor_ms5611/src/sensor_ms5611_types.h`

**Notes**:
- MS5611 I2C address: `0x77` (CSB low) or `0x76` (CSB high). Default: `0x77`.
- PROM read commands: `0xA0` to `0xAE` (8 words, 16-bit each; C1–C6 are words 1–6).
- Reset command: `0x1E` — send before PROM read.

---

### Task 6.2: MS5611 raw pressure & temperature read

**Description**: Implement the ADC conversion and raw data read for pressure (D1) and temperature (D2) from the MS5611.

**Acceptance Criteria**:
- [ ] Function to start ADC conversion (command + wait for conversion time)
- [ ] Function to read 24-bit ADC result
- [ ] Supports configurable OSR (Over-Sampling Ratio): 256, 512, 1024, 2048, 4096
- [ ] Default OSR: 4096 (highest precision, ~9.04 ms conversion time)
- [ ] Reads both D1 (pressure) and D2 (temperature) in sequence
- [ ] Handles I2C read errors

**Validation**:
- Flash to hardware, log raw D1 and D2 values, verify they are non-zero and in expected range

**Files to modify**:
- `micro/components/sensor_ms5611/src/sensor_ms5611.c`

**Notes**:
- Conversion commands: `0x40 + 2*OSR_index` (pressure), `0x50 + 2*OSR_index` (temperature)
- ADC read command: `0x00` — returns 3 bytes (24-bit value)
- Conversion time for OSR 4096: ~9.04 ms — use `vTaskDelay(pdMS_TO_TICKS(10))`
- Total read cycle for both P+T: ~20 ms → allows 10 Hz reads with margin

---

### Task 6.3: MS5611 compensation math

**Description**: Implement the second-order temperature compensation algorithm from the MS5611 datasheet to convert raw D1/D2 into calibrated pressure (Pa) and temperature (°C × 100).

**Acceptance Criteria**:
- [ ] Implements the full compensation algorithm per datasheet (including second-order for T < 20°C and T < -15°C)
- [ ] Output pressure in Pascals (int32_t)
- [ ] Output temperature in centi-Celsius (int32_t, e.g., 2350 = 23.50°C)
- [ ] Pure function (no side effects) for easy unit testing
- [ ] Uses 64-bit intermediate calculations to avoid overflow

**Validation**:
- Verify with known test vectors from MS5611 datasheet (Application Note AN520)

**Files to modify**:
- `micro/components/sensor_ms5611/src/sensor_ms5611.c`

**Notes**:
Datasheet test vector:
- C1=40127, C2=36924, C3=23317, C4=23282, C5=33464, C6=28312
- D1=9085466, D2=8569150
- Expected: TEMP=2007 (20.07°C), P=100009 (1000.09 mbar = 100009 Pa)

---

### Task 6.4: Ceedling unit tests for compensation

**Description**: Write comprehensive unit tests for the MS5611 compensation math using known test vectors.

**Acceptance Criteria**:
- [ ] Test file `micro/test/test_sensor_ms5611.c` exists
- [ ] Test: datasheet reference vector produces expected P and T values
- [ ] Test: second-order compensation activates for T < 20°C
- [ ] Test: second-order compensation activates for T < -15°C
- [ ] Test: init with null parameters returns error
- [ ] All tests pass with `ceedling test:all`

**Validation**:
- Run `./scripts/test.sh` — all tests green

**Files to create**:
- `micro/test/test_sensor_ms5611.c`

**Notes**: Mock the I2C layer with CMock. The compensation math should be in a static function that can be tested by calling the public `read` function with mocked I2C responses.

---

### Task 6.5: Integration test on hardware

**Description**: Run the MS5611 driver on actual hardware and verify readings are reasonable.

**Acceptance Criteria**:
- [ ] Pressure readings are in the range 30000–110000 Pa (300–1100 mbar)
- [ ] Temperature readings are in a reasonable range (e.g., 15–35°C indoors)
- [ ] Readings are stable (±10 Pa over 10 seconds at rest)
- [ ] 10 Hz read rate achieved without I2C errors
- [ ] Log output shows formatted pressure and temperature values

**Validation**:
- Flash firmware, observe sensor readings in serial monitor for 60 seconds
- Compare pressure reading with known altitude / weather station data

**Files to modify**:
- `micro/main/main.c` (temporary test loop, will be replaced by data pipeline)

---

## Phase 7: Kalman Filter

**Objective**: Implement a 2-state Kalman filter (altitude + vario) to smooth pressure readings and derive altitude and vertical speed.  
**Estimated Duration**: 2–3 days  
**Dependencies**: Phase 2 complete (compensation math for test data)

---

### Task 7.1: 2-state Kalman filter implementation

**Description**: Implement a 2-state Kalman filter for barometric altitude and vertical speed (vario). The state vector is `[altitude, vario]`. The filter takes pressure-derived altitude as input and outputs both smoothed altitude and estimated vertical speed.

**Acceptance Criteria**:
- [ ] Component `kalman_filter` created in `micro/components/kalman_filter/`
- [ ] State struct: `kalman_state_t` with 2-element state vector `[altitude, vario]`, 2x2 covariance matrix, process noise (Q), measurement noise (R)
- [ ] API: `kalman_init(state, Q, R, initial_altitude)`, `kalman_update(state, measured_altitude, dt)` → filtered altitude + vario
- [ ] Pure C, no ESP-IDF dependencies (fully testable on host)
- [ ] Configurable Q and R parameters for tuning

**Validation**:
- Unit tests with synthetic data show smoothing behavior and correct vario derivation

**Files to create**:
- `micro/components/kalman_filter/CMakeLists.txt`
- `micro/components/kalman_filter/include/kalman_filter.h`
- `micro/components/kalman_filter/src/kalman_filter.c`

**Notes**:
- State vector: `x = [altitude, vario]`. Prediction uses constant-velocity model.
- Measurement: altitude derived from pressure (only altitude is measured, vario is estimated).
- Reasonable starting values: Q=0.01, R=0.5 (tune with real sensor data).

---

### Task 7.2: Altitude calculation from pressure

**Description**: Implement the barometric altitude formula to convert filtered pressure to altitude in meters.

**Acceptance Criteria**:
- [ ] Function: `float altitude_from_pressure(int32_t pressure_pa, int32_t reference_pressure_pa)` → altitude in meters
- [ ] Uses the international barometric formula: `altitude = 44330 * (1 - (P/P0)^(1/5.255))`
- [ ] Reference pressure configurable (default: 101325 Pa = sea level)
- [ ] Pure function, no side effects

**Validation**:
- Known test: 101325 Pa at sea level → 0 m
- Known test: 89876 Pa → ~1000 m

**Files to modify**:
- `micro/components/kalman_filter/src/kalman_filter.c`
- `micro/components/kalman_filter/include/kalman_filter.h`

---

### Task 7.3: Vario (vertical speed) derivation

**Description**: Calculate vertical speed (vario) from the rate of change of filtered altitude.

**Acceptance Criteria**:
- [ ] Function computes vario as `(altitude_current - altitude_previous) / dt`
- [ ] Output in cm/s (integer, as required by LK8EX1)
- [ ] Handles first sample gracefully (vario = 0)
- [ ] Optionally applies a low-pass filter to vario to reduce noise

**Validation**:
- Synthetic test: constant altitude → vario = 0
- Synthetic test: linearly increasing altitude → constant positive vario

**Files to modify**:
- `micro/components/kalman_filter/src/kalman_filter.c`
- `micro/components/kalman_filter/include/kalman_filter.h`

---

### Task 7.4: Ceedling unit tests with synthetic data

**Description**: Write unit tests for the Kalman filter, altitude calculation, and vario derivation.

**Acceptance Criteria**:
- [ ] Test: Kalman filter converges to true value with noisy input
- [ ] Test: Kalman filter with constant input returns same value
- [ ] Test: altitude formula produces known results for known pressures
- [ ] Test: vario = 0 for constant altitude
- [ ] Test: vario correct sign for ascending/descending
- [ ] Test: init with null state returns error
- [ ] All tests pass in `ceedling test:all`

**Validation**:
- Run `./scripts/test.sh` — all tests green

**Files to create**:
- `micro/test/test_kalman_filter.c`

---

## Phase 4: LK8EX1 Protocol

**Objective**: Implement the LK8EX1 NMEA sentence formatter with checksum calculation.  
**Estimated Duration**: 1–2 days  
**Dependencies**: Phase 3 (for understanding the data fields)

---

### Task 3.1: LK8EX1 data abstraction layer

**Description**: Create the `lk8ex1` component that formats flight data into an LK8EX1 NMEA sentence string.

**Acceptance Criteria**:
- [ ] Component `lk8ex1` created in `micro/components/lk8ex1/`
- [ ] Input struct: `lk8ex1_data_t` with fields: `pressure_pa`, `altitude_m` (or 99999), `vario_cm_s`, `temperature_dc` (°C×10), `battery_mv` (or 999)
- [ ] Output function: `lk8ex1_format(data, buffer, buffer_size)` → `esp_err_t`
- [ ] Sentence format: `$LK8EX1,pressure,altitude,vario,temperature,battery*XX\r\n`
- [ ] Checksum: XOR of chars between `$` and `*` (exclusive), 2-digit uppercase hex
- [ ] Buffer size check to prevent overflow
- [ ] Pure C, no ESP-IDF dependencies (testable on host)

**Validation**:
- Unit tests verify correct output for known inputs

**Files to create**:
- `micro/components/lk8ex1/CMakeLists.txt`
- `micro/components/lk8ex1/include/lk8ex1.h`
- `micro/components/lk8ex1/src/lk8ex1.c`

---

### Task 3.2: NMEA checksum calculator

**Description**: Implement a reusable NMEA checksum function (XOR of characters between `$` and `*`).

**Acceptance Criteria**:
- [ ] Function: `uint8_t nmea_checksum(const char *sentence)` — computes XOR checksum
- [ ] Validation function: `bool nmea_validate_checksum(const char *sentence)` — checks existing checksum
- [ ] Handles edge cases: null input, missing `$` or `*`

**Validation**:
- Unit tests with known NMEA sentences

**Files to modify**:
- `micro/components/lk8ex1/src/lk8ex1.c`

**Notes**: The checksum function can also be used by the app-side parser for validation.

---

### Task 3.3: LK8EX1 sentence formatter

**Description**: Create the `lk8ex1` component that formats flight data into an LK8EX1 NMEA sentence string.

**Acceptance Criteria**:
- [ ] Component `lk8ex1` created in `micro/components/lk8ex1/`
- [ ] Input struct: `lk8ex1_data_t` with fields: `pressure_pa`, `altitude_m` (or 99999), `vario_cm_s`, `temperature_dc` (°C×10), `battery_mv` (or 999)
- [ ] Output function: `lk8ex1_format(data, buffer, buffer_size)` → `esp_err_t`
- [ ] Sentence format: `$LK8EX1,pressure,altitude,vario,temperature,battery*XX\r\n`
- [ ] Checksum: XOR of chars between `$` and `*` (exclusive), 2-digit uppercase hex
- [ ] Buffer size check to prevent overflow
- [ ] Pure C, no ESP-IDF dependencies (testable on host)

**Validation**:
- Unit tests verify correct output for known inputs

**Files to create**:
- `micro/components/lk8ex1/CMakeLists.txt`
- `micro/components/lk8ex1/include/lk8ex1.h`
- `micro/components/lk8ex1/src/lk8ex1.c`

---

### Task 3.4: Simulated data provider

**Description**: Provide simulated flight data for testing.

**Acceptance Criteria**:
- [ ] Function to generate random or patterned data
- [ ] Data rate: 10 Hz (matches sensor read rate)
- [ ] Data range: realistic (e.g., 30000–110000 Pa for pressure)
- [ ] Data format: matches LK8EX1 requirements

**Validation**:
- Verify with known test vectors from MS5611 datasheet (Application Note AN520)

**Files to modify**:
- `micro/components/lk8ex1/src/lk8ex1.c`

---

### Task 3.5: BLE + LK8EX1 integration (send simulated frames)

**Description**: Send simulated LK8EX1 frames over BLE.

**Acceptance Criteria**:
- [ ] Function to send simulated frames
- [ ] Frames sent at 4 Hz (matches BLE TX rate)
- [ ] Frames contain valid LK8EX1 data
- [ ] Frames are received correctly

**Validation**:
- Receive LK8EX1 sentences in nRF Connect at ~4 Hz rate

**Files to modify**:
- `micro/main/main.c` (temporary: send simulated LK8EX1 every 250 ms)

---

### Task 3.6: Ceedling unit tests

**Description**: Write unit tests for LK8EX1 formatting and checksum.

**Acceptance Criteria**:
- [ ] Test: format with typical values produces correct sentence
- [ ] Test: format with altitude=99999 (no GPS) works correctly
- [ ] Test: format with battery=999 (no battery readout) works correctly
- [ ] Test: checksum matches manual XOR calculation
- [ ] Test: validate checksum returns true for correct sentence
- [ ] Test: validate checksum returns false for corrupted sentence
- [ ] Test: format with null buffer returns error
- [ ] Test: format with insufficient buffer size returns error
- [ ] All tests pass in `ceedling test:all`

**Validation**:
- Run `./scripts/test.sh` — all tests green

**Files to create**:
- `micro/test/test_lk8ex1.c`

---

## Phase 4: LED Indicator

**Objective**: Drive the onboard WS2812 RGB LED to indicate device state (BLE connected/disconnected, WiFi stub).  
**Estimated Duration**: 1–2 days  
**Dependencies**: Phase 5 (BLE state callbacks)

---

### Task 4.1: WS2812 driver via RMT peripheral

**Description**: Create the `led_indicator` component that drives the WS2812 RGB LED using the ESP32-C3's RMT peripheral.

**Acceptance Criteria**:
- [ ] Component `led_indicator` created in `micro/components/led_indicator/`
- [ ] API: `led_indicator_init()`, `led_indicator_set_color(r, g, b)`, `led_indicator_off()`
- [ ] Uses RMT peripheral (ESP-IDF `led_strip` driver or direct RMT)
- [ ] GPIO8 (WS2812 data pin on DevKitC-02)
- [ ] Works correctly with WS2812 timing requirements

**Validation**:
- Flash firmware, LED lights up with specified color

**Files to create**:
- `micro/components/led_indicator/CMakeLists.txt`
- `micro/components/led_indicator/include/led_indicator.h`
- `micro/components/led_indicator/src/led_indicator.c`

**Notes**: ESP-IDF v5.x has a `led_strip` component — use it if available, or implement directly with RMT.

---

### Task 4.2: LED state machine (red/green/blue patterns)

**Description**: Implement a state machine that drives the LED with different blink patterns based on device state.

**Acceptance Criteria**:
- [ ] LED states: `DISCONNECTED` (red, 100ms on / 1900ms off), `CONNECTED` (green, 100ms on / 4900ms off), `WIFI_ACTIVE` (blue, steady)
- [ ] FreeRTOS task or software timer drives the blink pattern
- [ ] API: `led_indicator_set_state(led_state_e state)`
- [ ] State transitions are immediate (no waiting for current cycle to finish)
- [ ] Default state on boot: `DISCONNECTED` (red blink)

**Validation**:
- Boot without BLE connection → red blink (100ms/1900ms)
- Connect via BLE → green blink (100ms/4900ms)
- Disconnect → back to red blink

---

### Task 4.3: Integration with BLE connection state

**Description**: Register a BLE state callback to automatically change LED state on connect/disconnect.

**Acceptance Criteria**:
- [ ] BLE connect → LED state = `CONNECTED` (green)
- [ ] BLE disconnect → LED state = `DISCONNECTED` (red)
- [ ] Transition is immediate and visible

**Validation**:
- Connect/disconnect from phone, observe LED color changes

---

## Phase 5: Hardware Abstraction

**Objective**: Create a thin abstraction over I2C and define the sensor HAL interface that all sensor drivers will implement.  
**Estimated Duration**: 2–3 days  
**Dependencies**: Phase 0 complete

---

### Task 5.1: I2C bus driver wrapper

**Description**: Create the `i2c_bus` component — a thin wrapper around ESP-IDF's I2C master driver that simplifies common operations (write, read, write-then-read). This isolates sensor drivers from ESP-IDF's I2C API changes.

**Acceptance Criteria**:
- [ ] Component `i2c_bus` created in `micro/components/i2c_bus/`
- [ ] Public API: `i2c_bus_init()`, `i2c_bus_write()`, `i2c_bus_read()`, `i2c_bus_write_read()`, `i2c_bus_deinit()`
- [ ] All functions return `esp_err_t`
- [ ] Configurable I2C port, SDA/SCL pins, clock speed via config struct
- [ ] Header follows project conventions (include guard, `extern "C"`, Doxygen)
- [ ] Uses ESP-IDF's new I2C master driver (v5.x `i2c_master.h`)

**Validation**:
- Build succeeds with the new component
- (Optional) Quick test: init I2C bus, scan for devices, log found addresses

**Files to create**:
- `micro/components/i2c_bus/CMakeLists.txt`
- `micro/components/i2c_bus/include/i2c_bus.h`
- `micro/components/i2c_bus/src/i2c_bus.c`
- `micro/components/i2c_bus/src/i2c_bus_types.h`

**Notes**:
- ESP32-C3 has 1 I2C port (I2C_NUM_0).
- Default pins for DevKitC-02: SDA=GPIO4, SCL=GPIO5 (configurable).
- Use 100 kHz (standard mode) by default; MS5611 supports up to 400 kHz.

---

### Task 5.2: Sensor HAL interface definition

**Description**: Define the `sensor_hal` component — an abstract interface that all sensor drivers must implement. This allows the data pipeline to work with any sensor without knowing its specifics.

**Acceptance Criteria**:
- [ ] Component `sensor_hal` created in `micro/components/sensor_hal/`
- [ ] Interface struct `sensor_hal_interface_t` defined with function pointers:
  - `esp_err_t (*init)(void *ctx, const void *cfg)`
  - `esp_err_t (*read)(void *ctx)`
  - `void (*deinit)(void *ctx)`
  - `int32_t (*get_pressure_pa)(void *ctx)` — returns pressure in Pascals
  - `int32_t (*get_temperature_cc)(void *ctx)` — returns temperature in centi-Celsius (°C × 100)
- [ ] Public struct `sensor_hal_t` that wraps the interface + context pointer
- [ ] Convenience functions: `sensor_hal_init()`, `sensor_hal_read()`, etc. that dispatch through function pointers
- [ ] Header includes Doxygen documentation

**Validation**:
- Build succeeds
- Review: interface is generic enough for MS5611 and future BMP390

**Files to create**:
- `micro/components/sensor_hal/CMakeLists.txt`
- `micro/components/sensor_hal/include/sensor_hal.h`
- `micro/components/sensor_hal/src/sensor_hal.c`

**Notes**:
- The HAL uses `void *ctx` for the sensor instance and `void *cfg` for configuration to keep it type-agnostic.
- Each sensor driver will define its own `_t` struct and cast from `void *`.
- The pipeline task will only interact with `sensor_hal_t`, never with specific sensor types.

---

## Phase 6: MS5611 Sensor Driver

**Objective**: Implement a fully functional MS5611 barometric pressure sensor driver with calibration, compensation, and unit tests.  
**Estimated Duration**: 3–4 days  
**Dependencies**: Phase 1 complete

---

### Task 6.1: MS5611 PROM calibration read

**Description**: Implement reading the 6 factory calibration coefficients (C1–C6) from the MS5611's PROM via I2C. These are needed for pressure/temperature compensation.

**Acceptance Criteria**:
- [ ] Component `sensor_ms5611` created in `micro/components/sensor_ms5611/`
- [ ] `sensor_ms5611_init()` reads all 6 PROM coefficients via I2C
- [ ] Calibration data stored in driver struct
- [ ] Handles I2C errors (retry once, then return error)
- [ ] Validates PROM CRC if available
- [ ] Logs calibration values at INFO level on successful init

**Validation**:
- Flash to DevKitC-02 with MS5611 connected, verify calibration values in log output

**Files to create**:
- `micro/components/sensor_ms5611/CMakeLists.txt`
- `micro/components/sensor_ms5611/include/sensor_ms5611.h`
- `micro/components/sensor_ms5611/src/sensor_ms5611.c`
- `micro/components/sensor_ms5611/src/sensor_ms5611_types.h`

**Notes**:
- MS5611 I2C address: `0x77` (CSB low) or `0x76` (CSB high). Default: `0x77`.
- PROM read commands: `0xA0` to `0xAE` (8 words, 16-bit each; C1–C6 are words 1–6).
- Reset command: `0x1E` — send before PROM read.

---

### Task 6.2: MS5611 raw pressure & temperature read

**Description**: Implement the ADC conversion and raw data read for pressure (D1) and temperature (D2) from the MS5611.

**Acceptance Criteria**:
- [ ] Function to start ADC conversion (command + wait for conversion time)
- [ ] Function to read 24-bit ADC result
- [ ] Supports configurable OSR (Over-Sampling Ratio): 256, 512, 1024, 2048, 4096
- [ ] Default OSR: 4096 (highest precision, ~9.04 ms conversion time)
- [ ] Reads both D1 (pressure) and D2 (temperature) in sequence
- [ ] Handles I2C read errors

**Validation**:
- Flash to hardware, log raw D1 and D2 values, verify they are non-zero and in expected range

**Files to modify**:
- `micro/components/sensor_ms5611/src/sensor_ms5611.c`

**Notes**:
- Conversion commands: `0x40 + 2*OSR_index` (pressure), `0x50 + 2*OSR_index` (temperature)
- ADC read command: `0x00` — returns 3 bytes (24-bit value)
- Conversion time for OSR 4096: ~9.04 ms — use `vTaskDelay(pdMS_TO_TICKS(10))`
- Total read cycle for both P+T: ~20 ms → allows 10 Hz reads with margin

---

### Task 6.3: MS5611 compensation math

**Description**: Implement the second-order temperature compensation algorithm from the MS5611 datasheet to convert raw D1/D2 into calibrated pressure (Pa) and temperature (°C × 100).

**Acceptance Criteria**:
- [ ] Implements the full compensation algorithm per datasheet (including second-order for T < 20°C and T < -15°C)
- [ ] Output pressure in Pascals (int32_t)
- [ ] Output temperature in centi-Celsius (int32_t, e.g., 2350 = 23.50°C)
- [ ] Pure function (no side effects) for easy unit testing
- [ ] Uses 64-bit intermediate calculations to avoid overflow

**Validation**:
- Verify with known test vectors from MS5611 datasheet (Application Note AN520)

**Files to modify**:
- `micro/components/sensor_ms5611/src/sensor_ms5611.c`

**Notes**:
Datasheet test vector:
- C1=40127, C2=36924, C3=23317, C4=23282, C5=33464, C6=28312
- D1=9085466, D2=8569150
- Expected: TEMP=2007 (20.07°C), P=100009 (1000.09 mbar = 100009 Pa)

---

### Task 6.4: Ceedling unit tests for compensation

**Description**: Write comprehensive unit tests for the MS5611 compensation math using known test vectors.

**Acceptance Criteria**:
- [ ] Test file `micro/test/test_sensor_ms5611.c` exists
- [ ] Test: datasheet reference vector produces expected P and T values
- [ ] Test: second-order compensation activates for T < 20°C
- [ ] Test: second-order compensation activates for T < -15°C
- [ ] Test: init with null parameters returns error
- [ ] All tests pass with `ceedling test:all`

**Validation**:
- Run `./scripts/test.sh` — all tests green

**Files to create**:
- `micro/test/test_sensor_ms5611.c`

**Notes**: Mock the I2C layer with CMock. The compensation math should be in a static function that can be tested by calling the public `read` function with mocked I2C responses.

---

### Task 6.5: Integration test on hardware

**Description**: Run the MS5611 driver on actual hardware and verify readings are reasonable.

**Acceptance Criteria**:
- [ ] Pressure readings are in the range 30000–110000 Pa (300–1100 mbar)
- [ ] Temperature readings are in a reasonable range (e.g., 15–35°C indoors)
- [ ] Readings are stable (±10 Pa over 10 seconds at rest)
- [ ] 10 Hz read rate achieved without I2C errors
- [ ] Log output shows formatted pressure and temperature values

**Validation**:
- Flash firmware, observe sensor readings in serial monitor for 60 seconds
- Compare pressure reading with known altitude / weather station data

**Files to modify**:
- `micro/main/main.c` (temporary test loop, will be replaced by data pipeline)

---

## Phase 7: Kalman Filter

**Objective**: Implement a 2-state Kalman filter (altitude + vario) to smooth pressure readings and derive altitude and vertical speed.  
**Estimated Duration**: 2–3 days  
**Dependencies**: Phase 2 complete (compensation math for test data)

---

### Task 7.1: 2-state Kalman filter implementation

**Description**: Implement a 2-state Kalman filter for barometric altitude and vertical speed (vario). The state vector is `[altitude, vario]`. The filter takes pressure-derived altitude as input and outputs both smoothed altitude and estimated vertical speed.

**Acceptance Criteria**:
- [ ] Component `kalman_filter` created in `micro/components/kalman_filter/`
- [ ] State struct: `kalman_state_t` with 2-element state vector `[altitude, vario]`, 2x2 covariance matrix, process noise (Q), measurement noise (R)
- [ ] API: `kalman_init(state, Q, R, initial_altitude)`, `kalman_update(state, measured_altitude, dt)` → filtered altitude + vario
- [ ] Pure C, no ESP-IDF dependencies (fully testable on host)
- [ ] Configurable Q and R parameters for tuning

**Validation**:
- Unit tests with synthetic data show smoothing behavior and correct vario derivation

**Files to create**:
- `micro/components/kalman_filter/CMakeLists.txt`
- `micro/components/kalman_filter/include/kalman_filter.h`
- `micro/components/kalman_filter/src/kalman_filter.c`

**Notes**:
- State vector: `x = [altitude, vario]`. Prediction uses constant-velocity model.
- Measurement: altitude derived from pressure (only altitude is measured, vario is estimated).
- Reasonable starting values: Q=0.01, R=0.5 (tune with real sensor data).

---

### Task 7.2: Altitude calculation from pressure

**Description**: Implement the barometric altitude formula to convert filtered pressure to altitude in meters.

**Acceptance Criteria**:
- [ ] Function: `float altitude_from_pressure(int32_t pressure_pa, int32_t reference_pressure_pa)` → altitude in meters
- [ ] Uses the international barometric formula: `altitude = 44330 * (1 - (P/P0)^(1/5.255))`
- [ ] Reference pressure configurable (default: 101325 Pa = sea level)
- [ ] Pure function, no side effects

**Validation**:
- Known test: 101325 Pa at sea level → 0 m
- Known test: 89876 Pa → ~1000 m

**Files to modify**:
- `micro/components/kalman_filter/src/kalman_filter.c`
- `micro/components/kalman_filter/include/kalman_filter.h`

---

### Task 7.3: Vario (vertical speed) derivation

**Description**: Calculate vertical speed (vario) from the rate of change of filtered altitude.

**Acceptance Criteria**:
- [ ] Function computes vario as `(altitude_current - altitude_previous) / dt`
- [ ] Output in cm/s (integer, as required by LK8EX1)
- [ ] Handles first sample gracefully (vario = 0)
- [ ] Optionally applies a low-pass filter to vario to reduce noise

**Validation**:
- Synthetic test: constant altitude → vario = 0
- Synthetic test: linearly increasing altitude → constant positive vario

**Files to modify**:
- `micro/components/kalman_filter/src/kalman_filter.c`
- `micro/components/kalman_filter/include/kalman_filter.h`

---

### Task 7.4: Ceedling unit tests with synthetic data

**Description**: Write unit tests for the Kalman filter, altitude calculation, and vario derivation.

**Acceptance Criteria**:
- [ ] Test: Kalman filter converges to true value with noisy input
- [ ] Test: Kalman filter with constant input returns same value
- [ ] Test: altitude formula produces known results for known pressures
- [ ] Test: vario = 0 for constant altitude
- [ ] Test: vario correct sign for ascending/descending
- [ ] Test: init with null state returns error
- [ ] All tests pass in `ceedling test:all`

**Validation**:
- Run `./scripts/test.sh` — all tests green

**Files to create**:
- `micro/test/test_kalman_filter.c`

---

## Phase 4: LK8EX1 Protocol

**Objective**: Implement the LK8EX1 NMEA sentence formatter with checksum calculation.  
**Estimated Duration**: 1–2 days  
**Dependencies**: Phase 3 (for understanding the data fields)

---

### Task 3.1: LK8EX1 data abstraction layer

**Description**: Create the `lk8ex1` component that formats flight data into an LK8EX1 NMEA sentence string.

**Acceptance Criteria**:
- [ ] Component `lk8ex1` created in `micro/components/lk8ex1/`
- [ ] Input struct: `lk8ex1_data_t` with fields: `pressure_pa`, `altitude_m` (or 99999), `vario_cm_s`, `temperature_dc` (°C×10), `battery_mv` (or 999)
- [ ] Output function: `lk8ex1_format(data, buffer, buffer_size)` → `esp_err_t`
- [ ] Sentence format: `$LK8EX1,pressure,altitude,vario,temperature,battery*XX\r\n`
- [ ] Checksum: XOR of chars between `$` and `*` (exclusive), 2-digit uppercase hex
- [ ] Buffer size check to prevent overflow
- [ ] Pure C, no ESP-IDF dependencies (testable on host)

**Validation**:
- Unit tests verify correct output for known inputs

**Files to create**:
- `micro/components/lk8ex1/CMakeLists.txt`
- `micro/components/lk8ex1/include/lk8ex1.h`
- `micro/components/lk8ex1/src/lk8ex1.c`

---

### Task 3.2: NMEA checksum calculator

**Description**: Implement a reusable NMEA checksum function (XOR of characters between `$` and `*`).

**Acceptance Criteria**:
- [ ] Function: `uint8_t nmea_checksum(const char *sentence)` — computes XOR checksum
- [ ] Validation function: `bool nmea_validate_checksum(const char *sentence)` — checks existing checksum
- [ ] Handles edge cases: null input, missing `$` or `*`

**Validation**:
- Unit tests with known NMEA sentences

**Files to modify**:
- `micro/components/lk8ex1/src/lk8ex1.c`

**Notes**: The checksum function can also be used by the app-side parser for validation.

---

### Task 3.3: LK8EX1 sentence formatter

**Description**: Create the `lk8ex1` component that formats flight data into an LK8EX1 NMEA sentence string.

**Acceptance Criteria**:
- [ ] Component `lk8ex1` created in `micro/components/lk8ex1/`
- [ ] Input struct: `lk8ex1_data_t` with fields: `pressure_pa`, `altitude_m` (or 99999), `vario_cm_s`, `temperature_dc` (°C×10), `battery_mv` (or 999)
- [ ] Output function: `lk8ex1_format(data, buffer, buffer_size)` → `esp_err_t`
- [ ] Sentence format: `$LK8EX1,pressure,altitude,vario,temperature,battery*XX\r\n`
- [ ] Checksum: XOR of chars between `$` and `*` (exclusive), 2-digit uppercase hex
- [ ] Buffer size check to prevent overflow
- [ ] Pure C, no ESP-IDF dependencies (testable on host)

**Validation**:
- Unit tests verify correct output for known inputs

**Files to create**:
- `micro/components/lk8ex1/CMakeLists.txt`
- `micro/components/lk8ex1/include/lk8ex1.h`
- `micro/components/lk8ex1/src/lk8ex1.c`

---

### Task 3.4: Simulated data provider

**Description**: Provide simulated flight data for testing.

**Acceptance Criteria**:
- [ ] Function to generate random or patterned data
- [ ] Data rate: 10 Hz (matches sensor read rate)
- [ ] Data range: realistic (e.g., 30000–110000 Pa for pressure)
- [ ] Data format: matches LK8EX1 requirements

**Validation**:
- Verify with known test vectors from MS5611 datasheet (Application Note AN520)

**Files to modify**:
- `micro/components/lk8ex1/src/lk8ex1.c`

---

### Task 3.5: BLE + LK8EX1 integration (send simulated frames)

**Description**: Send simulated LK8EX1 frames over BLE.

**Acceptance Criteria**:
- [ ] Function to send simulated frames
- [ ] Frames sent at 4 Hz (matches BLE TX rate)
- [ ] Frames contain valid LK8EX1 data
- [ ] Frames are received correctly

**Validation**:
- Receive LK8EX1 sentences in nRF Connect at ~4 Hz rate

**Files to modify**:
- `micro/main/main.c` (temporary: send simulated LK8EX1 every 250 ms)

---

### Task 3.6: Ceedling unit tests

**Description**: Write unit tests for LK8EX1 formatting and checksum.

**Acceptance Criteria**:
- [ ] Test: format with typical values produces correct sentence
- [ ] Test: format with altitude=99999 (no GPS) works correctly
- [ ] Test: format with battery=999 (no battery readout) works correctly
- [ ] Test: checksum matches manual XOR calculation
- [ ] Test: validate checksum returns true for correct sentence
- [ ] Test: validate checksum returns false for corrupted sentence
- [ ] Test: format with null buffer returns error
- [ ] Test: format with insufficient buffer size returns error
- [ ] All tests pass in `ceedling test:all`

**Validation**:
- Run `./scripts/test.sh` — all tests green

**Files to create**:
- `micro/test/test_lk8ex1.c`

---

## Phase 4: LED Indicator

**Objective**: Drive the onboard WS2812 RGB LED to indicate device state (BLE connected/disconnected, WiFi stub).  
**Estimated Duration**: 1–2 days  
**Dependencies**: Phase 5 (BLE state callbacks)

---

### Task 4.1: WS2812 driver via RMT peripheral

**Description**: Create the `led_indicator` component that drives the WS2812 RGB LED using the ESP32-C3's RMT peripheral.

**Acceptance Criteria**:
- [ ] Component `led_indicator` created in `micro/components/led_indicator/`
- [ ] API: `led_indicator_init()`, `led_indicator_set_color(r, g, b)`, `led_indicator_off()`
- [ ] Uses RMT peripheral (ESP-IDF `led_strip` driver or direct RMT)
- [ ] GPIO8 (WS2812 data pin on DevKitC-02)
- [ ] Works correctly with WS2812 timing requirements

**Validation**:
- Flash firmware, LED lights up with specified color

**Files to create**:
- `micro/components/led_indicator/CMakeLists.txt`
- `micro/components/led_indicator/include/led_indicator.h`
- `micro/components/led_indicator/src/led_indicator.c`

**Notes**: ESP-IDF v5.x has a `led_strip` component — use it if available, or implement directly with RMT.

---

### Task 4.2: LED state machine (red/green/blue patterns)

**Description**: Implement a state machine that drives the LED with different blink patterns based on device state.

**Acceptance Criteria**:
- [ ] LED states: `DISCONNECTED` (red, 100ms on / 1900ms off), `CONNECTED` (green, 100ms on / 4900ms off), `WIFI_ACTIVE` (blue, steady)
- [ ] FreeRTOS task or software timer drives the blink pattern
- [ ] API: `led_indicator_set_state(led_state_e state)`
- [ ] State transitions are immediate (no waiting for current cycle to finish)
- [ ] Default state on boot: `DISCONNECTED` (red blink)

**Validation**:
- Boot without BLE connection → red blink (100ms/1900ms)
- Connect via BLE → green blink (100ms/4900ms)
- Disconnect → back to red blink

---

### Task 4.3: Integration with BLE connection state

**Description**: Register a BLE state callback to automatically change LED state on connect/disconnect.

**Acceptance Criteria**:
- [ ] BLE connect → LED state = `CONNECTED` (green)
- [ ] BLE disconnect → LED state = `DISCONNECTED` (red)
- [ ] Transition is immediate and visible

**Validation**:
- Connect/disconnect from phone, observe LED color changes

---

## Phase 5: Hardware Abstraction

**Objective**: Create a thin abstraction over I2C and define the sensor HAL interface that all sensor drivers will implement.  
**Estimated Duration**: 2–3 days  
**Dependencies**: Phase 0 complete

---

### Task 5.1: I2C bus driver wrapper

**Description**: Create the `i2c_bus` component — a thin wrapper around ESP-IDF's I2C master driver that simplifies common operations (write, read, write-then-read). This isolates sensor drivers from ESP-IDF's I2C API changes.

**Acceptance Criteria**:
- [ ] Component `i2c_bus` created in `micro/components/i2c_bus/`
- [ ] Public API: `i2c_bus_init()`, `i2c_bus_write()`, `i2c_bus_read()`, `i2c_bus_write_read()`, `i2c_bus_deinit()`
- [ ] All functions return `esp_err_t`
- [ ] Configurable I2C port, SDA/SCL pins, clock speed via config struct
- [ ] Header follows project conventions (include guard, `extern "C"`, Doxygen)
- [ ] Uses ESP-IDF's new I2C master driver (v5.x `i2c_master.h`)

**Validation**:
- Build succeeds with the new component
- (Optional) Quick test: init I2C bus, scan for devices, log found addresses

**Files to create**:
- `micro/components/i2c_bus/CMakeLists.txt`
- `micro/components/i2c_bus/include/i2c_bus.h`
- `micro/components/i2c_bus/src/i2c_bus.c`
- `micro/components/i2c_bus/src/i2c_bus_types.h`

**Notes**:
- ESP32-C3 has 1 I2C port (I2C_NUM_0).
- Default pins for DevKitC-02: SDA=GPIO4, SCL=GPIO5 (configurable).
- Use 100 kHz (standard mode) by default; MS5611 supports up to 400 kHz.

---

### Task 5.2: Sensor HAL interface definition

**Description**: Define the `sensor_hal` component — an abstract interface that all sensor drivers must implement. This allows the data pipeline to work with any sensor without knowing its specifics.

**Acceptance Criteria**:
- [ ] Component `sensor_hal` created in `micro/components/sensor_hal/`
- [ ] Interface struct `sensor_hal_interface_t` defined with function pointers:
  - `esp_err_t (*init)(void *ctx, const void *cfg)`
  - `esp_err_t (*read)(void *ctx)`
  - `void (*deinit)(void *ctx)`
  - `int32_t (*get_pressure_pa)(void *ctx)` — returns pressure in Pascals
  - `int32_t (*get_temperature_cc)(void *ctx)` — returns temperature in centi-Celsius (°C × 100)
- [ ] Public struct `sensor_hal_t` that wraps the interface + context pointer
- [ ] Convenience functions: `sensor_hal_init()`, `sensor_hal_read()`, etc. that dispatch through function pointers
- [ ] Header includes Doxygen documentation

**Validation**:
- Build succeeds
- Review: interface is generic enough for MS5611 and future BMP390

**Files to create**:
- `micro/components/sensor_hal/CMakeLists.txt`
- `micro/components/sensor_hal/include/sensor_hal.h`