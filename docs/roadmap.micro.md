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
- [x] **Phase 1: Software Architecture Design**
  - [x] Task 1.1: System architecture document
  - [x] Task 1.2: Component interface contracts
  - [x] Task 1.3: FreeRTOS task model and data flow design
- [x] **Phase 2: LK8EX1 Protocol**
  - [x] Task 2.1: LK8EX1 data types and formatter
  - [x] Task 2.2: NMEA checksum calculator
  - [x] Task 2.3: Ceedling unit tests for LK8EX1
- [x] **Phase 3: BLE NUS Service**
  - [x] Task 3.1: NimBLE initialization and GAP configuration
  - [x] Task 3.2: NUS GATT service registration
  - [x] Task 3.3: TX notification (send data)
  - [x] Task 3.4: RX write handler (reserved for future use)
  - [x] Task 3.5: Connection state management
  - [x] Task 3.6: BLE + LK8EX1 integration (send simulated frames)
  - [x] Task 3.7: Verify with nRF Connect
- [ ] **Phase 3.5: App Debug Stream Fast-Track (P0)**
  - [ ] Task 3.5.1: Define deterministic LK8EX1 simulated-frame profiles for app debugging
  - [ ] Task 3.5.2: Expose debug profile selection for integration tests (nominal/climb/sink/edge-cases)
  - [ ] Task 3.5.3: Validate end-to-end with app frame inspector and record evidence
  - [ ] Task 3.5.4: Validate NUS TX stream through Linux app console mirror
- [x] **Phase 4: LED Indicator**
  - [x] Task 4.1: WS2812 driver via RMT peripheral
  - [x] Task 4.2: LED state machine (patterns per `led_state_e`)
  - [x] Task 4.3: Integration with BLE connection state
- [ ] **Phase 5: Sensor HAL (Compile-Time Abstraction)**
  - [ ] Task 5.1: Kconfig sensor selection (`choice SENSOR_DRIVER`)
  - [ ] Task 5.2: `sensor_hal` public API and compile-time dispatch
  - [ ] Task 5.3: I2C bus initialization
- [ ] **Phase 6: MS5611 Sensor Driver**
  - [ ] Task 6.1: MS5611 PROM calibration read
  - [ ] Task 6.2: MS5611 raw pressure & temperature read
  - [ ] Task 6.3: MS5611 compensation math
  - [ ] Task 6.4: Ceedling unit tests for compensation
  - [ ] Task 6.5: Integration test on hardware
- [ ] **Phase 7: BMP390 Sensor Driver**
  - [ ] Task 7.1: BMP390 trimming coefficients read
  - [ ] Task 7.2: BMP390 raw pressure & temperature read
  - [ ] Task 7.3: BMP390 compensation math
  - [ ] Task 7.4: Ceedling unit tests for compensation
  - [ ] Task 7.5: Integration test on hardware
- [ ] **Phase 8: Kalman Filter**
  - [ ] Task 8.1: 2-state Kalman filter implementation
  - [ ] Task 8.2: Altitude calculation from pressure
  - [ ] Task 8.3: Altitude calibration (inverse barometric formula)
  - [ ] Task 8.4: Vario (vertical speed) derivation
  - [ ] Task 8.5: Ceedling unit tests with synthetic data
- [ ] **Phase 9: Data Pipeline**
  - [ ] Task 9.1: Shared flight data structure and mutex
  - [ ] Task 9.2: Calibration queue (sensor_task consumer)
  - [ ] Task 9.3: Sensor reader task (10 Hz)
  - [ ] Task 9.4: BLE sender task (4 Hz)
  - [ ] Task 9.5: Replace simulated provider with real sensor data
  - [ ] Task 9.6: End-to-end data flow validation
- [ ] **Phase 10: NVS Configuration**
  - [ ] Task 10.1: Config schema definition and defaults
  - [ ] Task 10.2: NVS read/write with validation
  - [ ] Task 10.3: BLE Config Service GATT (read/write characteristics)
  - [ ] Task 10.4: Config task (event-driven)
  - [ ] Task 10.5: Ceedling unit tests for config validation
- [ ] **Phase 11: Power Optimization**
  - [ ] Task 11.1: Light-sleep between sensor reads
  - [ ] Task 11.2: BLE connection interval optimization
  - [ ] Task 11.3: Peripheral power gating
  - [ ] Task 11.4: Power consumption measurement & logging
- [ ] **Phase 12: Integration & Validation**
  - [ ] Task 12.1: End-to-end test with XCTrack
  - [ ] Task 12.2: Long-duration stability test (8+ hours)
  - [ ] Task 12.3: Power consumption budget verification
  - [ ] Task 12.4: Edge case testing (BLE disconnect/reconnect, sensor errors)
- [ ] **Phase 13: Documentation & Cleanup**
  - [ ] Task 13.1: Firmware README with build/flash instructions
  - [ ] Task 13.2: Component API documentation
  - [ ] Task 13.3: Architecture diagram update (Mermaid)
  - [ ] Task 13.4: Code review pass (Boy Scout Rule)

---

## Cross-Phase Rule — `conductor-model-hardware`

**Mandatory scope**: all implementation phases (`0`, `2` to `12`).

For every module/component implemented in those phases:

- **Conductor**: orchestration, lifecycle, error mapping, retries, timing.
- **Model**: pure logic/state (deterministic and testable without ESP-IDF dependencies).
- **Hardware**: platform adapters and peripheral/driver calls.

**Phase completion gate**:
- No implementation phase is considered complete if new module code violates this split.
- Validation must include functional checks **after** refactorization to this pattern.
- The module checklist in `docs/architecture/conductor-model-hardware-template.md` must be applied.

---

## Phase 0: Project Bootstrap

**Objective**: Set up a working ESP-IDF project that compiles, flashes, and runs a "Hello World" on the ESP32-C3-DevKitC-02.  
**Estimated Duration**: 2–3 days  
**Dependencies**: None

**Refactorización (obligatoria)**:
- Aplicar Boy Scout Rule al cerrar cada tarea de la fase.
- Reducir duplicación y complejidad accidental sin cambiar comportamiento funcional.
- Mantener nombres y límites de módulo claros para código autoexplicativo.
- Repetir la validación de la fase después de cada refactorización.

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

**Description**: Create bash scripts in repository-root `scripts/micro/` for common firmware development workflows. All scripts should accept `-p PORT` for serial port override (default: `/dev/ttyUSB0`) and be runnable as `./scripts/micro/<name>.sh` from project root.

**Acceptance Criteria**:
- [x] `build.sh` — runs `idf.py build`, exits non-zero on failure
- [x] `flash.sh` — runs `idf.py -p $PORT flash`, supports `-p` flag
- [x] `monitor.sh` — runs `idf.py -p $PORT monitor`, supports `-p` flag
- [x] `test.sh` — runs `cd ../test && ceedling test:all`
- [x] `all.sh` — chains build → flash → monitor
- [x] All scripts are executable (`chmod +x`)
- [x] All scripts print colored status messages (green=success, red=error)

**Validation**:
- Run `./scripts/micro/build.sh` — build succeeds
- Run `./scripts/micro/test.sh` — Ceedling tests pass

**Files to create**:
- `scripts/micro/build.sh`
- `scripts/micro/flash.sh`
- `scripts/micro/monitor.sh`
- `scripts/micro/test.sh`
- `scripts/micro/all.sh`

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
- Run `./scripts/micro/all.sh` — observe startup message in monitor output

**Files to modify**:
- None (verification only)

**Notes**: This is the gate for Phase 0. Do not proceed to Phase 1 until this passes.

---

### Task 0.7: Create ESP-IDF environment activation script

**Description**: Create a sourceable shell script `scripts/micro/env.sh` that activates or deactivates the ESP-IDF environment (`idf.py`, toolchain, Python venv). This allows using the build/flash/monitor scripts from any terminal without manually sourcing ESP-IDF's `export.sh`.

**Usage**:
- Activate: `source ./scripts/micro/env.sh` or `. ./scripts/micro/env.sh`
- Deactivate: `idf_deactivate`

**Acceptance Criteria**:
- [x] `scripts/micro/env.sh` exists and is sourceable (not executable directly)
- [x] Sourcing it activates the ESP-IDF environment (adds `idf.py` to PATH)
- [x] Defines an `idf_deactivate` function that restores the original PATH/environment
- [x] Prints a colored status message indicating activation/deactivation
- [x] Is idempotent — sourcing twice does not duplicate PATH entries
- [x] Detects ESP-IDF installation path automatically or uses `IDF_PATH` if set
- [x] All existing scripts (`build.sh`, `flash.sh`, etc.) work after sourcing

**Validation**:
- Open a fresh terminal, `source ./scripts/micro/env.sh`, run `idf.py --version`
- Run `idf_deactivate`, verify `idf.py` is no longer in PATH
- Source again, run `./scripts/micro/build.sh` — build succeeds

**Files to create**:
- `scripts/micro/env.sh`

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
- [x] Document exists with clear structure and diagrams
- [x] Covers all components and their interactions
- [x] Includes timing diagrams and resource allocation
- [x] Defines `conductor-model-hardware` pattern and module responsibilities

**Validation**:
- Review: architecture is clear and comprehensive

**Files to create**:
- `docs/architecture/firmware-architecture.md`

---

### Task 1.2: Component interface contracts

**Description**: Define the interfaces that all components must implement.

**Acceptance Criteria**:
- [x] Interface contracts defined in `docs/architecture/ble_protocol.md`
- [x] Each component has a brief description in its header
- [x] Contracts are compatible with `conductor-model-hardware` split (public API in conductor)

**Validation**:
- Review: contracts are clear and sufficient

**Files to create**:
- `docs/architecture/ble_protocol.md`

---

### Task 1.3: FreeRTOS task model and data flow design

**Description**: Define the FreeRTOS task model and data flow.

**Acceptance Criteria**:
- [x] Task model defined in `docs/architecture/firmware-architecture.md`
- [x] Data flow diagrams included

**Validation**:
- Review: model is clear and realistic

**Files to create**:
- `docs/architecture/firmware-architecture.md`

---

---

## Phase 2: LK8EX1 Protocol

**Objective**: Implement the LK8EX1 NMEA sentence formatter with checksum calculation. This is a pure-C component with zero ESP-IDF dependencies, making it the ideal first component to implement and test with Ceedling.  
**Estimated Duration**: 1–2 days  
**Dependencies**: Phase 1 (interface contract in `firmware-architecture.md` §4.5)  
 
**Refactorización (obligatoria)**:
- Aplicar Boy Scout Rule al cerrar cada tarea de la fase.
- Reducir duplicación y complejidad accidental sin cambiar comportamiento funcional.
- Mantener nombres y límites de módulo claros para código autoexplicativo.
- Repetir la validación de la fase después de cada refactorización.

**Architecture Reference**: `firmware-architecture.md` §4.5 `lk8ex1`

---

### Task 2.1: LK8EX1 data types and formatter

**Description**: Create the `lk8ex1` component with the `lk8ex1_data_t` input struct and `lk8ex1_format()` function, following the contract defined in the architecture document.

**Acceptance Criteria**:
- [x] Component `lk8ex1` created in `micro/components/lk8ex1/`
- [x] Internal split applied: `lk8ex1_conductor.c` (conductor), `lk8ex1_model.c` (model), `lk8ex1_hardware.c` (hardware)
- [x] `lk8ex1_data_t` struct with fields per architecture §4.5: `pressure_pa`, `altitude_m`, `vario_cms`, `temperature_dc`, `battery_mv`
- [x] `esp_err_t lk8ex1_format(const lk8ex1_data_t *data, char *buffer, size_t buffer_size)`
- [x] Output format: `$LK8EX1,pressure,altitude,vario,temperature,battery*XX\r\n`
- [x] Returns `ESP_ERR_INVALID_ARG` for NULL pointers
- [x] Returns `ESP_ERR_INVALID_SIZE` if `buffer_size < LK8EX1_MAX_SENTENCE_LEN` (64 bytes)
- [x] All integer formatting — no floating-point operations
- [x] Pure C, no ESP-IDF dependencies (testable on host)

**Validation**:
- Unit tests verify correct output for known inputs

**Status Note (2026-02-27 — LK8EX1 commercial-format alignment)**:
- [x] Formatter output aligned to commercial-compatible field shape: pressure in Pa and temperature rendered with one decimal digit from `temperature_dc`.
- [x] Sentence compatibility target validated against examples such as `$LK8EX1,102108,99999,1,19.8,999,*36` (payload/checksum model unchanged).

**Files to create**:
- `micro/components/lk8ex1/CMakeLists.txt`
- `micro/components/lk8ex1/include/lk8ex1.h`
- `micro/components/lk8ex1/src/lk8ex1.c`
- `micro/components/lk8ex1/src/lk8ex1_model.c`
- `micro/components/lk8ex1/src/lk8ex1_model.h`
- `micro/components/lk8ex1/src/lk8ex1_hardware.c`
- `micro/components/lk8ex1/src/lk8ex1_hardware.h`

---

### Task 2.2: NMEA checksum calculator

**Description**: Implement checksum and validation functions per architecture contract.

**Acceptance Criteria**:
- [x] `uint8_t lk8ex1_checksum(const char *sentence, size_t len)` — XOR of chars between `$` and `*` (exclusive)
- [x] `bool lk8ex1_validate(const char *sentence)` — parses, computes, and compares checksum
- [x] Handles edge cases: null input, missing `$` or `*`

**Validation**:
- Unit tests with known NMEA sentences

**Files to modify**:
- `micro/components/lk8ex1/include/lk8ex1.h`
- `micro/components/lk8ex1/src/lk8ex1.c`

---

### Task 2.3: Ceedling unit tests for LK8EX1

**Description**: Write comprehensive unit tests for LK8EX1 formatting and checksum.

**Acceptance Criteria**:
- [x] Test file `micro/test/test_lk8ex1.c` exists
- [x] Test: format with typical values produces correct sentence (e.g., `$LK8EX1,101325,99999,50,23.5,999*XX\r\n`)
- [x] Test: format with `altitude_m = 99999` (no GPS) works correctly
- [x] Test: format with `battery_mv = 999` (no battery) works correctly
- [x] Test: checksum matches manual XOR calculation
- [x] Test: `lk8ex1_validate()` returns `true` for correct sentence
- [x] Test: `lk8ex1_validate()` returns `false` for corrupted sentence
- [x] Test: format with NULL buffer returns `ESP_ERR_INVALID_ARG`
- [x] Test: format with insufficient buffer size returns `ESP_ERR_INVALID_SIZE`
- [x] All tests pass in `ceedling test:all`

**Validation**:
- Run `./scripts/micro/test.sh` — all tests green

**Status Note (2026-02-27 — test updates + execution status)**:
- [x] LK8EX1 unit expectations updated for decimal temperature output and Pa pressure output.
- [ ] Full Ceedling rerun is currently blocked on this workstation by pre-existing config issue (`:paths -> :support -> support/mocks` missing).

**Files to create**:
- `micro/test/test_lk8ex1.c`

---

## Phase 3: BLE NUS Service

**Objective**: Implement Bluetooth Low Energy with NimBLE, advertising the Nordic UART Service (NUS), and supporting TX notifications. Integrate with LK8EX1 to send simulated flight data.  
**Estimated Duration**: 3–4 days  
**Dependencies**: Phase 2 (LK8EX1 formatter for integration test)  
 
**Refactorización (obligatoria)**:
- Aplicar Boy Scout Rule al cerrar cada tarea de la fase.
- Reducir duplicación y complejidad accidental sin cambiar comportamiento funcional.
- Mantener nombres y límites de módulo claros para código autoexplicativo.
- Repetir la validación de la fase después de cada refactorización.

**Architecture Reference**: `firmware-architecture.md` §4.6 `ble_nus`, `ble_protocol.md`

---

### Task 3.1: NimBLE initialization and GAP configuration

**Description**: Create the `ble_nus` component. Initialize the NimBLE host stack and configure GAP parameters following the API contract from the architecture document.

**Acceptance Criteria**:
- [x] Component `ble_nus` created in `micro/components/ble_nus/`
- [x] Internal split applied: `ble_nus_conductor.c` (conductor), `ble_nus_model.c` (model), `ble_nus_hardware.c` (hardware)
- [x] `ble_nus_cfg_t` struct with `device_name` (default: `"FlyInPeace"`) and `adv_interval_ms` (default: 100)
- [x] `esp_err_t ble_nus_init(const ble_nus_cfg_t *cfg)` — initializes NimBLE host, starts advertising
- [x] GAP event handler processes: `BLE_GAP_EVENT_CONNECT`, `BLE_GAP_EVENT_DISCONNECT`, `BLE_GAP_EVENT_MTU`
- [x] Advertising starts automatically at boot
- [x] Advertising restarts after disconnect
- [x] MTU negotiation: requests 256 bytes
- [x] Logs connection/disconnection events

**Validation**:
- Flash firmware, scan with nRF Connect — device `"FlyInPeace"` appears

**Files to create**:
- `micro/components/ble_nus/CMakeLists.txt`
- `micro/components/ble_nus/include/ble_nus.h`
- `micro/components/ble_nus/src/ble_nus.c`
- `micro/components/ble_nus/src/ble_nus_model.c`
- `micro/components/ble_nus/src/ble_nus_model.h`
- `micro/components/ble_nus/src/ble_nus_hardware.c`
- `micro/components/ble_nus/src/ble_nus_hardware.h`

**Notes**:
- NimBLE host task created internally by `nimble_port_freertos_init()` (Priority 4, 4096 bytes stack).
- UUIDs per `ble_protocol.md`: Service `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`.

---

### Task 3.2: NUS GATT service registration

**Description**: Register the Nordic UART Service (NUS) with NimBLE's GATT server, including TX (notify) and RX (write) characteristics.

**Acceptance Criteria**:
- [x] NUS service registered with UUID `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- [x] TX characteristic (UUID `6E400003-...`) with notify property
- [x] RX characteristic (UUID `6E400002-...`) with write property
- [x] CCCD (Client Characteristic Configuration Descriptor) for TX notifications
- [x] Access callback handles read/write/subscribe operations

**Validation**:
- Connect with nRF Connect — NUS service visible with both characteristics

**Files to modify**:
- `micro/components/ble_nus/src/ble_nus.c`

---

### Task 3.3: TX notification (send data)

**Description**: Implement the `ble_nus_send()` function per architecture contract.

**Acceptance Criteria**:
- [x] `esp_err_t ble_nus_send(const uint8_t *data, uint16_t len)` implemented
- [x] Checks if client is connected and CCCD subscribed to notifications
- [x] Respects MTU size — fragments data if payload exceeds (MTU - 3)
- [x] Returns `ESP_ERR_INVALID_STATE` if not connected or not subscribed
- [x] Thread-safe (can be called from any task)

**Validation**:
- Send test string, verify receipt in nRF Connect UART terminal

**Files to modify**:
- `micro/components/ble_nus/src/ble_nus.c`

---

### Task 3.4: RX write handler (reserved for future use)

**Description**: Implement the NUS RX write handler. Data is passed to a registered callback.

**Acceptance Criteria**:
- [x] `void ble_nus_register_rx_callback(ble_nus_rx_cb_t callback)` implemented
- [x] Callback receives `(const uint8_t *data, uint16_t len)` on GATT write
- [x] Handles partial writes / fragmented data
- [x] If no callback registered, RX data is silently discarded

**Validation**:
- Send text from nRF Connect UART, verify it arrives in the registered callback (log output)

**Files to modify**:
- `micro/components/ble_nus/src/ble_nus.c`

---

### Task 3.5: Connection state management

**Description**: Expose BLE connection state per architecture contract.

**Acceptance Criteria**:
- [x] `bool ble_nus_is_connected(void)` — returns current connection state (atomic read)
- [x] `void ble_nus_register_state_callback(ble_nus_state_cb_t cb)` — notified on connect/disconnect
- [x] Callback receives: `(bool connected, uint16_t conn_handle)`
- [x] Thread-safe state access

**Validation**:
- Connect/disconnect from nRF Connect, verify state callbacks fire and `is_connected()` updates

**Files to modify**:
- `micro/components/ble_nus/src/ble_nus.c`

---

### Task 3.6: BLE + LK8EX1 integration (send simulated frames)

**Description**: Send simulated LK8EX1 frames over BLE at 4 Hz to validate the full BLE → NUS TX pipeline.

**Acceptance Criteria**:
- [x] Temporary FreeRTOS task sends simulated `lk8ex1_data_t` every 250 ms
- [x] Data contains realistic values (pressure ~101325 Pa, vario 0, temp 230)
- [x] Frames are valid LK8EX1 sentences with correct checksum
- [ ] Frames received correctly in nRF Connect UART view

**Validation**:
- Receive LK8EX1 sentences in nRF Connect at ~4 Hz rate

**Files to modify**:
- `micro/main/main.c` (temporary: simulated sender, replaced in Phase 9)

---

### Task 3.7: Verify with nRF Connect

**Description**: Full BLE NUS validation with nRF Connect app.

**Acceptance Criteria**:
- [x] Device advertises and is visible in nRF Connect
- [x] Connection succeeds, NUS service and characteristics visible
- [x] TX notifications received correctly (full LK8EX1 sentences)
- [x] RX writes received correctly in firmware callback (log output)
- [x] Reconnection works after disconnect
- [x] `esp_err_t ble_nus_deinit(void)` cleans up resources

**Validation**:
- Test with nRF Connect (mandatory)
- Test with XCTrack (nice to have — validate LK8EX1 parsing)

**Status**:
- [x] Completed on 2026-02-19 after hardware validation with nRF Connect.
- [x] Automated pre-check passed on 2026-02-19:
  - BLE stack initializes without `esp_nimble_hci_init failed`.
  - Advertising starts successfully (`BLE advertising started: name=FlyInPeace interval_ms=100`).
  - NUS advertising payload fix validated (ADV + scan response split).

**Execution Checklist (ready to run)**:
1. Flash and open monitor:
  - `cd micro`
  - `source ./scripts/micro/env.sh`
  - `idf.py -p /dev/ttyUSB0 flash monitor`
2. In nRF Connect (Android):
  - Scan and verify device name `FlyInPeace` appears.
  - Connect to device.
  - Discover services and verify NUS UUID `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`.
  - Verify RX char `6E400002-...` (Write) and TX char `6E400003-...` (Notify).
  - Enable notifications on TX characteristic.
  - Confirm LK8EX1 frames arrive at ~4 Hz.
3. RX path test:
  - Write text payload from nRF Connect to RX characteristic.
  - Confirm firmware log prints RX length (from `ble_rx_log_callback`).
4. Reconnect test:
  - Disconnect from nRF Connect.
  - Verify firmware restarts advertising automatically.
  - Reconnect and re-enable notifications.
5. Deinit smoke test (optional, code-level):
  - Confirm `ble_nus_deinit()` returns `ESP_OK` if invoked during controlled shutdown.

**Pass Criteria (mark Task 3.7 done only if all pass)**:
- Device is discoverable as `FlyInPeace`.
- NUS service and both characteristics are visible.
- TX notifications stream valid LK8EX1 sentences continuously.
- RX writes are received by firmware callback (log evidence).
- Disconnect/reconnect cycle works without reboot.

**Evidence Template (fill during test run)**:
- Board/Port used: ________________________________
- Firmware build date/hash: ________________________
- nRF Connect version/device: ______________________
- Time to discovery (s): ___________________________
- TX sample sentence captured: _____________________
- RX callback log line captured: ___________________
- Reconnect attempts passed (count): _______________
- Final verdict: PASS / FAIL

**Quick Field Check (60s MVP)**:
1. Run `./scripts/micro/all.sh -p /dev/ttyUSB0` from repository root.
2. Open nRF Connect, scan, and connect to `FlyInPeace`.
3. Enable `Notify` on `6E400003-...` and confirm LK8EX1 lines appear.
4. Write `PING` to `6E400002-...` and confirm firmware logs RX length.
5. Disconnect and reconnect once; confirm stream resumes.

---

## Phase 4: LED Indicator

**Objective**: Drive the onboard WS2812 RGB LED to indicate device state using the state machine defined in the architecture.  
**Estimated Duration**: 1–2 days  
**Dependencies**: Phase 3 (BLE state callbacks for integration)  

---

## Phase 3.5: App Debug Stream Fast-Track (P0)

**Objective**: Prioritize app integration by streaming deterministic simulated LK8EX1 frames as early as possible, enabling frame-level debugging from the app before real sensor pipeline completion.
**Estimated Duration**: 0.5–1 day
**Dependencies**: Phase 3 complete

**Priority Rule (override)**:
- Execute Phase 3.5 before starting new hardware features not required for app integration.
- Do not postpone this phase behind LED, sensor, or UI-polish related work.

### Task 3.5.1: Deterministic LK8EX1 simulation profiles

**Description**: Define stable, repeatable LK8EX1 simulation profiles to feed app parser/debug tooling.

**Acceptance Criteria**:
- [x] Profile `nominal`: stable flight-like values with valid checksum
- [x] Profile `climb`: positive vario trend with realistic pressure/altitude relation
- [x] Profile `sink`: negative vario trend with realistic pressure/altitude relation
- [x] Profile `edge`: includes placeholders (`99999`, `999`) and boundary numeric values
- [x] Frame cadence fixed at debug target (default 4 Hz)

**Validation**:
- Capture at least 30 seconds per profile and verify deterministic field behavior between runs

**Status Note (2026-02-25 — implementation + build validation)**:
- Deterministic profile generator implemented in `micro/main/main.c` with fixed-sequence frames at `LK8EX1_TX_PERIOD_MS=250` (4 Hz).
- Profiles implemented: `nominal`, `climb`, `sink`, `edge`.
- Additional debug-only profiles implemented for validation matrix support: `malformed-checksum`, `malformed-shape`.

### Task 3.5.2: Debug profile selection for integration tests

**Description**: Provide a simple mechanism to switch simulation profile during app-integration sessions.

**Acceptance Criteria**:
- [x] Profile switch mechanism documented (build-time flag, compile-time constant, or runtime command)
- [x] Default profile remains `nominal` to preserve current behavior
- [x] Switching profile does not break BLE advertising or NUS notifications

**Validation**:
- Switch profile, reconnect app, confirm profile-specific frame behavior is visible in app debug tools

**Implemented Switch Mechanism**:
- Runtime BLE RX command parsing in firmware (`ble_rx_log_callback`) with profile queue handoff to TX task.
- Supported commands (case-insensitive substring match):
  - `PROFILE NOMINAL`
  - `PROFILE CLIMB`
  - `PROFILE SINK`
  - `PROFILE EDGE`
  - `PROFILE MALFORMED_CHECKSUM`
  - `PROFILE MALFORMED_SHAPE`
- Compile-time default preserved via `LK8EX1_SIM_PROFILE_DEFAULT` macro (`nominal` by default).

### Task 3.5.3: End-to-end validation with app frame inspector

**Description**: Validate simulated streams against the app frame-inspection screen and record evidence for integration sign-off.

**Acceptance Criteria**:
- [ ] App receives simulated LK8EX1 frames for all debug profiles
- [ ] App visual inspector can classify frames as valid/invalid by fields and checksum
- [ ] Evidence captured for each profile (sample frames + verdict)
- [ ] Final pass/fail verdict documented in both app and micro roadmaps

**Validation**:
- Run coordinated session with app debug screen, verify field-level interpretation and correctness verdicts

**Status Note (2026-02-25 — firmware side ready)**:
- Firmware now emits deterministic frames for all required debug profiles and keeps BLE/NUS streaming path unchanged.
- Pending closure items are app-side coordinated evidence capture and final shared verdict entry.

**Cross-Roadmap Verdict Sync (2026-02-25)**:
- App Task 3.5.4 matrix run completed with `8/8` PASS using `app/test/core/utils/lk8ex1_phase35_matrix_test.dart`.
- Shared Phase 3.5 verdict copied from app roadmap: **PASS** (host simulated matrix validation scope).
- Physical coordinated attempt (same date): `FlyInPeace` advertising/scan evidence is now **PASS**, but end-to-end A1–A8 app frame-capture evidence remains **FAIL** (pending interactive inspector run and BlueFlyVario source for A8).
- Compatibility rerun (same date): firmware rebuilt/flashed with `BLE_COMPAT_DEVICE_NAME="BlueFlyVario"`; host BLE scan evidence captured as `DC:DA:0C:81:52:26 BlueFlyVario`.
- Validation commands executed: `./scripts/micro/build.sh`, `./scripts/micro/test.sh` (`27/27` PASS), `./scripts/micro/flash.sh --force-release-port`.
- App roadmap sync (same date): Phase `3.6` BLE Session Recording implemented on app side with dual export (`.log` + `.csv`) and session metadata capture; no firmware code changes required.

### Task 3.5.4: Validate NUS TX stream through Linux app console mirror

**Description**: Close the TX observability loop using the app Linux console telemetry mirror to verify that ESP32-C3 notifications are received end-to-end in real time.

**Acceptance Criteria**:
- [ ] Firmware streams LK8EX1 frames and Linux app console prints them continuously while connected
- [ ] Validation covers at least `nominal`, `climb`, and `sink` profiles
- [ ] Captured evidence includes timestamped console lines and active firmware profile
- [ ] Cross-roadmap verdict synchronized with app Task 1.5.6

**Validation**:
- Flash firmware and run monitor
- Launch Linux app debug target and connect
- Collect console log evidence for each required profile
- Record PASS/FAIL in both roadmaps

### Shared Validation Matrix (micro ↔ app)

Use this matrix as the single source of truth for Phase 3.5 sign-off.

| Case | Profile | Example sentence expectation | App expected verdict | Notes |
|---|---|---|---|---|
| M1 | nominal | Stable pressure/altitude/vario, valid checksum | valid | Baseline integration gate |
| M2 | climb | Positive vario trend, coherent pressure drop | valid | Verify trend continuity for ≥ 30 s |
| M3 | sink | Negative vario trend, coherent pressure rise | valid | Verify no sign inversion in app fields |
| M4 | edge-placeholder-alt | `altitude=99999` | warning | Reason: placeholder altitude |
| M5 | edge-placeholder-bat | `battery=999` | warning | Reason: placeholder battery |
| M6 | malformed-checksum | Corrupted checksum | error | Parser must reject as invalid frame |
| M7 | malformed-shape | Missing field/count mismatch | error | Parser must flag malformed sentence |
| M8 | interoperability-bluefly | BlueFlyVario-style BLE source with LK8EX1-compatible payload | valid or warning | Valid if checksum/fields are correct |

### Coordinated Evidence Checklist (required)

- [ ] Session date/time recorded
- [ ] Firmware git hash + profile used recorded
- [ ] App git hash + debug screen version recorded
- [ ] BLE source profile recorded (`FlyInPeace` or `BlueFlyVario`)
- [ ] At least 5 captured frames per matrix case stored
- [ ] Verdict/result for each case (PASS/FAIL) recorded
- [ ] Final integration verdict copied to both roadmaps

### Execution Report Template (copy/paste)

Use this exact template at the end of each coordinated run:

```markdown
#### Phase 3.5 Coordinated Run Report
- Date/Time:
- Operator:
- Firmware hash/profile:
- App hash/build:
- BLE source (`FlyInPeace` | `BlueFlyVario`):

| Case | Expected | Observed | Verdict (PASS/FAIL) | Notes |
|---|---|---|---|---|
| M1/A1 nominal | valid |  |  |  |
| M2/A2 climb | valid |  |  |  |
| M3/A3 sink | valid |  |  |  |
| M4/A4 edge-placeholder-alt | warning |  |  |  |
| M5/A5 edge-placeholder-bat | warning |  |  |  |
| M6/A6 malformed-checksum | error |  |  |  |
| M7/A7 malformed-shape | error |  |  |  |
| M8/A8 interoperability-bluefly | valid/warning |  |  |  |

- Final verdict (overall): PASS / FAIL
- Blocking issues (if any):
- Next action:
```

### Quick Runbook (10 minutes)

1. **Prepare firmware stream (2 min)**
  - Build/flash firmware and start monitor.
  - Select/confirm simulation profile (`nominal`, `climb`, `sink`, `edge`).
2. **Start app inspection session (2 min)**
  - Open app frame inspector and connect over BLE.
  - Confirm first LK8EX1 frames arrive.
3. **Validate nominal/trend cases (3 min)**
  - Execute `M1/M2/M3` and confirm app verdict is `valid`.
  - Capture at least 5 frames per case.
4. **Validate edge/error cases (2 min)**
  - Execute `M4/M5` and confirm `warning`.
  - Execute `M6/M7` and confirm `error`.
5. **Interop + closure (1 min)**
  - Execute `M8` with BlueFlyVario-compatible source profile.
  - Fill Execution Report Template and copy final verdict to app roadmap.

### Command Pack (copy/paste)

Run from repository root (`.`):

```bash
# 1) Build + flash firmware
./scripts/micro/build.sh
./scripts/micro/flash.sh

# 2) Start monitor (new terminal)
./scripts/micro/monitor.sh

# 3) Optional quick BLE evidence (new terminal)
bluetoothctl --timeout 10 scan on || true
```
 
**Refactorización (obligatoria)**:
- Aplicar Boy Scout Rule al cerrar cada tarea de la fase.
- Reducir duplicación y complejidad accidental sin cambiar comportamiento funcional.
- Mantener nombres y límites de módulo claros para código autoexplicativo.
- Repetir la validación de la fase después de cada refactorización.

**Architecture Reference**: `firmware-architecture.md` §4.7 `led_indicator`, §8.3 LED State Machine

---

### Task 4.1: WS2812 driver via RMT peripheral

**Description**: Create the `led_indicator` component that drives the WS2812 RGB LED using the ESP32-C3's RMT peripheral.

**Acceptance Criteria**:
- [x] Component `led_indicator` created in `micro/components/led_indicator/`
- [x] `esp_err_t led_indicator_init(void)` — configures RMT channel on GPIO 8, creates LED task (Priority 1, 2048 bytes)
- [x] Internal functions to set RGB color and turn off LED via RMT
- [x] Uses ESP-IDF `led_strip` component or direct RMT encoding for WS2812 timing
- [x] GPIO 8 (WS2812 data pin on DevKitC-02 v1.1)

**Validation**:
- Flash firmware, LED lights up with a test color

**Status Note (2026-02-24 — implementation + build validation)**:
- New component added: `micro/components/led_indicator/`.
- Public API implemented: `led_indicator_init`, `led_indicator_deinit`, `led_indicator_set_state`, `led_indicator_get_state`.
- Internal split applied per project rule:
  - `led_indicator_conductor.c`
  - `led_indicator_model.c`
  - `led_indicator_hardware.c`
- WS2812 hardware backend implemented with `espressif/led_strip` over RMT on GPIO 8.
- Build validation:
  - `./scripts/micro/build.sh` ✅


**Files to create**:
- `micro/components/led_indicator/CMakeLists.txt`
- `micro/components/led_indicator/include/led_indicator.h`
- `micro/components/led_indicator/src/led_indicator.c`

---

### Task 4.2: LED state machine (patterns per `led_state_e`)

**Description**: Implement the LED state machine per architecture §8.3 with all defined states and patterns.

**Acceptance Criteria**:
- [x] `led_state_e` enum: `LED_STATE_BOOT`, `LED_STATE_BLE_DISCONNECTED`, `LED_STATE_BLE_CONNECTED`, `LED_STATE_WIFI_ENABLED`, `LED_STATE_ERROR`
- [x] `esp_err_t led_indicator_set_state(led_state_e state)` — thread-safe (queue-based, depth 1, overwrite)
- [x] `led_state_e led_indicator_get_state(void)` — returns current state
- [x] Pattern definitions per architecture §8.3:
  - `BOOT`: Blue solid (on during initialization)
  - `BLE_DISCONNECTED`: Red blink (100 ms ON / 1900 ms OFF)
  - `BLE_CONNECTED`: Green blink (100 ms ON / 4900 ms OFF)
  - `WIFI_ENABLED`: Blue blink (stub for future)
  - `ERROR`: Red fast blink (100 ms ON / 100 ms OFF)
- [x] LED task runs at 10 Hz (100 ms tick), evaluates on/off state within pattern cycle
- [x] Default state on boot: `LED_STATE_BOOT` → transitions to `LED_STATE_BLE_DISCONNECTED` after init

**Validation**:
- Boot → blue solid → red blink after init completes
- Verify all patterns with visual inspection

**Status Note (2026-02-24 — implementation + host validation)**:
- State machine implemented in `led_indicator_model.c` and consumed by a dedicated LED task (`10 Hz`) in `led_indicator_conductor.c`.
- Queue-based state updates implemented with depth `1` and `xQueueOverwrite` semantics.
- Pattern timing implemented exactly at 100 ms tick resolution:
  - `BOOT`: solid blue
  - `BLE_DISCONNECTED`: 1 tick ON / 19 ticks OFF
  - `BLE_CONNECTED`: 1 tick ON / 49 ticks OFF
  - `WIFI_ENABLED`: blue blink (stub)
  - `ERROR`: 1 tick ON / 1 tick OFF

---

### Task 4.3: Integration with BLE connection state

**Description**: Register a BLE state callback to automatically change LED state on connect/disconnect.

**Acceptance Criteria**:
- [x] `ble_nus_register_state_callback()` used to hook BLE state changes
- [x] BLE connect → `led_indicator_set_state(LED_STATE_BLE_CONNECTED)` (green)
- [x] BLE disconnect → `led_indicator_set_state(LED_STATE_BLE_DISCONNECTED)` (red)
- [x] `esp_err_t led_indicator_deinit(void)` cleans up task and RMT resources
- [x] Transition is immediate and visible

**Validation**:
- Connect/disconnect from phone, observe LED color changes

**Status Note (2026-02-24 — integration + regression validation)**:
- `main.c` now initializes `led_indicator` before BLE module setup.
- BLE state callback registered via `ble_nus_register_state_callback(...)` and mapped to LED states:
  - connected → `LED_STATE_BLE_CONNECTED`
  - disconnected → `LED_STATE_BLE_DISCONNECTED`
- Boot default transitions to `LED_STATE_BLE_DISCONNECTED` after module usage configuration.
- Main target updated to require `led_indicator` component.
- Regression validation:
  - `./scripts/micro/build.sh` ✅
  - `./scripts/micro/test.sh` ✅

---

## Phase 5: Sensor HAL (Compile-Time Abstraction)

**Objective**: Create the sensor hardware abstraction layer with compile-time driver selection via Kconfig. NO `i2c_bus` wrapper — sensor drivers use ESP-IDF I2C directly per architecture decision.  
**Estimated Duration**: 1–2 days  
**Dependencies**: Phase 1 (interface contract in `firmware-architecture.md` §4.1)  
 
**Refactorización (obligatoria)**:
- Aplicar Boy Scout Rule al cerrar cada tarea de la fase.
- Reducir duplicación y complejidad accidental sin cambiar comportamiento funcional.
- Mantener nombres y límites de módulo claros para código autoexplicativo.
- Convención de nombres por rol obligatoria: `*_conductor.c`, `*_model.c`, `*_hardware.c` (si aplica al módulo).
- Repetir la validación de la fase después de cada refactorización.

**Architecture Reference**: `firmware-architecture.md` §4.1 `sensor_hal`

---

### Task 5.1: Kconfig sensor selection (`choice SENSOR_DRIVER`)

**Description**: Create the Kconfig menu for compile-time sensor driver selection.

**Acceptance Criteria**:
- [ ] Component `sensor_hal` created in `micro/components/sensor_hal/`
- [ ] `sensor_hal/Kconfig` with `choice SENSOR_DRIVER` block per architecture §4.1
- [ ] Options: `CONFIG_SENSOR_MS5611` (default), `CONFIG_SENSOR_BMP390`
- [ ] Each option has help text with sensor specs (I2C address, resolution, accuracy)
- [ ] Selection visible in `idf.py menuconfig` under "Component config → Sensor driver"

**Validation**:
- `idf.py menuconfig` shows the sensor selection menu
- `sdkconfig` contains `CONFIG_SENSOR_MS5611=y` by default

**Files to create**:
- `micro/components/sensor_hal/Kconfig`
- `micro/components/sensor_hal/CMakeLists.txt`

---

### Task 5.2: `sensor_hal` public API and compile-time dispatch

**Description**: Implement the `sensor_hal` public API with `#if defined()` compile-time dispatch per architecture §4.1.

**Acceptance Criteria**:
- [ ] `sensor_data_t` struct per architecture: `pressure_pa` (int32), `temperature_mc` (int32), `timestamp_us` (int64)
- [ ] Public API per architecture contract:
  - `esp_err_t sensor_hal_init(void)` — configures I2C, reads calibration
  - `esp_err_t sensor_hal_read(sensor_data_t *out)` — full read cycle (trigger → wait → read → compensate)
  - `esp_err_t sensor_hal_deinit(void)` — releases I2C, powers down
  - `const char *sensor_hal_get_name(void)` — returns `"MS5611"` or `"BMP390"`
- [ ] `sensor_hal.c` uses `#if defined(CONFIG_SENSOR_MS5611)` / `#elif defined(CONFIG_SENSOR_BMP390)` dispatch
- [ ] `#else #error` if no sensor selected
- [ ] CMakeLists.txt conditionally adds `REQUIRES sensor_ms5611` or `sensor_bmp390` per architecture
- [ ] **NO function pointers, NO `void *ctx`** — compile-time dispatch only

**Validation**:
- Build succeeds with `CONFIG_SENSOR_MS5611=y`
- Build succeeds with `CONFIG_SENSOR_BMP390=y` (once Phase 7 exists; stub for now)
- Build fails with no sensor selected → `#error` message

**Files to create**:
- `micro/components/sensor_hal/include/sensor_hal.h`
- `micro/components/sensor_hal/src/sensor_hal.c`

---

### Task 5.3: I2C bus initialization

**Description**: Initialize the I2C master bus in `sensor_hal_init()` using ESP-IDF's I2C driver directly (no wrapper component). Per architecture, sensor drivers use ESP-IDF I2C directly.

**Acceptance Criteria**:
- [ ] I2C master bus configured in `sensor_hal_init()` before calling driver init
- [ ] I2C port: `I2C_NUM_0`, SDA: GPIO 6, SCL: GPIO 7, Clock: 400 kHz (per architecture §11.2)
- [ ] Pull-ups: configured via GPIO config (external 4.7 kΩ recommended)
- [ ] I2C bus released in `sensor_hal_deinit()`
- [ ] Uses ESP-IDF v5.x `i2c_master.h` API

**Validation**:
- Build succeeds
- (After Phase 6) I2C scan detects sensor at address 0x77

**Files to modify**:
- `micro/components/sensor_hal/src/sensor_hal.c`

**Notes**:
- **No `i2c_bus` wrapper component**: per architecture decision, sensor drivers use ESP-IDF I2C directly to minimize abstraction layers.
- I2C initialization happens once in `sensor_hal_init()`, then the driver handle is passed to the selected sensor driver.

---

## Phase 6: MS5611 Sensor Driver

**Objective**: Implement a fully functional MS5611 barometric pressure sensor driver with PROM calibration, ADC conversion, and second-order compensation.  
**Estimated Duration**: 3–4 days  
**Dependencies**: Phase 5 (sensor_hal + I2C init)  
 
**Refactorización (obligatoria)**:
- Aplicar Boy Scout Rule al cerrar cada tarea de la fase.
- Reducir duplicación y complejidad accidental sin cambiar comportamiento funcional.
- Mantener nombres y límites de módulo claros para código autoexplicativo.
- Convención de nombres por rol obligatoria: `*_conductor.c`, `*_model.c`, `*_hardware.c` (si aplica al módulo).
- Repetir la validación de la fase después de cada refactorización.

**Architecture Reference**: `firmware-architecture.md` §4.2 `sensor_ms5611`

---

### Task 6.1: MS5611 PROM calibration read

**Description**: Implement reading the 6 factory calibration coefficients (C1–C6) from the MS5611's PROM via I2C. These are needed for pressure/temperature compensation.

**Acceptance Criteria**:
- [ ] Component `sensor_ms5611` created in `micro/components/sensor_ms5611/`
- [ ] `sensor_ms5611_t` and `sensor_ms5611_cfg_t` structs per architecture §4.2
- [ ] `sensor_ms5611_init(sensor_ms5611_t *self, const sensor_ms5611_cfg_t *cfg)` reads all 6 PROM coefficients
- [ ] Sends reset command (`0x1E`) before PROM read
- [ ] Calibration data stored in `self->calibration[6]`
- [ ] Validates PROM CRC (word 7)
- [ ] Handles I2C errors (retry once, then return error)
- [ ] Logs calibration values at INFO level on successful init
- [ ] Uses ESP-IDF I2C driver directly (no wrapper)

**Validation**:
- Flash to DevKitC-02 with MS5611 connected, verify calibration values in log output

**Files to create**:
- `micro/components/sensor_ms5611/CMakeLists.txt`
- `micro/components/sensor_ms5611/include/sensor_ms5611.h`
- `micro/components/sensor_ms5611/src/sensor_ms5611.c`

**Notes**:
- MS5611 I2C address: `0x77` (CSB low) or `0x76` (CSB high). Default: `0x77`.
- PROM read commands: `0xA0` to `0xAE` (8 words, 16-bit each; C1–C6 are words 1–6).
- Reset command: `0x1E` — send before PROM read.

---

### Task 6.2: MS5611 raw pressure & temperature read

**Description**: Implement the ADC conversion and raw data read for pressure (D1) and temperature (D2).

**Acceptance Criteria**:
- [ ] `sensor_ms5611_read(sensor_ms5611_t *self, sensor_data_t *out)` performs full read cycle
- [ ] Starts D1 (pressure) ADC conversion, waits, reads 24-bit result
- [ ] Starts D2 (temperature) ADC conversion, waits, reads 24-bit result
- [ ] Configurable OSR per `sensor_ms5611_cfg_t.osr` (256, 512, 1024, 2048, 4096)
- [ ] Default OSR: 4096 (~9.04 ms conversion time per measurement)
- [ ] Populates `out->timestamp_us` with `esp_timer_get_time()`
- [ ] Handles I2C read errors

**Validation**:
- Flash to hardware, log raw D1 and D2 values, verify non-zero and in expected range

**Files to modify**:
- `micro/components/sensor_ms5611/src/sensor_ms5611.c`

**Notes**:
- Conversion commands: `0x40 + 2*OSR_index` (pressure), `0x50 + 2*OSR_index` (temperature)
- ADC read command: `0x00` — returns 3 bytes (24-bit value)
- Total read cycle for both P+T at OSR 4096: ~20 ms → allows 10 Hz with margin

---

### Task 6.3: MS5611 compensation math

**Description**: Implement the second-order temperature compensation algorithm from the MS5611 datasheet.

**Acceptance Criteria**:
- [ ] Full compensation per datasheet (including second-order for T < 20°C and T < -15°C)
- [ ] Output pressure in Pascals → `out->pressure_pa` (`int32_t`)
- [ ] Output temperature in milli-Celsius → `out->temperature_mc` (`int32_t`, e.g., 23500 = 23.5°C)
- [ ] Uses 64-bit intermediate calculations to avoid overflow
- [ ] Pure computation (no I2C calls) — separable for unit testing
- [ ] `sensor_ms5611_deinit(sensor_ms5611_t *self)` releases resources

**Validation**:
- Datasheet test vector: C1=40127, C2=36924, C3=23317, C4=23282, C5=33464, C6=28312, D1=9085466, D2=8569150 → TEMP=2007, P=100009

**Files to modify**:
- `micro/components/sensor_ms5611/src/sensor_ms5611.c`

---

### Task 6.4: Ceedling unit tests for compensation

**Description**: Write unit tests for the MS5611 compensation math using known test vectors.

**Acceptance Criteria**:
- [ ] Test file `micro/test/test_sensor_ms5611.c` exists
- [ ] Test: datasheet reference vector produces expected P and T values
- [ ] Test: second-order compensation activates for T < 20°C
- [ ] Test: second-order compensation activates for T < -15°C
- [ ] Test: init with NULL parameters returns `ESP_ERR_INVALID_ARG`
- [ ] All tests pass in `ceedling test:all`

**Validation**:
- Run `./scripts/micro/test.sh` — all tests green

**Files to create**:
- `micro/test/test_sensor_ms5611.c`

**Notes**: Mock the I2C layer with CMock. The compensation math should be testable by providing known raw values.

---

### Task 6.5: Integration test on hardware

**Description**: Run the MS5611 driver on actual hardware through `sensor_hal` and verify readings are reasonable.

**Acceptance Criteria**:
- [ ] Pressure readings in range 30000–110000 Pa (300–1100 mbar)
- [ ] Temperature readings reasonable (e.g., 15–35°C indoors → 15000–35000 milli-°C)
- [ ] Readings stable (±10 Pa over 10 seconds at rest)
- [ ] 10 Hz read rate achieved without I2C errors
- [ ] `sensor_hal_get_name()` returns `"MS5611"`
- [ ] Log output shows formatted pressure and temperature values

**Validation**:
- Flash firmware, observe sensor readings via `sensor_hal_read()` in serial monitor for 60 seconds
- Compare pressure reading with known altitude / weather station data

**Files to modify**:
- `micro/main/main.c` (temporary test loop: call `sensor_hal_init()`, loop `sensor_hal_read()` at 10 Hz)

---

## Phase 7: BMP390 Sensor Driver

**Objective**: Implement a fully functional BMP390 barometric pressure sensor driver with NVM trimming, compensation, and IIR filter support.  
**Estimated Duration**: 3–4 days  
**Dependencies**: Phase 5 (sensor_hal + I2C init)  
 
**Refactorización (obligatoria)**:
- Aplicar Boy Scout Rule al cerrar cada tarea de la fase.
- Reducir duplicación y complejidad accidental sin cambiar comportamiento funcional.
- Mantener nombres y límites de módulo claros para código autoexplicativo.
- Convención de nombres por rol obligatoria: `*_conductor.c`, `*_model.c`, `*_hardware.c` (si aplica al módulo).
- Repetir la validación de la fase después de cada refactorización.

**Architecture Reference**: `firmware-architecture.md` §4.3 `sensor_bmp390`

---

### Task 7.1: BMP390 trimming coefficients read

**Description**: Implement reading the 11 NVM trimming coefficients from the BMP390 and validate the chip ID.

**Acceptance Criteria**:
- [ ] Component `sensor_bmp390` created in `micro/components/sensor_bmp390/`
- [ ] `sensor_bmp390_t` and `sensor_bmp390_cfg_t` structs per architecture §4.3
- [ ] `sensor_bmp390_init(sensor_bmp390_t *self, const sensor_bmp390_cfg_t *cfg)` implemented
- [ ] Reads and validates chip ID register (expected: `0x60`)
- [ ] Returns `ESP_ERR_NOT_FOUND` if chip ID doesn't match
- [ ] Reads 11 trimming coefficients from NVM (par_t1..par_t3, par_p1..par_p11)
- [ ] Converts raw NVM bytes to `float` coefficients per Bosch datasheet
- [ ] Configures OSR, ODR, and IIR filter settings per `sensor_bmp390_cfg_t`
- [ ] Handles I2C errors (retry once, then return error)
- [ ] Logs sensor info at INFO level on successful init
- [ ] Uses ESP-IDF I2C driver directly (no wrapper)

**Validation**:
- Flash to DevKitC-02 with BMP390 connected, verify chip ID and coefficients in log output

**Files to create**:
- `micro/components/sensor_bmp390/CMakeLists.txt`
- `micro/components/sensor_bmp390/include/sensor_bmp390.h`
- `micro/components/sensor_bmp390/src/sensor_bmp390.c`

**Notes**:
- BMP390 I2C address: `0x77` (SDO=GND) or `0x76` (SDO=VCC). Default: `0x77`.
- Chip ID register: `0x00`, expected value: `0x60`.
- NVM trimming data: registers `0x31`–`0x45` (21 bytes → 11 coefficients).

---

### Task 7.2: BMP390 raw pressure & temperature read

**Description**: Implement forced measurement mode and raw data read for pressure and temperature.

**Acceptance Criteria**:
- [ ] `sensor_bmp390_read(sensor_bmp390_t *self, sensor_data_t *out)` performs full read cycle
- [ ] Sets forced mode in PWR_CTRL register (`0x1B`)
- [ ] Waits for data ready (poll STATUS register `0x03`, bit 5+6)
- [ ] Reads 24-bit raw pressure and 24-bit raw temperature from data registers
- [ ] Configurable OSR_P and OSR_T per `sensor_bmp390_cfg_t` (1x, 2x, 4x, 8x, 16x, 32x)
- [ ] Default: OSR_P = 8x, OSR_T = 1x
- [ ] IIR filter coefficient configurable (default: 3)
- [ ] Populates `out->timestamp_us` with `esp_timer_get_time()`

**Validation**:
- Flash to hardware, log raw pressure and temperature values

**Files to modify**:
- `micro/components/sensor_bmp390/src/sensor_bmp390.c`

**Notes**:
- Conversion time depends on OSR: ~5 ms (1x) to ~40 ms (32x).
- Data registers: pressure `0x04`–`0x06`, temperature `0x07`–`0x09`.

---

### Task 7.3: BMP390 compensation math

**Description**: Implement the compensation algorithm per Bosch BMP390 datasheet using float arithmetic.

**Acceptance Criteria**:
- [ ] Full compensation algorithm using 11 trimming coefficients
- [ ] Output pressure in Pascals → `out->pressure_pa` (`int32_t`)
- [ ] Output temperature in milli-Celsius → `out->temperature_mc` (`int32_t`)
- [ ] Uses `float` arithmetic (ESP32-C3 has no FPU; `float` is faster than `double` in software)
- [ ] Pure computation — separable for unit testing
- [ ] `sensor_bmp390_deinit(sensor_bmp390_t *self)` releases resources

**Validation**:
- Compare output with Bosch reference implementation / BMP3 API

**Files to modify**:
- `micro/components/sensor_bmp390/src/sensor_bmp390.c`

---

### Task 7.4: Ceedling unit tests for compensation

**Description**: Write unit tests for the BMP390 compensation math.

**Acceptance Criteria**:
- [ ] Test file `micro/test/test_sensor_bmp390.c` exists
- [ ] Test: known trimming coefficients + raw values produce expected P and T
- [ ] Test: init with NULL parameters returns `ESP_ERR_INVALID_ARG`
- [ ] Test: chip ID validation (correct ID vs wrong ID)
- [ ] All tests pass in `ceedling test:all`

**Validation**:
- Run `./scripts/micro/test.sh` — all tests green

**Files to create**:
- `micro/test/test_sensor_bmp390.c`

---

### Task 7.5: Integration test on hardware

**Description**: Run the BMP390 driver on actual hardware through `sensor_hal` and verify readings.

**Acceptance Criteria**:
- [ ] Pressure readings in range 30000–125000 Pa (300–1250 hPa)
- [ ] Temperature readings reasonable (15000–35000 milli-°C indoors)
- [ ] Readings stable (noise ≤ ±3 Pa at rest — BMP390 is more precise than MS5611)
- [ ] 10 Hz read rate achieved without I2C errors
- [ ] `sensor_hal_get_name()` returns `"BMP390"`
- [ ] Sensor selected via `idf.py menuconfig` → `CONFIG_SENSOR_BMP390=y`

**Validation**:
- Flash firmware with `CONFIG_SENSOR_BMP390=y`, observe readings in serial monitor
- Compare with MS5611 readings (if both sensors available)

**Files to modify**:
- `micro/main/main.c` (same test loop as Phase 6, but with BMP390 selected)

---

## Phase 8: Kalman Filter

**Objective**: Implement a 2-state Kalman filter (altitude + vario) to smooth pressure readings and derive altitude and vertical speed.  
**Estimated Duration**: 2–3 days  
**Dependencies**: Phase 6 or Phase 7 (compensated pressure data for validation)  
 
**Refactorización (obligatoria)**:
- Aplicar Boy Scout Rule al cerrar cada tarea de la fase.
- Reducir duplicación y complejidad accidental sin cambiar comportamiento funcional.
- Mantener nombres y límites de módulo claros para código autoexplicativo.
- Convención de nombres por rol obligatoria: `*_conductor.c`, `*_model.c`, `*_hardware.c` (si aplica al módulo).
- Repetir la validación de la fase después de cada refactorización.

**Architecture Reference**: `firmware-architecture.md` §4.4 `kalman_filter`

---

### Task 8.1: 2-state Kalman filter implementation

**Description**: Implement the 2-state Kalman filter per architecture contract §4.4.

**Acceptance Criteria**:
- [ ] Component `kalman_filter` created in `micro/components/kalman_filter/`
- [ ] `kalman_cfg_t` struct: `q_altitude` (default 0.01), `q_vario` (default 0.01), `r_measurement` (default 0.5), `reference_pressure_pa` (default 101325.0)
- [ ] `kalman_state_t` struct: `altitude_m`, `vario_ms`, `p[2][2]` covariance, `last_timestamp_us`, `initialized` flag
- [ ] `kalman_filter_init(state, cfg)` — sets initial state, covariance = identity
- [ ] `kalman_filter_update(state, cfg, pressure_pa, timestamp_us)` — predict + correct step, uses `cfg->reference_pressure_pa` as P0
- [ ] First call sets altitude from pressure, marks `initialized = true` (no predict step)
- [ ] `kalman_filter_reset(state)` — clears state
- [ ] `kalman_filter_calibrate(cfg, state, known_altitude_m, current_pressure_pa)` — computes new P0 via inverse barometric formula, stores in `cfg->reference_pressure_pa`, resets filter state
- [ ] `calibrate()` validates: altitude ∈ [-500, 10000] m, pressure ∈ [20000, 120000] Pa
- [ ] Pure C, no ESP-IDF dependencies (fully host-testable)
- [ ] All math uses `float` (not `double`)

**Validation**:
- Unit tests show convergence and correct vario derivation

**Files to create**:
- `micro/components/kalman_filter/CMakeLists.txt`
- `micro/components/kalman_filter/include/kalman_filter.h`
- `micro/components/kalman_filter/src/kalman_filter.c`

**Notes**:
- State vector: `x = [altitude, vario]`. Prediction uses constant-velocity model.
- Measurement: altitude derived from pressure. Vario is estimated by the filter.
- Constant-velocity model: altitude_predicted = altitude + vario × dt

---

### Task 8.2: Altitude calculation from pressure

**Description**: Implement the barometric formula for converting pressure to altitude.

**Acceptance Criteria**:
- [ ] Internal function converts `pressure_pa` to altitude in meters
- [ ] Uses ISA barometric formula: $h = 44330 \times (1 - (P/P_0)^{0.1903})$
- [ ] Reference pressure $P_0$ taken from `kalman_cfg_t.reference_pressure_pa` (default: 101325 Pa)
- [ ] Input: `float pressure_pa`, output: `float altitude_m`
- [ ] Pure function, no side effects

**Validation**:
- 101325 Pa → 0 m, 89876 Pa → ~1000 m, 79501 Pa → ~2000 m

---

### Task 8.3: Altitude calibration (inverse barometric formula)

**Description**: Implement `kalman_filter_calibrate()` to derive a new reference pressure (QNH) from a known altitude and current pressure, enabling barometric altimeter calibration.

**Acceptance Criteria**:
- [ ] `esp_err_t kalman_filter_calibrate(kalman_cfg_t *cfg, kalman_state_t *state, float known_altitude_m, float current_pressure_pa)` implemented
- [ ] Computes P0 using inverse barometric formula: $P_0 = P / (1 - h/44330)^{5.255}$
- [ ] Stores result in `cfg->reference_pressure_pa`
- [ ] Resets filter state (`kalman_filter_reset`) so next update uses new P0 immediately
- [ ] Validates inputs: `known_altitude_m` ∈ [-500, 10000], `current_pressure_pa` ∈ [20000, 120000]
- [ ] Returns `ESP_ERR_INVALID_ARG` for out-of-range values; P0 unchanged on error
- [ ] Pure function, no ESP-IDF dependencies

**Validation**:
- Calibrate at sea level (0 m, 101325 Pa) → P0 = 101325
- Calibrate at 500 m with 95461 Pa → P0 ≈ 101325
- Calibrate at 1000 m with 89876 Pa → P0 ≈ 101325
- After calibration, `kalman_filter_update()` produces altitude matching known value

**Files to modify**:
- `micro/components/kalman_filter/include/kalman_filter.h`
- `micro/components/kalman_filter/src/kalman_filter.c`

---

### Task 8.4: Vario (vertical speed) derivation

**Description**: The Kalman filter directly estimates vario as its second state variable. Verify it produces correct vertical speed values.

**Acceptance Criteria**:
- [ ] Vario output in m/s (`float`) from `kalman_state_t.vario_ms`
- [ ] Constant pressure input → vario converges to 0 m/s
- [ ] Linearly decreasing pressure → positive vario (ascending)
- [ ] Linearly increasing pressure → negative vario (descending)
- [ ] Vario converted to cm/s (`int32_t`) when passed to `lk8ex1_data_t.vario_cms`

**Validation**:
- Synthetic test data with known altitude trajectories

---

### Task 8.5: Ceedling unit tests with synthetic data

**Description**: Write comprehensive unit tests for the Kalman filter.

**Acceptance Criteria**:
- [ ] Test file `micro/test/test_kalman_filter.c` exists
- [ ] Test: filter converges to true value with noisy sinusoidal input
- [ ] Test: constant input → altitude stable, vario ≈ 0
- [ ] Test: altitude formula produces correct results for known pressures
- [ ] Test: ascending pressure sequence → positive vario
- [ ] Test: descending pressure sequence → negative vario
- [ ] Test: `kalman_filter_reset()` clears state properly
- [ ] Test: `kalman_filter_calibrate()` with known altitude derives correct P0
- [ ] Test: `kalman_filter_calibrate()` rejects out-of-range altitude/pressure
- [ ] Test: after calibration, `update()` produces altitude matching known value
- [ ] Test: init with NULL state returns `ESP_ERR_INVALID_ARG`
- [ ] All tests pass in `ceedling test:all`

**Validation**:
- Run `./scripts/micro/test.sh` — all tests green

**Files to create**:
- `micro/test/test_kalman_filter.c`

---

## Phase 9: Data Pipeline

**Objective**: Wire up the FreeRTOS task model defined in the architecture: sensor_task reads the sensor at 10 Hz, runs the Kalman filter, and publishes to the shared flight data structure. ble_sender_task reads at 4 Hz, formats LK8EX1, and sends over BLE.  
**Estimated Duration**: 3–4 days  
**Dependencies**: Phases 3 (BLE), 6 or 7 (sensor), 8 (Kalman), 2 (LK8EX1)  
 
**Refactorización (obligatoria)**:
- Aplicar Boy Scout Rule al cerrar cada tarea de la fase.
- Reducir duplicación y complejidad accidental sin cambiar comportamiento funcional.
- Mantener nombres y límites de módulo claros para código autoexplicativo.
- Convención de nombres por rol obligatoria: `*_conductor.c`, `*_model.c`, `*_hardware.c` (si aplica al módulo).
- Repetir la validación de la fase después de cada refactorización.

**Architecture Reference**: `firmware-architecture.md` §5 Tasks, §6 Inter-Task Communication, §7 Data Flow

---

### Task 9.1: Shared flight data structure and mutex

**Description**: Implement the `shared_flight_data_t` struct and access primitives per architecture §6.1.

**Acceptance Criteria**:
- [ ] `shared_flight_data_t` struct per architecture §6.1: `altitude_m`, `vario_ms`, `pressure_pa`, `temperature_mc`, `reference_pressure_pa`, `timestamp_us`, `sensor_valid`
- [ ] Mutex created with `xSemaphoreCreateMutex()`
- [ ] Writer API (sensor_task): `xSemaphoreTake` → write all fields → `xSemaphoreGive`
- [ ] Reader API (ble_sender): `xSemaphoreTake` → copy struct → `xSemaphoreGive`
- [ ] Mutex timeout: `pdMS_TO_TICKS(10)` to avoid deadlocks
- [ ] Defined in a shared header accessible to both tasks

**Validation**:
- Build succeeds, mutex created in `app_main()`

**Files to create/modify**:
- `micro/main/flight_data.h` (shared struct + mutex extern)
- `micro/main/main.c` (mutex creation in `app_main()`)

---

### Task 9.2: Calibration queue (sensor_task consumer)

**Description**: Implement the `calibration_queue` per architecture §6.3 — a depth-1 queue that routes altitude calibration requests from `config_task` to `sensor_task`.

**Acceptance Criteria**:
- [ ] `calibration_request_t` struct: `known_altitude_m` (`float`)
- [ ] Queue: `xQueueCreate(1, sizeof(calibration_request_t))` — depth 1, overwrite mode
- [ ] Producer: `config_task` (on `CONFIG_REQUEST_CALIBRATE`)
- [ ] Consumer: `sensor_task` (non-blocking poll with `xQueueReceive(..., 0)` at start of each cycle)
- [ ] On receive: `sensor_task` calls `kalman_filter_calibrate()` with known altitude + last pressure reading
- [ ] After calibration: `config_manager_save()` persists new `reference_pressure_pa`
- [ ] Queue created in `app_main()` alongside other queues

**Validation**:
- Send calibration request via BLE → sensor_task processes → altitude now matches known value
- New P0 persists across reboot

**Files to create/modify**:
- `micro/main/flight_data.h` (add `calibration_request_t` and queue extern)
- `micro/main/sensor_task.c` (non-blocking poll at start of loop)
- `micro/main/config_task.c` (post to calibration_queue)
- `micro/main/main.c` (queue creation)

---

### Task 9.3: Sensor reader task (10 Hz)

**Description**: Implement `sensor_task_fn` per architecture §5.2, running at 10 Hz.

**Acceptance Criteria**:
- [ ] `sensor_task` created with Priority 5, stack 4096 bytes (per architecture §5.1)
- [ ] Loop every 100 ms:
  1. Check `calibration_queue` for pending calibration (non-blocking poll)
     → If received: `kalman_filter_calibrate()` + `config_manager_save()` to persist new P0
  2. `sensor_hal_read(&sensor_data)` — blocks ~18 ms for sensor conversion
  3. `kalman_filter_update(&state, &cfg, sensor_data.pressure_pa, sensor_data.timestamp_us)`
  4. `xSemaphoreTake(mutex)` → copy Kalman state + sensor data to `shared_flight_data` → `xSemaphoreGive(mutex)`
  5. `vTaskDelay(remaining time to hit 100 ms period)`
- [ ] Handles sensor read errors: skip Kalman update, set `sensor_valid = false` after 3 consecutive failures
- [ ] Registered with Task Watchdog Timer (TWDT), fed at end of each cycle
- [ ] TWDT timeout: 5 seconds (per architecture §10.3)

**Validation**:
- Log output shows sensor reads at 10 Hz
- Kalman altitude/vario values update continuously

**Files to create/modify**:
- `micro/main/sensor_task.c`
- `micro/main/sensor_task.h`
- `micro/main/main.c` (task creation)

---

### Task 9.4: BLE sender task (4 Hz)

**Description**: Implement `ble_sender_task_fn` per architecture §5.2, running at 4 Hz.

**Acceptance Criteria**:
- [ ] `ble_sender_task` created with Priority 3, stack 4096 bytes (per architecture §5.1)
- [ ] Loop every 250 ms:
  1. `xSemaphoreTake(mutex)` → copy `shared_flight_data` → `xSemaphoreGive(mutex)`
  2. Build `lk8ex1_data_t` from flight data (convert vario m/s → cm/s, temperature milli-°C → deci-°C)
  3. `lk8ex1_format(&data, buffer, sizeof(buffer))`
  4. `ble_nus_send((uint8_t *)buffer, strlen(buffer))`
  5. `vTaskDelay(remaining time to hit 250 ms period)`
- [ ] Silently skips send if BLE not connected (`ble_nus_is_connected()` or `ESP_ERR_INVALID_STATE`)
- [ ] Handles `sensor_valid == false`: sends LK8EX1 with `altitude=99999, vario=0`

**Validation**:
- Connect with nRF Connect, verify LK8EX1 sentences arriving at ~4 Hz with real sensor data

**Files to create/modify**:
- `micro/main/ble_sender_task.c`
- `micro/main/ble_sender_task.h`
- `micro/main/main.c` (task creation)

---

### Task 9.5: Replace simulated provider with real sensor data

**Description**: Remove the temporary simulated LK8EX1 sender from Phase 3 and use the real pipeline.

**Acceptance Criteria**:
- [ ] Temporary simulation code in `main.c` removed
- [ ] `app_main()` orchestrates initialization in correct order:
  1. `led_indicator_init()` → `LED_STATE_BOOT`
  2. `sensor_hal_init()`
  3. `kalman_filter_init()`
  4. `ble_nus_init()`
  5. Register BLE state callback → LED
  6. Create `sensor_task`, `ble_sender_task`
  7. `led_indicator_set_state(LED_STATE_BLE_DISCONNECTED)`
- [ ] All tasks running with correct priorities

**Validation**:
- Boot → sensor reads start → BLE advertises → connect → real LK8EX1 data flows

**Files to modify**:
- `micro/main/main.c`

---

### Task 9.6: End-to-end data flow validation

**Description**: Validate the complete pipeline from sensor to BLE, matching the timing budget in architecture §7.3.

**Acceptance Criteria**:
- [ ] Sensor reads at 10 Hz (±5% jitter)
- [ ] BLE sends at 4 Hz (±5% jitter)
- [ ] LK8EX1 sentences contain real pressure, altitude, vario, temperature
- [ ] Total sensor cycle ≤ 20 ms (budget: 18.5 ms per architecture §7.3)
- [ ] System runs stably for 30+ minutes without crashes, memory leaks, or watchdog resets
- [ ] Stack high-water marks checked for all tasks (should be >25% remaining)
- [ ] Free heap monitored (should not decrease over time)

**Validation**:
- Monitor serial output for 30 minutes
- `uxTaskGetStackHighWaterMark()` for each task
- `esp_get_free_heap_size()` at boot and after 30 minutes

---

## Phase 10: NVS Configuration

**Objective**: Implement persistent device configuration with NVS storage and expose a BLE Config Service for remote configuration.  
**Estimated Duration**: 3–4 days  
**Dependencies**: Phase 3 (BLE NUS for GATT server), Phase 9 (data pipeline for applying config)  
 
**Refactorización (obligatoria)**:
- Aplicar Boy Scout Rule al cerrar cada tarea de la fase.
- Reducir duplicación y complejidad accidental sin cambiar comportamiento funcional.
- Mantener nombres y límites de módulo claros para código autoexplicativo.
- Convención de nombres por rol obligatoria: `*_conductor.c`, `*_model.c`, `*_hardware.c` (si aplica al módulo).
- Repetir la validación de la fase después de cada refactorización.

**Architecture Reference**: `firmware-architecture.md` §4.8 `config_manager`, `ble_protocol.md` §Config Service

---

### Task 10.1: Config schema definition and defaults

**Description**: Define the `device_config_t` struct and default values per architecture §4.8.

**Acceptance Criteria**:
- [ ] Component `config_manager` created in `micro/components/config_manager/`
- [ ] `device_config_t` struct per architecture: `sensor_rate_hz` (10), `ble_tx_rate_hz` (4), `kalman_q` (0.01), `kalman_r` (0.5), `reference_pressure_pa` (101325.0), `device_name` ("FlyInPeace"), `wifi_enabled` (false)
- [ ] `const device_config_t *config_manager_get_defaults(void)` returns pointer to static defaults
- [ ] Validation rules per architecture: `sensor_rate_hz` ∈ [1,100], `ble_tx_rate_hz` ∈ [1,50], etc.
- [ ] Internal validation function: returns `ESP_ERR_INVALID_ARG` for out-of-range values

**Validation**:
- Unit tests verify defaults are correct and validation rejects invalid values

**Files to create**:
- `micro/components/config_manager/CMakeLists.txt`
- `micro/components/config_manager/include/config_manager.h`
- `micro/components/config_manager/src/config_manager.c`

---

### Task 10.2: NVS read/write with validation

**Description**: Implement persistent configuration storage in NVS.

**Acceptance Criteria**:
- [ ] `esp_err_t config_manager_init(void)` — opens NVS namespace `"fip_config"`, writes defaults if no config exists
- [ ] `esp_err_t config_manager_load(device_config_t *config)` — reads all fields; uses default for any field with read error
- [ ] `esp_err_t config_manager_save(const device_config_t *config)` — validates all fields before writing; returns `ESP_ERR_INVALID_ARG` on failure; does NOT apply partial writes
- [ ] `esp_err_t config_manager_reset_defaults(void)` — overwrites NVS with defaults
- [ ] Thread-safe: internal mutex protects NVS access

**Validation**:
- Save config, reboot, load config — values persist
- Save invalid config → returns error, NVS unchanged

**Files to modify**:
- `micro/components/config_manager/src/config_manager.c`

---

### Task 10.3: BLE Config Service GATT (read/write characteristics)

**Description**: Add the Config Service to the BLE GATT server per `ble_protocol.md`.

**Acceptance Criteria**:
- [ ] Config Service GATT registered alongside NUS in `ble_nus_init()`
- [ ] Config read characteristic: returns current `device_config_t` as JSON (includes `reference_pressure_pa`)
- [ ] Config write characteristic: accepts JSON payload, parses, validates, saves
- [ ] Write handler detects `"action": "calibrate"` payloads per `ble_protocol.md` §Calibrate Action
  - [ ] Parses `altitude_m` from JSON, validates range [-500, 10000]
  - [ ] Posts `CONFIG_REQUEST_CALIBRATE` (with `altitude_m` in payload) to config queue
- [ ] Regular config writes (without `action` field) post `CONFIG_REQUEST_WRITE` to config queue
- [ ] Write response includes success/error status

**Validation**:
- Read config via nRF Connect → JSON with current values including `reference_pressure_pa`
- Write config via nRF Connect → values saved and applied
- Write `{"action": "calibrate", "altitude_m": 450}` via nRF Connect → ack returned, altitude updates in LK8EX1 stream

**Files to modify**:
- `micro/components/ble_nus/src/ble_nus.c` (add Config GATT service)
- `micro/components/ble_nus/include/ble_nus.h` (config service types)

**Notes**: Config Service UUID defined in `ble_protocol.md` §Config Service.

---

### Task 10.4: Config task (event-driven)

**Description**: Implement the `config_task` per architecture §5.2 — event-driven via queue.

**Acceptance Criteria**:
- [ ] `config_task` created with Priority 2, stack 2048 bytes
- [ ] Blocks on `xQueueReceive(config_queue, &request, portMAX_DELAY)`
- [ ] Handles request types per architecture §6.2: `CONFIG_REQUEST_READ`, `CONFIG_REQUEST_WRITE`, `CONFIG_REQUEST_RESET`, `CONFIG_REQUEST_CALIBRATE`
- [ ] On `CONFIG_REQUEST_WRITE`: validate → save → apply changes to running system (e.g., update Kalman Q/R, BLE device name)
- [ ] On `CONFIG_REQUEST_RESET`: reset defaults → restart system
- [ ] On `CONFIG_REQUEST_CALIBRATE`: extract known altitude from payload → post to `calibration_queue` (consumed by `sensor_task`) → send ack
- [ ] Queue: `xQueueCreate(4, sizeof(config_request_t))`

**Validation**:
- Write config via BLE → config_task processes → values applied and persisted

**Files to create/modify**:
- `micro/main/config_task.c`
- `micro/main/config_task.h`
- `micro/main/main.c` (task + queue creation)

---

### Task 10.5: Ceedling unit tests for config validation

**Description**: Write unit tests for config validation logic.

**Acceptance Criteria**:
- [ ] Test file `micro/test/test_config_manager.c` exists
- [ ] Test: defaults are within valid ranges
- [ ] Test: `sensor_rate_hz = 0` rejected
- [ ] Test: `sensor_rate_hz = 101` rejected
- [ ] Test: `reference_pressure_pa = 79999` rejected
- [ ] Test: `reference_pressure_pa = 120001` rejected
- [ ] Test: `kalman_q = 0` rejected
- [ ] Test: `device_name` empty rejected
- [ ] Test: `device_name` > 20 chars rejected
- [ ] Test: all valid values accepted
- [ ] All tests pass in `ceedling test:all`

**Validation**:
- Run `./scripts/micro/test.sh` — all tests green

**Files to create**:
- `micro/test/test_config_manager.c`

---

## Phase 11: Power Optimization

**Objective**: Optimize power consumption for battery operation using light-sleep, BLE interval tuning, and peripheral gating.  
**Estimated Duration**: 2–3 days  
**Dependencies**: Phase 9 (data pipeline running), Phase 10 (config for tuning)  
 
**Refactorización (obligatoria)**:
- Aplicar Boy Scout Rule al cerrar cada tarea de la fase.
- Reducir duplicación y complejidad accidental sin cambiar comportamiento funcional.
- Mantener nombres y límites de módulo claros para código autoexplicativo.
- Repetir la validación de la fase después de cada refactorización.

**Architecture Reference**: `firmware-architecture.md` §4.9 `power_manager`, §7.3 Timing Budget

---

### Task 11.1: Light-sleep between sensor reads

**Description**: Enable automatic light-sleep using ESP-IDF power management.

**Acceptance Criteria**:
- [ ] Component `power_manager` created in `micro/components/power_manager/`
- [ ] `esp_err_t power_manager_init(void)` — configures `esp_pm_configure()` with `light_sleep_enable = true`
- [ ] `esp_err_t power_manager_enable_light_sleep(bool enable)` — dynamically toggles automatic light-sleep
- [ ] FreeRTOS tickless idle handles the sleep/wake cycle when PM is enabled
- [ ] System enters light-sleep during ~81.5 ms idle gap per sensor cycle (per architecture §7.3 timing budget)
- [ ] Light-sleep does not interfere with BLE advertising or connections

**Validation**:
- Verify with power meter: current drops significantly during sleep periods
- System still responds to BLE connections during light-sleep

**Files to create**:
- `micro/components/power_manager/CMakeLists.txt`
- `micro/components/power_manager/include/power_manager.h`
- `micro/components/power_manager/src/power_manager.c`

---

### Task 11.2: BLE connection interval optimization

**Description**: Optimize BLE connection parameters to reduce radio-on time while maintaining acceptable latency.

**Acceptance Criteria**:
- [ ] Request connection interval: 200–400 ms (lower power than default 30 ms)
- [ ] Slave latency: 0 (respond to every connection event)
- [ ] Supervision timeout: 4000 ms
- [ ] Connection parameter update request sent after connection established
- [ ] Verify data still arrives at 4 Hz (BLE sender rate)

**Validation**:
- `ble_gap_conn_params_update()` succeeds without errors
- Data rate unaffected

**Files to modify**:
- `micro/components/ble_nus/src/ble_nus.c` (connection parameter negotiation)

---

### Task 11.3: Peripheral power gating

**Description**: Disable unused peripherals to reduce baseline power consumption.

**Acceptance Criteria**:
- [ ] WiFi radio disabled when not in use (`wifi_enabled == false` in config)
- [ ] `esp_err_t power_manager_get_battery_mv(uint16_t *battery_mv)` — reads battery via ADC (or returns 999 if no ADC in MVP)
- [ ] Unused GPIO pins set to input with pull-down to prevent floating
- [ ] I2C bus only active during sensor reads (optional: release between reads)

**Validation**:
- Verify baseline current with unused peripherals disabled

**Files to modify**:
- `micro/components/power_manager/src/power_manager.c`

---

### Task 11.4: Power consumption measurement & logging

**Description**: Measure and document power consumption in various states.

**Acceptance Criteria**:
- [ ] Measure current in: BLE advertising, BLE connected idle, BLE connected + sensor reads, light-sleep
- [ ] Document results in a power budget table
- [ ] Estimate battery life for target battery capacity (e.g., 500 mAh → 8+ hours target)
- [ ] Identify any unexpected power drains

**Validation**:
- Power budget documented
- Battery life estimate meets 8+ hour target

**Files to create**:
- `docs/power-budget.md` (measurement results and analysis)

---

## Phase 12: Integration & Validation

**Objective**: Full system validation with real-world testing, long-duration stability, and edge case coverage.  
**Estimated Duration**: 2–3 days  
**Dependencies**: All previous phases complete

**Refactorización (obligatoria)**:
- Aplicar Boy Scout Rule al cerrar cada corrección detectada durante validación.
- Corregir deuda técnica localizada sin ampliar el alcance funcional.
- Mantener coherencia de contratos entre módulos después de cada ajuste.
- Revalidar end-to-end tras cada refactorización aplicada.

---

### Task 12.1: End-to-end test with XCTrack

**Description**: Validate the complete system with XCTrack (the target variometer app for paragliding).

**Acceptance Criteria**:
- [ ] XCTrack detects "FlyInPeace" as a vario sensor source
- [ ] XCTrack receives and parses LK8EX1 sentences correctly
- [ ] Altitude display in XCTrack matches expected value (±10 m at known altitude)
- [ ] Vario display responds to pressure changes (blow on sensor → positive reading)
- [ ] No data gaps or parsing errors in XCTrack logs

**Validation**:
- Run XCTrack with FlyInPeace connected for 15+ minutes
- Screenshot/document XCTrack readings

---

### Task 12.2: Long-duration stability test (8+ hours)

**Description**: Run the system for extended duration to verify stability.

**Acceptance Criteria**:
- [ ] System runs for 8+ hours without crashes, reboots, or watchdog resets
- [ ] No memory leaks (free heap stable over time)
- [ ] BLE connection remains stable (or re-establishes after disconnect)
- [ ] Sensor readings remain consistent (no drift or failures)
- [ ] Stack high-water marks remain safe (>25% remaining)

**Validation**:
- Serial monitor log captured for full duration
- Heap and stack metrics at start, 1h, 4h, 8h

---

### Task 12.3: Power consumption budget verification

**Description**: Verify actual power consumption meets the design budget from Phase 11.

**Acceptance Criteria**:
- [ ] Actual current draw matches documented power budget (±20%)
- [ ] Battery life meets target (8+ hours on target battery)
- [ ] Light-sleep activation confirmed via power measurement

**Validation**:
- Run on battery for full duration test
- Document actual vs expected power consumption

---

### Task 12.4: Edge case testing (BLE disconnect/reconnect, sensor errors)

**Description**: Test error handling and recovery scenarios.

**Acceptance Criteria**:
- [ ] BLE disconnect → advertising restarts → reconnection works
- [ ] Rapid connect/disconnect cycles (10x) → no crashes
- [ ] Sensor I2C error (e.g., disconnect sensor briefly) → `sensor_valid = false` → LED shows `ERROR` → sensor reconnect → recovery
- [ ] NVS full/corrupt → defaults applied, system runs
- [ ] OOM scenario: verify graceful handling
- [ ] Multiple phones scanning simultaneously → no crash

**Validation**:
- Document each test case result (pass/fail)

---

## Phase 13: Documentation & Cleanup

**Objective**: Final documentation, code cleanup, and preparation for ongoing development.  
**Estimated Duration**: 1–2 days  
**Dependencies**: Phase 12 complete

---

### Task 13.1: Firmware README with build/flash instructions

**Description**: Write a comprehensive README for the firmware directory.

**Acceptance Criteria**:
- [ ] `micro/README.md` includes:
  - Project overview and architecture summary
  - Prerequisites (ESP-IDF version, Python, tools)
  - Build instructions (`idf.py build`)
  - Flash instructions (`./scripts/micro/flash.sh`)
  - Monitor instructions (`./scripts/micro/monitor.sh`)
  - Sensor selection via `idf.py menuconfig`
  - Test instructions (`./scripts/micro/test.sh`)
  - Troubleshooting (common errors)

**Validation**:
- A new developer can follow README to build, flash, and run

**Files to create**:
- `micro/README.md`

---

### Task 13.2: Component API documentation

**Description**: Ensure all public headers have complete Doxygen documentation.

**Acceptance Criteria**:
- [ ] All public functions in all component headers have Doxygen `@brief`, `@param`, `@return`
- [ ] All public structs have field documentation
- [ ] All enums have value documentation
- [ ] No undocumented public symbols

**Validation**:
- Review all `include/*.h` files

---

### Task 13.3: Architecture diagram update (Mermaid)

**Description**: Update the architecture document with final Mermaid diagrams reflecting the actual implementation.

**Acceptance Criteria**:
- [ ] Component dependency diagram matches actual `REQUIRES` in CMakeLists.txt files
- [ ] Data flow diagram matches actual task implementation
- [ ] State machine diagrams match actual code behavior
- [ ] Any deviations from original architecture documented with rationale

**Validation**:
- Diagrams render correctly in GitHub Markdown

**Files to modify**:
- `docs/architecture/firmware-architecture.md`

---

### Task 13.4: Code review pass (Boy Scout Rule)

**Description**: Final code review and cleanup following the Boy Scout Rule ("leave the code cleaner than you found it").

**Acceptance Criteria**:
- [ ] All `TODO` and `FIXME` comments resolved or tracked as issues
- [ ] Code formatted with `.clang-format` (`./scripts/micro/format.sh`)
- [ ] No compiler warnings with `-Wall -Wextra -Werror`
- [ ] No unused includes, variables, or functions
- [ ] Consistent naming conventions across all components
- [ ] All magic numbers replaced with named constants
- [ ] All temporary test code removed from `main.c`

**Validation**:
- Clean build with no warnings
- `./scripts/micro/test.sh` — all tests pass
- Code review checklist completed
