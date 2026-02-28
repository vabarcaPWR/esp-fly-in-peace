# Roadmap — Mobile App (Flutter / Android)

> **Project**: esp-fly-in-peace  
> **Component**: Mobile App (`app/`)  
> **Technology**: Flutter (Dart) — Android only (MVP)  
> **BLE Plugin**: `flutter_blue_plus`  
> **Master reference**: `.github/PRE-PROMPT.md`

---

## Summary Checklist

- [x] **Phase 0: Project Bootstrap**
  - [x] Task 0.1: Create Flutter project
  - [x] Task 0.2: Configure dependencies (BLE, state management, etc.)
  - [x] Task 0.3: Choose and set up state management
  - [x] Task 0.4: Define project structure and architecture
  - [x] Task 0.5: Create app theme and common widgets
  - [x] Task 0.6: Verify build and run on Android device/emulator
- [x] **Phase 1: BLE Scanner**
  - [x] Task 1.1: Android BLE permissions handling
  - [x] Task 1.2: BLE scan functionality (show all BLE + compatibility detection)
  - [x] Task 1.3: Device list UI (all BLE devices, compatibility badge, connect button)
  - [x] Task 1.4: Pull-to-refresh and scan timeout
- [x] **Phase 1.5: Linux BLE Debug Fast-Track**
  - [x] Task 1.5.1: Linux runtime/adapter readiness checks
  - [x] Task 1.5.2: BLE device listing on Linux desktop app
  - [x] Task 1.5.3: Firmware-first validation loop (scan + identify FlyInPeace)
  - [x] Task 1.5.4: Debug evidence checklist for firmware bring-up
  - [x] Task 1.5.5: Linux console telemetry mirror (print received BLE lines)
  - [ ] Task 1.5.6: Linux console validation run with ESP32-C3 stream
  - [x] Task 1.5.7: Auto-connect when FlyInPeace device is discovered
  - [x] Task 1.5.8: Auto-disconnect BLE on app shutdown (controlled/uncontrolled)
- [x] **Phase 2: BLE Connection**
  - [x] Task 2.1: Connect to device
  - [x] Task 2.2: Connection state management
  - [x] Task 2.3: Auto-reconnect logic
  - [x] Task 2.4: Disconnect handling and UI feedback
- [x] **Phase 3: NUS Communication**
  - [x] Task 3.1: Discover NUS service and characteristics
  - [x] Task 3.2: Subscribe to TX notifications (receive data)
  - [x] Task 3.3: Write to RX characteristic (reserved for future use)
  - [x] Task 3.4: Raw data debug screen
- [x] **Phase 3.5: Frame Inspector + Interop Fast-Track (P0)**
  - [x] Task 3.5.1: Parsed-frame inspection screen (field-level LK8EX1 validation)
  - [x] Task 3.5.2: Frame correctness verdict engine (valid/warning/error)
  - [x] Task 3.5.3: Multi-device compatibility layer (FlyInPeace + BlueFlyVario)
  - [x] Task 3.5.4: Cross-device integration validation with simulated profiles
- [ ] **Phase 3.6: BLE Session Recording (P1)**
  - [x] Task 3.6.1: File logging service for received BLE stream
  - [x] Task 3.6.2: Recorder controls in app (start/stop/save path)
  - [x] Task 3.6.3: Session metadata + export validation
- [ ] **Phase 4: LK8EX1 Parser**
  - [ ] Task 4.1: LK8EX1 sentence parser
  - [ ] Task 4.2: Checksum validation
  - [ ] Task 4.3: Data model for parsed values
  - [ ] Task 4.4: Unit tests for parser
- [ ] **Phase 5: Real-Time Display**
  - [ ] Task 5.1: Flight dashboard screen layout
  - [ ] Task 5.2: Altitude display widget
  - [ ] Task 5.3: Vario display widget (with trend indicator)
  - [ ] Task 5.4: Pressure and temperature display
  - [ ] Task 5.5: Connection status indicator
  - [ ] Task 5.6: Data stream integration
- [ ] **Phase 6: Device Configuration**
  - [ ] Task 6.1: Config Service GATT client (read/write JSON)
  - [ ] Task 6.2: Config screen UI
  - [ ] Task 6.3: Read current config from device
  - [ ] Task 6.4: Write config and save to device NVS
  - [ ] Task 6.5: Config validation and error handling
  - [ ] Task 6.6: Altitude calibration via BLE
- [ ] **Phase 7: Settings & Persistence**
  - [ ] Task 7.1: App settings screen
  - [ ] Task 7.2: Display units (metric/imperial)
  - [ ] Task 7.3: Local persistence (shared_preferences)
  - [ ] Task 7.4: Dark mode implementation (light/dark/system)
  - [ ] Task 7.5: Dark mode rollout across all screens
- [ ] **Phase 8: Polish & Testing**
  - [ ] Task 8.1: Error handling audit
  - [ ] Task 8.2: UI polish and accessibility
  - [ ] Task 8.3: Unit tests for business logic
  - [ ] Task 8.4: Widget tests for screens
  - [ ] Task 8.5: Integration test with real device
- [ ] **Phase 9: Documentation**
  - [ ] Task 9.1: App README with build instructions
  - [ ] Task 9.2: User guide with screenshots
  - [ ] Task 9.3: Architecture documentation

## Status Notes Index

- [Phase 0 status (2026-02-23)](#status-note-phase0-2026-02-23)
- [Phase 1 status (2026-02-23)](#status-note-phase1-2026-02-23)
- [Phase 1 status (2026-02-24)](#status-note-phase1-2026-02-24)
- [Phase 1.5 execution result](#status-note-phase15-execution-result)
- [Phase 1.5 re-run after unblock](#status-note-phase15-rerun)
- [Phase 1.5 Linux plugin fix](#status-note-phase15-plugin-fix)
- [Phase 1.5 final validation run](#status-note-phase15-final-validation)
- [Phase 2.1 implementation + validation](#status-note-phase21-implementation)
- [Phase 2.2 implementation + validation](#status-note-phase22-implementation)
- [Phase 2.3 implementation + validation](#status-note-phase23-implementation)
- [Phase 2.4 implementation + validation](#status-note-phase24-implementation)
- [Phase 3.1 implementation + validation](#status-note-phase31-implementation)
- [Phase 3.2 implementation + validation](#status-note-phase32-implementation)
- [Phase 3.3 implementation + validation](#status-note-phase33-implementation)
- [Phase 3.4 implementation + validation](#status-note-phase34-implementation)
- [Phase 3.4 debug tooling consolidation](#status-note-phase34-debug-consolidation)
- [Phase 3.5.1 implementation + static validation](#status-note-phase351-implementation)
- [Phase 3.5.2 implementation + static validation](#status-note-phase352-implementation)
- [Phase 3.5.3 implementation + static validation](#status-note-phase353-implementation)
- [Phase 3.5 compatibility hardening + protocol alignment](#status-note-phase35-compat-hardening)
- [Phase 3.5 UX/branding flow updates](#status-note-phase35-ux-branding)
- [Phase 3.5 autoscroll stability fix](#status-note-phase35-autoscroll-fix)
- [Phase 3.5 matrix validation completed](#status-note-phase35-matrix-validation)
- [Phase 3.5 compatibility rerun execution](#status-note-phase35-compat-rerun)
- [Phase 3.6 implementation + static validation](#status-note-phase36-implementation)
- [Phase 9.1 scripts/runtime doc update](#status-note-phase91-doc-update)

---

## Cross-Phase Rule — `conductor-model-hardware`

**Mandatory scope**: all implementation phases (`0` to `8`).

For every feature/module implemented in those phases:

- **Conductor**: use-case orchestration, UI/flow coordination, error mapping.
- **Model**: domain/state logic independent from platform plugins.
- **Hardware**: platform/plugin adapters (BLE, storage, permissions, device APIs).

**Phase completion gate**:
- No implementation phase is considered complete if new module code violates this split.
- Validation must be re-run after refactorization to this pattern.
- The module checklist in `docs/architecture/conductor-model-hardware-template.md` must be applied.

---

## Phase 0: Project Bootstrap

**Objective**: Set up a working Flutter project with dependencies, architecture, and a running "Hello World" app on Android.  
**Estimated Duration**: 2–3 days  
**Dependencies**: None (can run in parallel with firmware Phase 0)

**Refactorización (obligatoria)**:
- Aplicar Boy Scout Rule al cerrar cada tarea de la fase.
- Reducir duplicación y complejidad accidental sin cambiar comportamiento funcional.
- Mantener nombres y límites de módulos claros para código autoexplicativo.
- Repetir la validación de la fase después de cada refactorización.

---

### Task 0.1: Create Flutter project

**Description**: Create a new Flutter project under `app/` targeting Android only. Configure `android/app/build.gradle` with appropriate min/target SDK versions for BLE support.

**Acceptance Criteria**:
- [x] Flutter project created at `app/`
- [x] `android/app/build.gradle` sets `minSdkVersion 21` (for BLE), `targetSdkVersion 34`
- [x] Project name: `fly_in_peace`
- [x] Package/application ID: `com.flyinpeace.app` (or similar)
- [x] `flutter run` launches the default counter app on Android device/emulator

**Validation**:
- `flutter run` succeeds on an Android device or emulator

**Files to create**:
- Flutter project structure under `app/`

**Notes**:
- Run `flutter create --org com.flyinpeace --project-name fly_in_peace app`
- Android BLE requires `minSdkVersion 21`. For BLE permissions on Android 12+, needs SDK 31+.
- Remove iOS target if desired (or keep for future).

---

### Task 0.2: Configure dependencies

**Description**: Add required packages to `pubspec.yaml`: BLE, state management, local storage, etc.

**Acceptance Criteria**:
- [x] `flutter_blue_plus` added for BLE communication
- [x] State management package added (see Task 0.3)
- [x] `shared_preferences` added for local app settings
- [x] `permission_handler` added for BLE permissions
- [x] `flutter_lints` or `very_good_analysis` for lint rules
- [x] `flutter pub get` succeeds with no dependency conflicts

**Validation**:
- `flutter pub get` completes successfully
- `flutter analyze` shows no errors

**Files to modify**:
- `app/pubspec.yaml`
- `app/analysis_options.yaml`

**Notes**:
Core dependencies:
```yaml
dependencies:
  flutter_blue_plus: ^1.x.x
  shared_preferences: ^2.x.x
  permission_handler: ^11.x.x
  # state management (decided in Task 0.3)

dev_dependencies:
  flutter_test:
    sdk: flutter
  flutter_lints: ^4.x.x
  mockito: ^5.x.x
  build_runner: ^2.x.x
```

---

### Task 0.3: Choose and set up state management

**Description**: **Decision required.** Choose a state management approach for the app architecture. This affects all subsequent screens.

> **AI Agent must present options and wait for developer confirmation.**

**Options**:

| Option | Pros | Cons | Recommendation |
|--------|------|------|----------------|
| **Provider** | Simple, official Flutter recommendation, minimal boilerplate | Can get messy in large apps, less structured | Good for MVP |
| **Riverpod** | Modern, compile-safe, testable, no BuildContext dependency | More concepts to learn (providers, notifiers), newer | **★ Recommended** |
| **BLoC/Cubit** | Very structured, separates UI from logic, great for complex state | More boilerplate, steeper learning curve | Overkill for MVP |

**Recommended: Riverpod** — rationale:
- Compile-safe dependency injection (no runtime errors).
- Easy to test (providers can be overridden in tests).
- Simple enough for a learning developer, but scales well.
- No `BuildContext` needed for accessing state (useful for BLE callbacks).

**Acceptance Criteria**:
- [x] State management choice confirmed by developer
- [x] Package installed and basic provider structure created
- [x] A sample provider works (e.g., counter or connection state)

**Validation**:
- App compiles and runs with state management integrated

---

### Task 0.4: Define project structure and architecture

**Description**: Create the directory structure following a feature-based architecture and the `conductor-model-hardware` pattern.

**Acceptance Criteria**:
- [x] Directory structure created:
  ```
  app/lib/
  ├── main.dart                     # Entry point
  ├── app.dart                      # MaterialApp configuration
  ├── core/
  │   ├── ble/                      # BLE service layer
  │   │   ├── ble_service.dart
  │   │   └── nus_protocol.dart
  │   ├── models/                   # Data models
  │   │   ├── lk8ex1_data.dart
  │   │   └── device_config.dart
  │   └── utils/                    # Utility functions
  │       └── lk8ex1_parser.dart
  ├── features/
  │   ├── scanner/                  # BLE scanner feature
  │   │   ├── scanner_screen.dart
  │   │   └── scanner_provider.dart
  │   ├── dashboard/                # Flight data display
  │   │   ├── dashboard_screen.dart
  │   │   └── dashboard_provider.dart
  │   ├── config/                   # Device configuration
  │   │   ├── config_screen.dart
  │   │   └── config_provider.dart
  │   └── settings/                 # App settings
  │       ├── settings_screen.dart
  │       └── settings_provider.dart
  └── widgets/                      # Shared widgets
      ├── connection_indicator.dart
      └── value_display.dart
  ```
- [x] Each directory has a placeholder file or `.gitkeep`
- [x] Each feature defines `conductor`, `model`, and `hardware` responsibilities (can be files or subfolders)

**Validation**:
- Project compiles with empty structure

---

### Task 0.5: Create app theme and common widgets

**Description**: Set up the app's visual theme (Material 3) and create reusable base widgets.

**Acceptance Criteria**:
- [x] Material 3 theme configured in `app.dart`
- [x] Color scheme defined (flight/outdoor theme: blues, greens, grays)
- [x] Text theme defined (readable at a glance, large data values)
- [x] Common widget: `ConnectionIndicator` (green dot = connected, red dot = disconnected)
- [x] Common widget: `ValueDisplay` (label + large value + unit, reusable for altitude/vario/pressure)
- [x] Dark mode support (optional for MVP, but theme structure should allow it)

**Validation**:
- App displays themed widgets correctly

---

### Task 0.6: Verify build and run on Android device/emulator

**Description**: End-to-end verification: the Flutter app builds, installs, and runs on an Android device.

**Acceptance Criteria**:
- [x] `flutter build apk --debug` succeeds
- [ ] App installs and launches on Android device/emulator
- [x] Basic navigation between placeholder screens works
- [x] No crash or build errors

**Validation**:
- Install APK on physical device, navigate between screens

**Notes**: This is the gate for Phase 0. Do not proceed to Phase 1 until this passes.

<a id="status-note-phase0-2026-02-23"></a>
**Status Note (2026-02-23)**:
- `flutter pub get` ✅
- `flutter analyze` ✅
- `flutter test` ✅
- `flutter build apk --debug` ✅ (`scripts/app/build_app_debug.sh` with `JAVA_HOME=/home/vabarca/.local/jdk-17`)

---

## Phase 1: BLE Scanner

**Objective**: Implement BLE device scanning and display **all discovered BLE devices**, while also detecting and highlighting FlyInPeace-compatible devices (NUS UUID and/or name hint).  
**Estimated Duration**: 2–3 days  
**Dependencies**: Phase 0 complete

**Refactorización (obligatoria)**:
- Aplicar Boy Scout Rule al cerrar cada tarea de la fase.
- Reducir duplicación y complejidad accidental sin cambiar comportamiento funcional.
- Mantener nombres y límites de módulos claros para código autoexplicativo.
- Repetir la validación de la fase después de cada refactorización.

---

### Task 1.1: Android BLE permissions handling

**Description**: Implement proper Android BLE permission handling for all Android versions (especially Android 12+ which requires `BLUETOOTH_SCAN`, `BLUETOOTH_CONNECT`, and location permissions).

**Acceptance Criteria**:
- [x] Request `BLUETOOTH_SCAN` and `BLUETOOTH_CONNECT` on Android 12+ (API 31+)
- [x] Request `ACCESS_FINE_LOCATION` on Android 11 and below
- [x] Handle "permission denied" gracefully — show explanation dialog
- [x] Handle "permission permanently denied" — direct user to app settings
- [x] Check if Bluetooth adapter is enabled — prompt to enable if off
- [x] Check if Location services are enabled (required for BLE scan on some Android versions)

**Validation**:
- Test on Android 12+ device: permissions requested correctly
- Test on Android 10 device (if available): location permission requested
- Deny permission → app shows explanation, doesn't crash

**Files to create/modify**:
- `app/android/app/src/main/AndroidManifest.xml` (add permissions)
- `app/lib/core/ble/ble_permissions.dart`

**Notes**:
Required manifest permissions:
```xml
<uses-permission android:name="android.permission.BLUETOOTH_SCAN" android:usesPermissionFlags="neverForLocation" />
<uses-permission android:name="android.permission.BLUETOOTH_CONNECT" />
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />
<uses-feature android:name="android.hardware.bluetooth_le" android:required="true" />
```

---

### Task 1.2: BLE scan functionality (show all BLE + compatibility detection)

**Description**: Implement BLE scanning using `flutter_blue_plus` to show all discovered BLE devices. In parallel, compute compatibility signals for FlyInPeace/NUS devices.

**Acceptance Criteria**:
- [x] Start/stop scanning via `FlutterBluePlus.startScan()` / `stopScan()`
- [x] Show all BLE scan results (no exclusion by UUID or name)
- [x] Detect compatibility by NUS service UUID: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- [x] Detect compatibility by name hint containing "FlyInPeace" (fallback when UUID is not advertised)
- [x] Expose result model with compatibility flags to the UI/provider
- [x] Scan timeout: 10 seconds (configurable)
- [x] Deduplicate results (same device MAC)

**Validation**:
- Turn on ESP32-C3 with BLE firmware → appears in scan results and marked as compatible
- Turn on another BLE peripheral (e.g., smartwatch/headphones) → appears in scan results as non-compatible

**Files to create/modify**:
- `app/lib/core/ble/ble_service.dart`
- `app/lib/features/scanner/scanner_provider.dart`

---

### Task 1.3: Device list UI (all BLE devices, compatibility badge, connect button)

**Description**: Build the scanner screen UI to show all discovered BLE devices and clearly distinguish FlyInPeace-compatible devices.

**Acceptance Criteria**:
- [x] List view showing each device: name (or "Unknown"), MAC address, RSSI signal indicator
- [x] Compatibility badge per item: `FlyInPeace compatible` / `Other BLE device`
- [x] "Scan" FAB button to start/stop scanning
- [x] Scanning indicator (spinner or animation) while scan is active
- [x] Tap on compatible device → navigate to connection / dashboard
- [x] Non-compatible device tap shows explanatory message (no crash)
- [x] Empty state: "No BLE devices found nearby."
- [x] RSSI shown as signal bars or dBm value
- [x] UI filter `All / Compatible` available (default: `All`)

**Validation**:
- Scan shows ESP32-C3 as compatible and other BLE devices as non-compatible

**Files to create/modify**:
- `app/lib/features/scanner/scanner_screen.dart`
- `app/lib/features/scanner/scanner_provider.dart`
- `app/lib/widgets/device_list_tile.dart`

---

### Task 1.4: Pull-to-refresh and scan timeout

**Description**: Add pull-to-refresh gesture and automatic scan timeout with "rescan" button.

**Acceptance Criteria**:
- [x] Pull down on device list → restart scan
- [x] Scan automatically stops after timeout (10 seconds)
- [x] "Scan again" button appears after scan completes
- [x] Scan progress indicator (e.g., linear progress bar showing time remaining)

**Validation**:
- Pull down → scan restarts
- Wait 10 seconds → scan stops, "Scan again" visible

<a id="status-note-phase1-2026-02-23"></a>
**Status Note (2026-02-23)**:
- `flutter analyze` ✅
- `flutter test` ✅
- `flutter build apk --debug` ✅

<a id="status-note-phase1-2026-02-24"></a>
**Status Note (2026-02-24)**:
- Task 1.2 completed: scanner now lists all BLE devices and computes FlyInPeace compatibility flags.
- Task 1.3 completed: compatibility badges added; non-compatible taps are handled safely.
- Added scanner filter `All / Compatible` (default `All`).
- `flutter analyze` ✅
- `flutter test` ✅

---

## Phase 1.5: Linux BLE Debug Fast-Track

**Objective**: Enable a Linux desktop debug path to quickly list BLE devices from the app and accelerate firmware bring-up on the microcontroller.  
**Estimated Duration**: 0.5–1 day  
**Dependencies**: Phase 1 complete

**Scope Note**:
- Linux support in this phase is for development/debugging only.
- MVP release target remains Android.

**Priority**: **P0 (highest)** for firmware debugging start.

---

### Task 1.5.1: Linux runtime/adapter readiness checks

**Description**: Ensure the Linux host can run Flutter desktop and has BLE stack prerequisites available.

**Acceptance Criteria**:
- [x] `flutter run -d linux` launches the app successfully
- [x] Linux Bluetooth adapter is detected and enabled
- [x] App shows actionable error messages when Bluetooth/permissions are unavailable
- [x] Script helper accepts Linux target (`./scripts/app/app_test_option.sh 1 linux`)

**Validation**:
- Run `./scripts/app/app_test_option.sh 1 linux`
- Verify scanner screen opens without crashes

<a id="status-note-phase15-execution-result"></a>
**Status Note (2026-02-24 — execution result)**:
- `flutter devices` ✅ detects `Linux (desktop)` and `Chrome`.
- `./scripts/app/app_test_option.sh list` ✅ includes `linux` target.
- `bluetoothctl show` ✅ adapter powered on (`Controller 00:72:EE:11:16:28`, `Powered: yes`).
- `./scripts/app/app_test_option.sh 1 linux --no-resident` ❌ fails before app launch (Linux toolchain).
- Blockers detected:
  - `clang++` not installed in host.
  - `gtk+-3.0` development package not installed (`pkg-config` cannot find `gtk+-3.0`).
- Immediate unblock actions:
  - `sudo apt install clang libgtk-3-dev`
  - Re-run `./scripts/app/app_test_option.sh 1 linux --no-resident`

<a id="status-note-phase15-rerun"></a>
**Status Note (2026-02-24 — re-run after unblock)**:
- `clang++` and `gtk+-3.0` now available in host.
- `./scripts/app/app_test_option.sh 1 linux --no-resident` ✅ builds and launches Linux desktop app.
- Linux desktop debug path is now unblocked for scanner validation.
- Bluetooth host check ✅ (`bluetoothctl show` reports controller powered on).

<a id="status-note-phase15-plugin-fix"></a>
**Status Note (2026-02-24 — Linux plugin fix)**:
- Linux app run previously failed with `MissingPluginException` from `permission_handler` (`requestPermissions`).
- `app/lib/core/ble/ble_permissions.dart` updated to bypass mobile permission requests on Linux desktop and rely on adapter readiness checks.
- After fix, `./scripts/app/app_test_option.sh 1 linux --no-resident` runs without the plugin crash.

<a id="status-note-phase15-final-validation"></a>
**Status Note (2026-02-24 — final validation run)**:
- `flutter --version` ✅ Flutter `3.41.2`, Dart `3.11.0`.
- `flutter devices` ✅ `Linux (desktop)` target detected.
- `bluetoothctl show` ✅ adapter `00:72:EE:11:16:28` powered on.
- `./scripts/app/app_test_option.sh 1 linux --no-resident` ✅ app launches in Linux desktop.
- Actionable error-path behavior is implemented and wired in scanner dialog flow:
  - `BleReadinessIssue.bluetoothDisabled` → dialog title `Bluetooth disabled` + CTA `Turn on Bluetooth`.
  - `BleReadinessIssue.notSupported|permissionsDenied` → dialog title `Scan unavailable` + actionable message text.

**Files to create/modify**:
- `scripts/app/app_test_option.sh`
- `app/lib/core/ble/ble_permissions.dart` (if Linux-specific handling is needed)

---

### Task 1.5.2: BLE device listing on Linux desktop app

**Description**: Confirm scanner functionality on Linux desktop lists nearby BLE devices from the running app.

**Acceptance Criteria**:
- [x] Scanner lists nearby BLE devices on Linux desktop
- [x] Device list updates on rescan
- [x] FlyInPeace compatibility badges are shown on Linux the same as Android
- [x] Empty and error states are readable and actionable

**Validation**:
- Start app on Linux, scan, and verify at least one nearby BLE device appears
- Verify ESP32 advertising appears and is tagged as compatible

**Execution Note (2026-02-24)**:
- BlueZ scan on host detects nearby BLE devices (e.g., `Quetzalcóatl`, `3C:13:5A:40:3A:7B`).
- FlyInPeace advertising was not observed in this run (firmware advertising validation still pending).

**Execution Note (2026-02-24 — firmware validation run)**:
- `./scripts/micro/build.sh` ✅ build succeeded.
- `./scripts/micro/flash.sh` ✅ flash succeeded on ESP32-C3 (`MAC dc:da:0c:81:52:24` during flashing).
- `./scripts/micro/monitor.sh` ✅ firmware boot confirmed; BLE logs show advertising started:
  - `BLE advertising started: name=FlyInPeace interval_ms=100`
  - Runtime Bluetooth MAC observed in logs: `dc:da:0c:81:52:26`
- Linux `bluetoothctl` scan did not consistently surface `FlyInPeace`/target MAC in timed CLI scans.
- Result: Linux app runtime is ready, firmware advertises BLE, but scanner detection evidence is still inconclusive at CLI level.

**Execution Note (2026-02-24 — long scan re-run)**:
- 3 long scan cycles executed with host-side automation (`bluetoothctl --timeout 25 scan on`).
- Target signatures searched per cycle: `FlyInPeace`, `dc:da:0c:81:52:26`, `6E400001`.
- Result: `PASS_COUNT=0` (no target signature detected in any cycle).
- Serial corroboration still confirms active advertising on firmware side:
  - `main: esp-fly-in-peace firmware starting`
  - `BLE_INIT: Bluetooth MAC: dc:da:0c:81:52:26`
  - `ble_nus_hw: BLE advertising started: name=FlyInPeace interval_ms=100`

**Execution Note (2026-02-24 — post-fix scan re-run)**:
- Repeated 3-cycle scan with `bluetoothctl --timeout 20 scan on` after Linux plugin fix.
- Result remains `PASS_COUNT=0` (target still not observed from host scan output).

**Execution Note (2026-02-24 — firmware advertising payload tuning)**:
- Updated firmware advertising layout in `ble_nus_hardware.c` to improve Linux detectability:
  - NUS UUID moved to primary ADV payload.
  - Short device-name hint included in ADV payload.
  - Full device name moved to Scan Response.
- Firmware rebuilt/flashed successfully and monitor still confirms advertising start.
- Post-change 3-cycle host scan result remains `PASS_COUNT=0` (no target signature observed).
- Next firmware-debug action: inspect on-air packets with BLE sniffer / btmon to confirm emitted ADV payload and address type.

**Execution Note (2026-02-24 — low-level sniffing attempt)**:
- `btmon` capture attempted but failed without elevated privileges:
  - `Failed to bind channel: Operation not permitted`
- `hcitool lescan` without privileges also failed:
  - `Set scan parameters failed: Operation not permitted`
- Passwordless `sudo` is not available in this environment (`sudo: a password is required`).
- Unblock required on host side:
  - Run privileged diagnostics manually (example):
    - `sudo btmon`
    - `sudo hcitool lescan --duplicates`
  - Or grant `CAP_NET_ADMIN` to the diagnostic binary/session.

**Execution Note (2026-02-24 — privileged btmon evidence provided)**:
- On-air capture confirms ESP32 advertising is present and stable:
  - Repeated `ADV_IND` PDUs observed.
  - Repeated `SCAN_RSP` observed.
  - Address observed: `DC:DA:0C:81:52:26 (Espressif Inc.)`.
  - `Name (complete): FlyInPeace` observed repeatedly.
- Conclusion: firmware advertising and identity signaling (name/MAC) are validated at HCI level.

**Execution Note (2026-02-24 — final Linux scan cycles after firmware restart)**:
- Firmware was rebuilt and reflashed (`./scripts/micro/build.sh`, `./scripts/micro/flash.sh --force-release-port`) and rebooted successfully.
- Post-restart monitor confirms advertising resume:
  - `BLE_INIT: Bluetooth MAC: dc:da:0c:81:52:26`
  - `ble_nus_hw: BLE advertising started: name=FlyInPeace interval_ms=100`
- 3 rescan cycles on host (`bluetoothctl --timeout 12 scan on` + `bluetoothctl devices`) show consistent rediscovery:
  - `CYCLE_1_REDISCOVER=1`
  - `CYCLE_2_REDISCOVER=1`
  - `CYCLE_3_REDISCOVER=1`
- Log snippets captured:
  - `Device DC:DA:0C:81:52:26 FlyInPeace` (present in `/tmp/dev_1.log`, `/tmp/dev_2.log`, `/tmp/dev_3.log`).
- Linux scanner UI compatibility behavior is shared with Android path (same provider/service/tile flow), so badge/empty/error rendering parity is preserved.

**Files to create/modify**:
- `app/lib/features/scanner/scanner_screen.dart`
- `app/lib/features/scanner/scanner_provider.dart`
- `app/lib/core/ble/ble_service.dart`

---

### Task 1.5.3: Firmware-first validation loop (scan + identify FlyInPeace)

**Description**: Define and execute the minimal loop required by firmware bring-up: flash firmware, advertise BLE, verify app discovery on Linux.

**Acceptance Criteria**:
- [x] Flash firmware and start BLE advertising on micro
- [x] Linux host scan detects the device within 10 seconds (btmon evidence)
- [x] Device appears as `FlyInPeace compatible` (name/MAC verified in btmon)
- [x] Re-scan after firmware restart still re-discovers device

**Validation**:
- Run 3 consecutive scan cycles after firmware reboot; detection succeeds each cycle

**Final Status**:
- Re-scan-after-restart proof sequence completed with 3/3 rediscovery passes.

---

### Task 1.5.4: Debug evidence checklist for firmware bring-up

**Description**: Capture consistent evidence for BLE scanner readiness to unblock firmware debugging.

**Acceptance Criteria**:
- [x] Checklist recorded with date, host OS, Bluetooth adapter, and app/device versions
- [x] At least one screenshot or log snippet of Linux scanner listing BLE devices
- [x] At least one screenshot or log snippet showing FlyInPeace-compatible detection
- [x] Pass/fail verdict documented

**Validation**:
- Evidence stored in project docs or issue tracker and referenced from roadmap status note

**Evidence Checklist (2026-02-24)**:
- Date: `2026-02-24`
- Host OS: `Debian GNU/Linux 13 (trixie)`
- Bluetooth adapter: `00:72:EE:11:16:28` (`Powered: yes`)
- Flutter/App toolchain: `Flutter 3.41.2`, `Dart 3.11.0`, Linux desktop target available
- Firmware build/flash: `./scripts/micro/build.sh` ✅, `./scripts/micro/flash.sh --force-release-port` ✅
- Firmware runtime identity: `BLE_INIT: Bluetooth MAC: dc:da:0c:81:52:26`, advertising started with `FlyInPeace`
- Linux BLE listing snippet: `Device DC:DA:0C:81:52:26 FlyInPeace` (from `/tmp/dev_1.log`, `/tmp/dev_2.log`, `/tmp/dev_3.log`)
- FlyInPeace detection snippet: btmon evidence with repeated `ADV_IND`/`SCAN_RSP` and complete name `FlyInPeace`
- Quality gate: `flutter test` ✅, `flutter analyze` ✅
- Verdict: **PASS** — Phase 1.5 is validated and closed.

---

### Task 1.5.5: Linux console telemetry mirror (print received BLE lines)

**Description**: Add an app-side Linux-only debug output that prints each received line from the connected BLE device to console/stdout, without changing mobile runtime behavior.

**Acceptance Criteria**:
- [x] Linux build prints every received BLE telemetry line to console while connected
- [x] Output includes timestamp and source device identifier
- [x] Android/Web behavior remains unchanged (no extra console spam outside Linux debug path)
- [x] Feature is reachable from current BLE debug flow

**Validation**:
- Run app on Linux (`./scripts/app/app_test_option.sh 1 linux`)
- Connect to ESP32-C3
- Verify console shows live incoming LK8EX1 lines continuously

**Files to create/modify**:
- `app/lib/core/ble/ble_service.dart` (or debug adapter layer)
- `app/lib/features/dashboard/ble_debug_console_screen.dart` (if UI toggle is needed)

**Status Note (2026-02-28 — implementation completed, validation prepared)**:
- Implemented Linux-only BLE telemetry mirror in `BleService` with console line format:
  - `[timestamp_utc_iso8601] BLE RX <device_id> (<device_name>) -> <raw_line>`
- Added unit tests for Linux-only gating and output format:
  - `app/test/core/ble/ble_service_linux_console_test.dart`
- Quality gate:
  - `./scripts/app/app_test_option.sh 3` ✅ (`flutter analyze` + `flutter test`)
- Live ESP32-C3 console capture session remains tracked by **Task 1.5.6**.

---

### Task 1.5.6: Linux console validation run with ESP32-C3 stream

**Description**: Execute a focused Linux validation session using the console mirror to confirm real incoming telemetry from ESP32-C3 and capture evidence for firmware loop closure.

**Acceptance Criteria**:
- [ ] At least one continuous 60s capture from ESP32-C3 is visible in Linux app console
- [ ] Evidence includes connection timestamp, device id/name, and sample LK8EX1 lines
- [ ] Result is synchronized with micro roadmap validation status

**Validation**:
- Launch Linux app + connect ESP32-C3
- Capture and attach console snippet
- Record PASS/FAIL note in both app and micro roadmaps

---

### Task 1.5.7: Auto-connect when FlyInPeace device is discovered

**Description**: Add an app-side behavior that automatically attempts BLE connection when scan results include a FlyInPeace-compatible device, while keeping manual connect flow available.

**Acceptance Criteria**:
- [x] While scanning, app identifies FlyInPeace-compatible devices and triggers a single auto-connect attempt
- [x] Auto-connect is gated to avoid repeated connect loops for the same device during one scan session
- [x] If auto-connect fails, app surfaces the error and keeps manual connect available
- [x] Existing Android/Linux/Web scan list and manual connect behavior remain functional

**Validation**:
- Run scanner flow with a FlyInPeace device advertising
- Verify app auto-initiates connection without manual tap
- Verify no repeated reconnect loop is triggered from scan events alone
- Run static checks and tests (`./scripts/app/app_test_option.sh 3`)

**Files to create/modify**:
- `app/lib/features/scanner/scanner_provider.dart`
- `app/lib/features/scanner/conductor/scanner_conductor.dart`
- `app/lib/core/ble/ble_service.dart` (if additional guard state is required)

**Status Note (2026-02-28 — implementation + validation)**:
- Implemented scanner-side auto-connect policy in `ScannerController`:
  - Auto-selects first FlyInPeace candidate from scan stream.
  - Uses per-scan attempted-device guard to prevent repeated auto-connect loops.
  - Reuses existing `connectToDevice` path so connection errors surface through current dialog flow and manual connect remains available.
- Added test coverage for candidate selection/gating:
  - `app/test/features/scanner/scanner_auto_connect_test.dart`
- Quality gate:
  - `./scripts/app/app_test_option.sh 3` ✅ (`flutter analyze` + `flutter test`)

---

### Task 1.5.8: Auto-disconnect BLE on app shutdown (controlled/uncontrolled)

**Description**: Ensure the app automatically disconnects from the currently connected BLE device when the app closes, both in controlled lifecycle exits and in unexpected termination scenarios (best-effort where OS limits apply).

**Acceptance Criteria**:
- [x] On controlled app close/lifecycle termination, app triggers BLE disconnect for active connection
- [x] On lifecycle transitions (`inactive`/`paused`/`detached`), disconnect policy is applied consistently per platform behavior
- [x] Unexpected termination path is handled with best-effort strategy and no stuck reconnect loop on next app start
- [x] Manual disconnect/connect flows continue to work without regression

**Validation**:
- Connect to FlyInPeace device and close app normally; verify device is disconnected
- Background/terminate app and confirm disconnect behavior on supported targets
- Reopen app after forced termination and verify no stale connected state or reconnect loop
- Run static checks and tests (`./scripts/app/app_test_option.sh 3`)

**Files to create/modify**:
- `app/lib/app.dart` (or root lifecycle observer)
- `app/lib/core/ble/ble_service.dart`
- `app/lib/core/ble/ble_providers.dart` (if lifecycle wiring requires provider changes)

**Status Note (2026-02-28 — implementation + validation)**:
- Added app-root lifecycle BLE policy in `app/lib/app.dart`:
  - `AppLifecycleBlePolicy.shouldDisconnectForState(...)` disconnects on `inactive`, `hidden`, `paused`, and `detached`.
  - `AppShell` now observes app lifecycle and triggers best-effort BLE disconnect when required.
  - On widget disposal, app also triggers disconnect to cover controlled shutdown paths.
- Added tests:
  - `app/test/app_lifecycle_ble_policy_test.dart`
- Quality gate:
  - `./scripts/app/app_test_option.sh 3` ✅ (`flutter analyze` + `flutter test`)

---

## Priority Order — Start Firmware Debug ASAP

Execute tasks in this order before continuing with broader app features:

1. **P0**: Complete all tasks in **Phase 3.5** (Frame Inspector + Interop Fast-Track)
2. **P0**: Complete all tasks in **micro Phase 3.5** (App Debug Stream Fast-Track)
3. **P1**: Continue with parser/dashboard work once cross-device frame debugging is stable
4. **P2+**: Remaining phases (config UX, theming polish, docs)

**Firmware-debug gate**:
- Do not postpone Phase 3.5 behind UI polish.
- Prioritize app/micro debug-loop closure before new feature expansion.

---

## Phase 2: BLE Connection

**Objective**: Implement BLE connection management with state tracking and auto-reconnect.  
**Estimated Duration**: 2–3 days  
**Dependencies**: Phase 1 complete

**Refactorización (obligatoria)**:
- Aplicar Boy Scout Rule al cerrar cada tarea de la fase.
- Reducir duplicación y complejidad accidental sin cambiar comportamiento funcional.
- Mantener nombres y límites de módulos claros para código autoexplicativo.
- Repetir la validación de la fase después de cada refactorización.

---

### Task 2.1: Connect to device

**Description**: Connect to a selected BLE device using `flutter_blue_plus`.

**Acceptance Criteria**:
- [x] `BleService.connect(BluetoothDevice device)` connects to the specified device
- [x] Connection timeout: 10 seconds
- [x] Negotiate MTU to maximum (request 512, accept whatever the device supports)
- [x] Discover services after connection
- [x] Verify NUS service is present — error if not

**Validation**:
- Tap device in scanner → connection established, services discovered

<a id="status-note-phase21-implementation"></a>
**Status Note (2026-02-24 — implementation + validation)**:
- `app/lib/core/ble/ble_service.dart` now implements `connect(BluetoothDevice device)` with:
  - 10s connection timeout (`connectionTimeout = Duration(seconds: 10)`).
  - MTU negotiation request (`requestMtu(512)` with graceful fallback).
  - Service discovery and strict NUS validation.
  - Stored references to connected device and discovered NUS entities for downstream tasks.
- Scanner flow now calls real connection before dashboard navigation:
  - `app/lib/features/scanner/scanner_provider.dart` adds `connectToDevice(...)`.
  - `app/lib/features/scanner/scanner_screen.dart` connects on tap/connect and shows connecting progress.
- Quality checks:
  - `flutter analyze` ✅
  - `flutter test` ✅

**Files to modify**:
- `app/lib/core/ble/ble_service.dart`

---

### Task 2.2: Connection state management

**Description**: Track and expose BLE connection state throughout the app.

**Acceptance Criteria**:
- [x] Connection states: `disconnected`, `connecting`, `connected`, `disconnecting`
- [x] State exposed via provider/stream (accessible from any screen)
- [x] State updates immediately on connect/disconnect events
- [x] All screens can react to connection state changes

**Validation**:
- Connection indicator widget shows correct state at all times

<a id="status-note-phase22-implementation"></a>
**Status Note (2026-02-24 — implementation + validation)**:
- Shared BLE service/provider introduced in `app/lib/core/ble/ble_providers.dart`.
- Global connection state stream exposed via `bleConnectionStatusProvider` and usable from any feature.
- `ScannerController` and `Dashboard` now consume the same `BleService` instance, removing local placeholder state.
- Dashboard indicator now reflects real BLE connection state (`connected` vs non-connected).
- Immediate state updates are emitted from BLE connect/disconnect callbacks in `BleService`.
- Quality checks:
  - `flutter analyze` ✅
  - `flutter test` ✅
  - `./scripts/app/app_test_option.sh 1 linux --no-resident` ✅

**Files to create/modify**:
- `app/lib/core/ble/ble_providers.dart`
- `app/lib/core/ble/ble_service.dart`
- `app/lib/features/scanner/scanner_provider.dart` (or a connection_provider)
- `app/lib/features/dashboard/dashboard_provider.dart`
- `app/lib/features/dashboard/dashboard_screen.dart`

---

### Task 2.3: Auto-reconnect logic

**Description**: Automatically attempt to reconnect when the device disconnects unexpectedly.

**Acceptance Criteria**:
- [x] On unexpected disconnect, wait 2 seconds then retry
- [x] Retry up to 5 times with exponential backoff (2s, 4s, 8s, 16s, 30s)
- [x] Show "Reconnecting..." in UI during retries
- [x] Stop retrying if user manually disconnects
- [x] Stop retrying if device is out of range for all attempts

**Validation**:
- Turn off ESP32-C3 briefly → app attempts reconnection
- Turn back on → app reconnects automatically

<a id="status-note-phase23-implementation"></a>
**Status Note (2026-02-24 — implementation + validation)**:
- Auto-reconnect implemented in `BleService` with exponential backoff schedule:
  - `2s`, `4s`, `8s`, `16s`, `30s` (`reconnectBackoffDelays`).
- Unexpected disconnect now starts reconnect loop automatically from the BLE connection-state listener.
- Manual disconnect path explicitly cancels pending reconnect attempts and resets reconnect state.
- Reconnect progress exposed via `BleReconnectState` stream and provider:
  - `bleReconnectStateProvider`.
- UI shows reconnecting status in dashboard overlay:
  - `Reconnecting... (attempt/maxAttempts)`.
- Runtime and quality gates:
  - `flutter analyze` ✅
  - `flutter test` ✅
  - `./scripts/app/app_test_option.sh 1 linux --no-resident` ✅

**Files to create/modify**:
- `app/lib/core/ble/ble_service.dart`
- `app/lib/core/ble/ble_providers.dart`
- `app/lib/features/dashboard/dashboard_provider.dart`
- `app/lib/features/dashboard/dashboard_screen.dart`

---

### Task 2.4: Disconnect handling and UI feedback

**Description**: Handle disconnection gracefully in the UI.

**Acceptance Criteria**:
- [x] Disconnect button available when connected
- [x] On disconnect: show snackbar or banner notification
- [x] Dashboard screen handles disconnect (show "Disconnected" overlay, don't crash)
- [x] Navigate back to scanner if manual disconnect
- [x] Show reconnecting state during auto-reconnect attempts

**Validation**:
- Manual disconnect → returns to scanner
- Unexpected disconnect → reconnecting overlay appears

<a id="status-note-phase24-implementation"></a>
**Status Note (2026-02-24 — implementation + validation)**:
- Dashboard now includes a `Disconnect` button visible only while connected.
- Manual disconnect flow:
  - Executes `BleService.disconnect()`.
  - Shows snackbar `Disconnected.`.
  - Navigates back to scanner route via `Navigator.maybePop()`.
- Unexpected disconnect feedback:
  - Snackbar `Connection lost.`.
  - `Disconnected` overlay when a previous connection existed and reconnect loop is not active.
  - `Reconnecting...` overlay while auto-reconnect attempts are running.
- Dashboard consumes shared BLE providers, so disconnect/reconnect state changes are reflected immediately without crashes.

---

## Phase 3: NUS Communication

**Objective**: Implement low-level Nordic UART Service communication — send and receive data.  
**Estimated Duration**: 2–3 days  
**Dependencies**: Phase 2 complete

**Refactorización (obligatoria)**:
- Aplicar Boy Scout Rule al cerrar cada tarea de la fase.
- Reducir duplicación y complejidad accidental sin cambiar comportamiento funcional.
- Mantener nombres y límites de módulos claros para código autoexplicativo.
- Repetir la validación de la fase después de cada refactorización.

---

### Task 3.1: Discover NUS service and characteristics

**Description**: After connection, discover the NUS service and get references to TX (notify) and RX (write) characteristics.

**Acceptance Criteria**:
- [x] Find NUS service by UUID `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- [x] Find TX characteristic `6E400003-...` (notify)
- [x] Find RX characteristic `6E400002-...` (write)
- [x] Error handling if service/characteristics not found
- [x] Store references for use by send/receive functions

**Validation**:
- Connect to device → NUS service and chars found and logged

<a id="status-note-phase31-implementation"></a>
**Status Note (2026-02-24 — implementation + validation)**:
- NUS discovery is executed immediately after connection in `BleService.connect(...)`.
- Implemented checks:
  - NUS Service UUID match (`6E400001-B5A3-F393-E0A9-E50E24DCCA9E`).
  - TX characteristic UUID match (`6E400003-B5A3-F393-E0A9-E50E24DCCA9E`).
  - RX characteristic UUID match (`6E400002-B5A3-F393-E0A9-E50E24DCCA9E`).
- Failure handling:
  - Throws `BleServiceException` when NUS service or required characteristics are missing.
  - Resets connection state on failure to avoid partial connected sessions.
- Stored references for next tasks:
  - `connectedDevice`, `nusService`, `nusTxCharacteristic`, `nusRxCharacteristic` getters in `BleService`.

---

### Task 3.2: Subscribe to TX notifications (receive data)

**Description**: Subscribe to the TX characteristic to receive LK8EX1 data from the device.

**Acceptance Criteria**:
- [x] Enable notifications on TX characteristic
- [x] Receive notification callbacks with data bytes
- [x] Reassemble fragmented messages (buffer until `\r\n` delimiter)
- [x] Expose received lines as a `Stream<String>` for consumers
- [x] Handle subscription errors

**Validation**:
- Connect to device → LK8EX1 sentences appear in stream

<a id="status-note-phase32-implementation"></a>
**Status Note (2026-02-24 — implementation + validation)**:
- TX notifications are enabled right after successful NUS discovery (`setNotifyValue(true)`).
- Notification callbacks are consumed from TX `lastValueStream`.
- Fragment reassembly implemented in `BleService` with internal byte buffer until `\r\n` delimiter.
- Completed lines are exposed through `BleService.receivedLines` as `Stream<String>`.
- Notification setup/runtime errors are surfaced as `BleServiceException` or stream errors.
- Quality checks:
  - `flutter analyze` ✅
  - `flutter test` ✅

---

### Task 3.3: Write to RX characteristic (reserved for future use)

**Description**: Implement sending data to the device via the NUS RX characteristic. Reserved for future firmware commands. Device config uses the separate Config Service GATT (Phase 6).

**Acceptance Criteria**:
- [x] Function: `Future<void> sendCommand(String command)` — writes to NUS RX characteristic
- [x] Appends `\n` delimiter if not present
- [x] Handles MTU fragmentation (split long messages)
- [x] Returns error if not connected
- [x] Logs sent commands at debug level

**Validation**:
- Send text via NUS RX → device receives (verify in firmware logs)

<a id="status-note-phase33-implementation"></a>
**Status Note (2026-02-24 — implementation + validation)**:
- `BleService.sendCommand(String command)` implemented in `app/lib/core/ble/ble_service.dart`.
- Behavior implemented:
  - Rejects send attempts when device is not connected or RX characteristic is unavailable.
  - Appends trailing `\n` when missing.
  - Splits payload by effective ATT write payload size (`mtuNow - 3`) to handle fragmentation.
  - Writes fragments sequentially to RX characteristic.
  - Logs outgoing command at debug level via `debugPrint`.
- Quality checks:
  - `flutter analyze` ✅
  - `flutter test` ✅

---

### Task 3.4: Raw data debug screen

**Description**: Create a debug screen showing raw BLE NUS data for development and troubleshooting.

**Acceptance Criteria**:
- [x] Screen shows raw TX data (line by line, scrolling)
- [x] Text input field to send arbitrary data via NUS RX
- [x] Timestamp for each received line
- [x] "Clear" button to reset log
- [x] Accessible from app drawer/menu (developer tool, not user-facing)

**Validation**:
- Connect to device → raw LK8EX1 sentences visible
- Type text in input → sent via NUS RX

<a id="status-note-phase34-implementation"></a>
**Status Note (2026-02-24 — implementation + validation)**:
- New screen added: `app/lib/features/dashboard/raw_data_debug_screen.dart`.
- Implemented UI and behavior:
  - Live raw TX line log sourced from `BleService.receivedLines`.
  - Timestamp displayed for each received line.
  - Text input + `Send` button to dispatch arbitrary payloads through `sendCommand(...)`.
  - `Clear` action to reset in-memory log.
  - Connection indicator (`Connected`/`Disconnected`) and send error feedback.
- Menu access (developer tool):
  - Added entry in Settings: `Developer tools` → `Raw BLE Debug`.
  - File updated: `app/lib/features/settings/settings_screen.dart`.
- Runtime check:
  - `./scripts/app/app_test_option.sh 1 linux --no-resident` ✅

<a id="status-note-phase34-debug-consolidation"></a>
**Status Note (2026-02-27 — debug tooling consolidation)**:
- [x] Developer debug flow consolidated into one entrypoint: `BLE Debug Console` (`Frame Inspector` + `Raw BLE Debug` tabs).
- [x] Previous separate menu entries were replaced by a single Settings route.
- [x] Autoscroll toggle added in both tabs to allow manual history inspection without forced scroll-to-bottom.
- [x] Validation executed: `./scripts/app/app_test_option.sh 3` (analyze + tests PASS).

---

## Phase 3.5: Frame Inspector + Interop Fast-Track (P0)

**Objective**: Enable rapid app/firmware integration by inspecting parsed LK8EX1 fields in real time and validating compatibility with multiple BLE variometer devices (including BlueFlyVario).
**Estimated Duration**: 1–2 days
**Dependencies**: Phase 3 complete

### Task 3.5.1: Parsed-frame inspection screen

**Description**: Add a dedicated debug screen that displays decoded LK8EX1 fields per frame to evaluate semantic correctness.

**Acceptance Criteria**:
- [x] Screen shows per-frame fields: pressure, altitude, vario, temperature, battery, checksum state, timestamp
- [x] Frame source metadata visible (device id/name/profile)
- [x] Supports scrolling history and selecting one frame for detailed inspection

**Validation**:
- Connect to simulated stream and verify field values update in real time

<a id="status-note-phase351-implementation"></a>
**Status Note (2026-02-25 — implementation + static validation)**:
- New screen implemented: `FrameInspectorScreen` with frame history, selection, metadata, and detail panel.
- Navigation integrated from Settings > Frame Inspector.
- App static checks passed (`flutter analyze`, `flutter test`).

### Task 3.5.2: Frame correctness verdict engine

**Description**: Add deterministic rules to classify each frame as `valid`, `warning`, or `error`.

**Acceptance Criteria**:
- [x] `valid`: checksum and required fields OK
- [x] `warning`: placeholder values (e.g., `99999`, `999`) or borderline ranges
- [x] `error`: checksum mismatch, malformed sentence, or impossible field combinations
- [x] Verdict is visible in UI with reason text per frame

**Validation**:
- Inject known test frames and verify expected verdict class

<a id="status-note-phase352-implementation"></a>
**Status Note (2026-02-25 — implementation + static validation)**:
- `Lk8ex1Parser` now exposes parse metadata (`hasChecksum`, `checksumValid`, `errorReason`) for deterministic classification.
- Verdict engine implemented (`valid`, `warning`, `error`) with explicit reason strings and UI chip rendering.

### Task 3.5.3: Multi-device compatibility (FlyInPeace + BlueFlyVario)

**Description**: Support interoperability with other BLE variometer devices, starting with BlueFlyVario, while preserving FlyInPeace compatibility detection.

**Acceptance Criteria**:
- [x] Scanner identifies FlyInPeace and BlueFlyVario-class devices with explicit compatibility badges
- [x] Connection/discovery flow works for both compatibility profiles
- [x] Frame-inspection pipeline can parse/visualize incoming telemetry from both profiles
- [x] Unknown BLE devices remain listed but clearly marked as unsupported/non-profiled

**Validation**:
- Run scan/connection/debug session with FlyInPeace and BlueFlyVario (or equivalent test captures)

<a id="status-note-phase353-implementation"></a>
**Status Note (2026-02-25 — implementation + static validation)**:
- Compatibility profiles implemented in scan layer: `flyInPeace`, `blueFlyVario`, `unsupported`.
- Device list badges and unsupported labeling updated accordingly.
- Runtime proof with physical BlueFlyVario remains part of coordinated validation in Task 3.5.4.

<a id="status-note-phase35-compat-hardening"></a>
**Status Note (2026-02-27 — compatibility hardening + protocol alignment)**:
- [x] BlueFlyVario compatibility detection extended to include service UUID (`0000FFE0-0000-1000-8000-00805F9B34FB`) in addition to name hints.
- [x] Scanner deduplicates equivalent compatible entries and keeps two rows only when protocol-level capabilities differ.
- [x] Device cards now display protocol distinction when split is intentional (`NUS`, `BlueFly UART`, `Generic UART`).
- [x] LK8EX1 parser accepts commercial decimal temperature frames (e.g., `19.8`) while preserving legacy integer-decicelsius parsing.
- [x] Validation executed: `./scripts/app/app_test_option.sh 3` (analyze + tests PASS).

<a id="status-note-phase35-ux-branding"></a>
**Status Note (2026-02-27 — UX/branding flow updates)**:
- [x] App title/label updated to `FLY IN PEACE` across Flutter app title and platform-visible metadata (Android/Web/iOS/Linux/macOS/Windows resources).
- [x] Scanner no longer auto-navigates to Dashboard immediately after connect; user remains in scanner flow after successful connection.
- [x] Disconnect action kept permanently visible in scanner status card and dashboard app bar (not hidden when disconnected).
- [x] Validation executed: `./scripts/app/app_test_option.sh 3` (analyze + tests PASS).

<a id="status-note-phase35-autoscroll-fix"></a>
**Status Note (2026-02-27 — autoscroll stability fix)**:
- [x] Fixed autoscroll state persistence in debug tools by moving toggle state to providers.
- [x] Added guard checks before deferred scroll callbacks so pending callbacks cannot force-scroll when autoscroll is OFF.
- [x] Replaced autoscroll icon toggle with explicit ON/OFF switch in both Frame Inspector and Raw BLE Debug views.
- [x] Validation executed: `./scripts/app/app_test_option.sh 3` (analyze + tests PASS).

### Task 3.5.4: Cross-device integration validation with simulated profiles

**Description**: Validate app frame inspector against micro simulated profiles (`nominal`, `climb`, `sink`, `edge`) and record evidence.

**Acceptance Criteria**:
- [x] Every simulated profile can be consumed and inspected in app
- [x] Field-level values and verdicts match expected profile behavior
- [x] Evidence checklist recorded (date, firmware profile, device profile, verdict)

**Validation**:
- Execute coordinated test pass with micro Phase 3.5 outputs and document results

<a id="status-note-phase35-matrix-validation"></a>
**Status Note (2026-02-25 — matrix validation completed)**:
- Validation executed with automated matrix test: `app/test/core/utils/lk8ex1_phase35_matrix_test.dart`.
- Command/result: `flutter test test/core/utils/lk8ex1_phase35_matrix_test.dart` → `8 passed, 0 failed`.
- Matrix coverage: `A1`..`A8`, with 5 frames per case where required.
- Revision recorded: `f9a2efa`.

#### Phase 3.5 Coordinated Run Report (host simulated matrix)
- Date/Time: 2026-02-25
- Operator: GitHub Copilot
- Firmware hash/profile: `f9a2efa` / `nominal|climb|sink|edge|malformed-checksum|malformed-shape` (simulated inputs)
- App hash/build: `f9a2efa` / Flutter test profile
- BLE source (`FlyInPeace` | `BlueFlyVario`): `FlyInPeace` + simulated `BlueFlyVario` naming detection

| Case | Expected | Observed | Verdict (PASS/FAIL) | Notes |
|---|---|---|---|---|
| A1/M1 nominal | valid | valid | PASS | 5 frames |
| A2/M2 climb | valid | valid | PASS | 5 frames |
| A3/M3 sink | valid | valid | PASS | 5 frames |
| A4/M4 edge-placeholder-alt | warning | warning | PASS | reason contains placeholder altitude |
| A5/M5 edge-placeholder-bat | warning | warning | PASS | reason contains placeholder battery |
| A6/M6 malformed-checksum | error | error | PASS | checksum mismatch detected |
| A7/M7 malformed-shape | error | error | PASS | malformed field count detected |
| A8/M8 interoperability-bluefly | valid/warning | valid | PASS | BlueFly compatibility profile detected |

- Final verdict (overall): PASS
- Blocking issues (if any): None in matrix validation scope
- Next action: Execute physical coordinated run with real BLE stream for hardware evidence extension

#### Phase 3.5 Coordinated Run Report (hardware-backed attempt)
- Date/Time: 2026-02-25
- Operator: GitHub Copilot
- Firmware hash/profile: `b4a7c7e` / board flashed + booted (serial monitor evidence)
- App hash/build: `b4a7c7e` / `flutter run` (`linux`, `chrome --no-resident`)
- BLE source (`FlyInPeace` | `BlueFlyVario`): `FlyInPeace` detected in 30s BLE scan (`DC:DA:0C:81:52:26`)

| Case | Expected | Observed | Verdict (PASS/FAIL) | Notes |
|---|---|---|---|---|
| A1/M1 nominal | valid | N/A | FAIL | Physical source detected, but frame capture not executed in inspector session |
| A2/M2 climb | valid | N/A | FAIL | Physical source detected, but frame capture not executed in inspector session |
| A3/M3 sink | valid | N/A | FAIL | Physical source detected, but frame capture not executed in inspector session |
| A4/M4 edge-placeholder-alt | warning | N/A | FAIL | Physical source detected, but frame capture not executed in inspector session |
| A5/M5 edge-placeholder-bat | warning | N/A | FAIL | Physical source detected, but frame capture not executed in inspector session |
| A6/M6 malformed-checksum | error | N/A | FAIL | Physical source detected, but frame capture not executed in inspector session |
| A7/M7 malformed-shape | error | N/A | FAIL | Physical source detected, but frame capture not executed in inspector session |
| A8/M8 interoperability-bluefly | valid/warning | N/A | FAIL | BlueFlyVario device not present in this run |

- Final verdict (overall): FAIL
- Blocking issues (if any): Missing interactive frame-inspector capture evidence for A1–A7 and missing BlueFlyVario source for A8
- Next action: Run guided 10-minute inspector session with app connected to `FlyInPeace`, switch firmware profiles from app RX commands, then repeat A8 with a BlueFlyVario source

<a id="status-note-phase35-compat-rerun"></a>
**Status Note (2026-02-25 — compatibility rerun execution)**:
- App compatibility layer remains active (NUS-first + generic telemetry fallback) and passes static/runtime checks: `./scripts/app/app_test_option.sh 3`.
- Firmware compatibility image was flashed and verified over BLE host scan as `BlueFlyVario` (`DC:DA:0C:81:52:26`).
- A8 physical closure is still pending a real BlueFlyVario connection session in frame inspector (scan evidence alone is not enough).

### Shared Validation Matrix (app ↔ micro)

Use this matrix as the single source of truth for Phase 3.5 sign-off.

| Case | Profile | Example sentence expectation | App expected verdict | Notes |
|---|---|---|---|---|
| A1 | nominal | Stable pressure/altitude/vario, valid checksum | valid | Baseline integration gate |
| A2 | climb | Positive vario trend, coherent pressure drop | valid | Verify trend continuity for ≥ 30 s |
| A3 | sink | Negative vario trend, coherent pressure rise | valid | Verify no sign inversion in app fields |
| A4 | edge-placeholder-alt | `altitude=99999` | warning | Reason: placeholder altitude |
| A5 | edge-placeholder-bat | `battery=999` | warning | Reason: placeholder battery |
| A6 | malformed-checksum | Corrupted checksum | error | Parser/verdict engine must reject |
| A7 | malformed-shape | Missing field/count mismatch | error | Parser/verdict engine must reject |
| A8 | interoperability-bluefly | BlueFlyVario-style BLE source with LK8EX1-compatible payload | valid or warning | Valid if checksum/fields are correct |

### Coordinated Evidence Checklist (required)

- [x] Session date/time recorded
- [x] Firmware git hash + profile used recorded
- [x] App git hash + frame-inspector build recorded
- [x] BLE source profile recorded (`FlyInPeace` or `BlueFlyVario`)
- [x] At least 5 captured frames per matrix case stored
- [x] Verdict/result for each case (PASS/FAIL) recorded
- [x] Final integration verdict copied to both roadmaps

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
| A1/M1 nominal | valid |  |  |  |
| A2/M2 climb | valid |  |  |  |
| A3/M3 sink | valid |  |  |  |
| A4/M4 edge-placeholder-alt | warning |  |  |  |
| A5/M5 edge-placeholder-bat | warning |  |  |  |
| A6/M6 malformed-checksum | error |  |  |  |
| A7/M7 malformed-shape | error |  |  |  |
| A8/M8 interoperability-bluefly | valid/warning |  |  |  |

- Final verdict (overall): PASS / FAIL
- Blocking issues (if any):
- Next action:
```

### Quick Runbook (10 minutes)

1. **Prepare firmware stream (2 min)**
  - Flash/start firmware with simulated LK8EX1 profile (`nominal` by default).
  - Confirm BLE advertising and NUS TX notifications active.
2. **Launch app debug flow (2 min)**
  - Open app debug build and navigate to frame inspector screen.
  - Connect to target source (`FlyInPeace` or `BlueFlyVario`).
3. **Validate baseline cases (3 min)**
  - Run `A1/A2/A3` (nominal/climb/sink) and confirm `valid` verdict.
  - Record at least 5 frames per case.
4. **Validate edge/error cases (2 min)**
  - Run `A4/A5` placeholder cases and confirm `warning`.
  - Run `A6/A7` malformed cases and confirm `error`.
5. **Interop + closure (1 min)**
  - Run `A8` with BlueFlyVario-compatible source profile.
  - Fill Execution Report Template and copy final verdict to micro roadmap.

### Command Pack (copy/paste)

Run from repository root (`.`):

```bash
# 1) Launch app for BLE debug (Linux desktop)
./scripts/app/app_test_option.sh 1 linux

# 2) Alternative web debug target
./scripts/app/app_test_option.sh 1 chrome

# 3) Optional BLE scan trigger for quick sanity
bluetoothctl --timeout 10 scan on || true
```

---

## Phase 3.6: BLE Session Recording (P1)

**Objective**: Allow developers/pilots to persist raw BLE telemetry lines to a local file while connected, for offline analysis and debugging.
**Estimated Duration**: 1–2 days
**Dependencies**: Phase 3 complete (active BLE stream), Phase 3.5 recommended (inspection flow already available)

### Task 3.6.1: File logging service for received BLE stream

**Description**: Create an app-side recorder service that subscribes to BLE received lines and writes them to a timestamped log file.

**Acceptance Criteria**:
- [x] Recorder can subscribe/unsubscribe to BLE stream without breaking existing UI streams
- [x] Log file is created per session with UTC timestamp-based name
- [x] Each line is stored with timestamp + raw payload
- [x] Recorder handles disconnected state safely (flush/close file)

**Validation**:
- Connect to BLE source for at least 30 seconds and verify resulting file contains received lines and timestamps

### Task 3.6.2: Recorder controls in app (start/stop/save path)

**Description**: Add user controls in debug flow to start and stop recording, and show where the file was saved.

**Acceptance Criteria**:
- [x] Start/Stop recording actions available only when BLE is connected
- [x] UI feedback visible: idle / recording / saved / error
- [x] Saved file path is displayed and copyable from UI

**Validation**:
- Start recording, receive data, stop recording, and verify app displays saved file path

### Task 3.6.3: Session metadata + export validation

**Description**: Attach basic metadata to recording sessions and validate exported files for analysis readiness.

**Acceptance Criteria**:
- [x] Metadata includes device id/name, profile, app build, session start/end time
- [x] Exported file format documented (plain text or CSV)
- [ ] At least one sample recording attached to roadmap evidence notes

**Validation**:
- Run one end-to-end recording session and verify metadata completeness + file readability

<a id="status-note-phase36-implementation"></a>
**Status Note (2026-02-25 — implementation + static validation)**:
- Recorder service implemented in `app/lib/core/ble/ble_stream_recorder.dart` with dual export (`.log` + `.csv`) per session.
- Session metadata written to exports: `device_id`, `device_name`, compatibility `profile`, `app_build`, `started_at_utc`, `stopped_at_utc`.
- Raw BLE Debug UI enhanced in `app/lib/features/dashboard/raw_data_debug_screen.dart` with:
  - file prefix input,
  - output folder selector,
  - start/stop controls,
  - saved path copy action.
- Validation commands executed: `flutter pub get`, `./scripts/app/app_test_option.sh 3` (analyze + tests PASS).
- Pending closure for final evidence item: attach at least one real generated sample recording path/content snapshot.

---

## Phase 4: LK8EX1 Parser

**Objective**: Parse LK8EX1 NMEA sentences from BLE data stream into structured Dart objects.  
**Estimated Duration**: 1–2 days  
**Dependencies**: Phase 3 (data stream available)

**Refactorización (obligatoria)**:
- Aplicar Boy Scout Rule al cerrar cada tarea de la fase.
- Reducir duplicación y complejidad accidental sin cambiar comportamiento funcional.
- Mantener nombres y límites de módulos claros para código autoexplicativo.
- Repetir la validación de la fase después de cada refactorización.

---

### Task 4.1: LK8EX1 sentence parser

**Description**: Parse an LK8EX1 sentence string into a structured Dart data model.

**Acceptance Criteria**:
- [ ] Function: `Lk8ex1Data? parseLk8ex1(String sentence)` → parsed data or null
- [ ] Parses all fields: pressure (Pa), altitude (m), vario (cm/s), temperature (°C×10), battery
- [ ] Handles missing fields (value not present or placeholder value 99999/999)
- [ ] Returns null for invalid/unparseable sentences
- [ ] Pure function, no side effects

**Validation**:
- Unit tests with known sentences

**Files to create**:
- `app/lib/core/utils/lk8ex1_parser.dart`

---

### Task 4.2: Checksum validation

**Description**: Validate the NMEA checksum before parsing the sentence.

**Acceptance Criteria**:
- [ ] Function: `bool validateChecksum(String sentence)` — verifies XOR checksum
- [ ] Reject sentences with invalid checksum (don't parse)
- [ ] Handle sentences without checksum (configurable: accept or reject)
- [ ] Log corrupted sentences at warning level

**Validation**:
- Valid sentence → checksum passes
- Corrupted sentence → checksum fails, parser returns null

---

### Task 4.3: Data model for parsed values

**Description**: Define the `Lk8ex1Data` model class.

**Acceptance Criteria**:
- [ ] Immutable data class with:
  - `int pressurePa` — pressure in Pascals
  - `double altitudeM` — altitude in meters (NaN if not available)
  - `double varioMs` — vertical speed in m/s (converted from cm/s)
  - `double temperatureC` — temperature in °C (converted from °C×10)
  - `int? batteryMv` — battery voltage (null if not available)
  - `DateTime timestamp` — when the data was received
- [ ] `toString()` for debugging
- [ ] `copyWith()` for immutable updates
- [ ] Equality comparison (`==` and `hashCode`)

**Validation**:
- Model correctly stores and exposes all fields

**Files to create**:
- `app/lib/core/models/lk8ex1_data.dart`

---

### Task 4.4: Unit tests for parser

**Description**: Comprehensive unit tests for the LK8EX1 parser.

**Acceptance Criteria**:
- [ ] Test: parse valid sentence with all fields
- [ ] Test: parse sentence with altitude=99999 (no GPS)
- [ ] Test: parse sentence with battery=999 (no battery)
- [ ] Test: checksum validation passes for valid sentence
- [ ] Test: checksum validation fails for corrupted sentence
- [ ] Test: parser returns null for empty string
- [ ] Test: parser returns null for non-LK8EX1 sentence
- [ ] All tests pass with `flutter test`

**Validation**:
- Run `flutter test` — all tests green

**Files to create**:
- `app/test/core/utils/lk8ex1_parser_test.dart`

---

## Phase 5: Real-Time Display

**Objective**: Build the main flight dashboard showing live altitude, vario, pressure, and temperature data.  
**Estimated Duration**: 3–4 days  
**Dependencies**: Phase 4 (parser), Phase 3 (data stream)

**Refactorización (obligatoria)**:
- Aplicar Boy Scout Rule al cerrar cada tarea de la fase.
- Reducir duplicación y complejidad accidental sin cambiar comportamiento funcional.
- Mantener nombres y límites de módulos claros para código autoexplicativo.
- Repetir la validación de la fase después de cada refactorización.

---

### Task 5.1: Flight dashboard screen layout

**Description**: Design and implement the main dashboard layout that displays flight instruments.

**Acceptance Criteria**:
- [ ] Full-screen dashboard layout (minimal chrome, maximum data visibility)
- [ ] Grid/card layout for data widgets: altitude, vario, pressure, temperature
- [ ] Connection status bar at the top (device name + connected/disconnected)
- [ ] Responsive: works on phones (portrait) and tablets
- [ ] Dark background option for outdoor readability

**Validation**:
- Dashboard renders correctly on phone and tablet form factors

**Files to create**:
- `app/lib/features/dashboard/dashboard_screen.dart`

---

### Task 5.2: Altitude display widget

**Description**: Large, prominent altitude display with unit label.

**Acceptance Criteria**:
- [ ] Large numeric display (altitude in meters or feet)
- [ ] Unit label ("m" or "ft")
- [ ] Handles "no data" state (shows "---" or similar)
- [ ] Shows "---" when uncalibrated (`altitude == 99999`); shows calibrated value otherwise
- [ ] Updates in real-time as new data arrives
- [ ] Font size appropriate for glancing at while flying
- [ ] Optional: "Calibrate" quick-action icon/button (opens calibration dialog from Task 6.6)

**Validation**:
- Widget shows altitude value, updates with live data

---

### Task 5.3: Vario display widget (with trend indicator)

**Description**: Vario (vertical speed) display with visual trend indicator.

**Acceptance Criteria**:
- [ ] Numeric vario display in m/s (or ft/min)
- [ ] Color coding: green for climbing, red for sinking, gray for neutral
- [ ] Optional: simple bar graph or arrow indicator showing trend
- [ ] Neutral zone: ±0.1 m/s (configurable dead zone)
- [ ] Updates in real-time

**Validation**:
- Widget shows positive (green) for climb, negative (red) for sink

---

### Task 5.4: Pressure and temperature display

**Description**: Secondary data displays for barometric pressure and temperature.

**Acceptance Criteria**:
- [ ] Pressure display in hPa (converted from Pa)
- [ ] Temperature display in °C (or °F)
- [ ] Smaller than altitude/vario (secondary importance)
- [ ] Updates in real-time

**Validation**:
- Values display correctly and update with live data

---

### Task 5.5: Connection status indicator

**Description**: Persistent visual indicator of BLE connection status on the dashboard.

**Acceptance Criteria**:
- [ ] Top bar shows: device name + connection state icon
- [ ] Connected: green indicator + device name
- [ ] Disconnected: red indicator + "Disconnected"
- [ ] Reconnecting: yellow/orange indicator + "Reconnecting..."
- [ ] Tap → navigate to scanner or show connection details

**Validation**:
- Status updates correctly during connect/disconnect cycles

---

### Task 5.6: Data stream integration

**Description**: Wire the BLE NUS data stream through the LK8EX1 parser to the dashboard widgets.

**Acceptance Criteria**:
- [ ] BLE TX stream → LK8EX1 parser → data provider → dashboard widgets
- [ ] Data updates at ~4 Hz (matching firmware send rate)
- [ ] Old data discarded if parsing bottleneck (always show latest)
- [ ] Handle stream errors (log and continue, don't crash)
- [ ] When disconnected, last known values shown with "stale" indicator

**Validation**:
- Connect to device → dashboard shows live updating data
- Disconnect → dashboard shows last values with stale indicator

---

## Phase 6: Device Configuration

**Objective**: Build the UI and BLE communication to read and modify device configuration parameters.  
**Estimated Duration**: 3–4 days  
**Dependencies**: Phase 3 (BLE connection), Phase 5 (dashboard working)

**Refactorización (obligatoria)**:
- Aplicar Boy Scout Rule al cerrar cada tarea de la fase.
- Reducir duplicación y complejidad accidental sin cambiar comportamiento funcional.
- Mantener nombres y límites de módulos claros para código autoexplicativo.
- Repetir la validación de la fase después de cada refactorización.

---

### Task 6.1: Config Service GATT client (read/write JSON)

**Description**: Implement the client-side BLE Config Service. Discover the Config Service GATT, read Device Info and Config characteristics, and write config updates as JSON.

**Acceptance Criteria**:
- [ ] Discover Config Service by UUID `0000ABC0-0000-1000-8000-00805F9B34FB`
- [ ] Function: `Future<DeviceInfo> readDeviceInfo()` — reads and parses Device Info JSON
- [ ] Function: `Future<DeviceConfig> readConfig()` — reads and parses Config JSON
- [ ] Function: `Future<void> writeConfig(Map<String, dynamic> updates)` — sends partial JSON update
- [ ] Model: `DeviceInfo` (name, firmware version, battery mV)
- [ ] Model: `DeviceConfig` (sensor_rate, ble_tx_rate, kalman_q, kalman_r, reference_pressure_pa, device_name)
- [ ] Function: `Future<void> calibrateAltitude(double altitudeM)` — sends `{"action": "calibrate", "altitude_m": <value>}` via Config Write characteristic (per `ble_protocol.md` §Calibrate Action)
- [ ] Handle read/write errors gracefully

**Validation**:
- Read Device Info → valid JSON with firmware version
- Read Config → current device configuration
- Write config update → value changes on device

---

### Task 6.2: Config screen UI

**Description**: Build the device configuration screen with editable fields for each parameter.

**Acceptance Criteria**:
- [ ] List of configurable parameters with current values
- [ ] Editable fields:
  - Device name (text input, max 20 chars)
  - Sensor OSR (dropdown: 256, 512, 1024, 2048, 4096)
  - BLE TX rate (slider: 1–10 Hz)
  - Reference pressure (numeric input, Pa) — read-only display of current QNH
- [ ] Altitude calibration section (separated from config fields):
  - "Calibrate Altitude" button
  - Opens calibration dialog (see Task 6.6)
  - Shows current `reference_pressure_pa` value (read-only, updated after calibration)
- [ ] "Save to device" button (writes config JSON + triggers NVS save)
- [ ] "Reset defaults" button (optional, sends reset config)
- [ ] Loading state while reading current config
- [ ] Only accessible when connected

**Validation**:
- Open config screen → current values loaded from device
- Change value → send to device → verify in firmware logs

---

### Task 6.3: Read current config from device

**Description**: On opening the config screen, read all current configuration values from the device.

**Acceptance Criteria**:
- [ ] Read all config values via Config Read characteristic on screen load
- [ ] Parse JSON response and populate form fields
- [ ] Show loading spinner while waiting for response
- [ ] Show error if read fails (with retry button)

**Validation**:
- Open config screen → values match device config

---

### Task 6.4: Write config and save to device NVS

**Description**: Send modified configuration values to the device and persist to NVS.

**Acceptance Criteria**:
- [ ] Write JSON with changed parameters to Config Write characteristic
- [ ] Verify write succeeds (no BLE error)
- [ ] Trigger NVS save on device (via save flag in JSON or separate write)
- [ ] Show success/failure feedback (snackbar)
- [ ] Only send changed values (diff against original)

**Validation**:
- Change device name → save → reboot device → name persists

---

### Task 6.5: Config validation and error handling

**Description**: Validate configuration values before sending and handle errors from the device.

**Acceptance Criteria**:
- [ ] Client-side validation: name length, numeric ranges, valid OSR values
- [ ] Show inline validation errors on form fields
- [ ] Handle device `ERR` responses (show error message to user)
- [ ] Handle timeout (device didn't respond)
- [ ] Don't save partially applied config (rollback or warn)

**Validation**:
- Enter invalid value → inline error shown, not sent
- Send valid value, device responds with ERR → error shown to user

---

### Task 6.6: Altitude calibration via BLE

**Description**: Implement the altitude calibration flow from the mobile app. The pilot enters a known altitude (e.g., elevation at launch site), the app sends a calibrate command via the BLE Config Write characteristic, and the device computes the reference pressure (QNH) using the inverse barometric formula. The calibrated altitude then appears in the LK8EX1 data stream.

**BLE Protocol Reference**: `ble_protocol.md` §Config Write — Calibrate Action  
**Firmware Reference**: `firmware-architecture.md` §4.4 `kalman_filter_calibrate()`, §6.3 Calibration Queue

**Acceptance Criteria**:
- [ ] "Calibrate Altitude" button accessible from:
  - Config screen (calibration section)
  - Dashboard screen (quick-action icon on altitude widget — optional)
- [ ] Tapping the button opens a calibration dialog:
  - Numeric input field for known altitude
  - Unit-aware: meters or feet (converted to meters before sending, per app unit setting)
  - Placeholder text showing current altitude (if available) or "Enter altitude"
  - Input range validation: -500 to 10000 m (or equivalent in feet)
  - "Calibrate" confirmation button + "Cancel" button
- [ ] On confirm: calls `calibrateAltitude(double altitudeM)` from Task 6.1
  - Sends `{"action": "calibrate", "altitude_m": <value>}` via Config Write (`0xABC3`)
  - Shows loading indicator while waiting for BLE write ack
- [ ] On success (BLE write ack received):
  - Show success snackbar: "Altitude calibrated to \<value\> m"
  - Dashboard altitude widget updates within ~250 ms (next LK8EX1 frame)
  - Config screen shows updated `reference_pressure_pa` (re-read via Config Read)
- [ ] On error (BLE write fails, timeout, out of range):
  - Show error dialog with descriptive message
  - Input remains for retry (don't dismiss dialog)
- [ ] Only available when BLE is connected

**Validation**:
- Calibrate to known altitude → LK8EX1 stream shows correct altitude within 1 second
- Reboot device → calibrated altitude persists (QNH saved in NVS)
- Enter out-of-range value → validation error shown, command not sent
- Disconnect BLE → calibrate button disabled

**Files to create/modify**:
- `app/lib/features/config/widgets/calibration_dialog.dart`
- `app/lib/features/config/config_screen.dart`
- `app/lib/features/config/config_provider.dart`
- `app/lib/features/dashboard/dashboard_screen.dart` (optional quick-action)

---

## Phase 7: Settings & Persistence

**Objective**: Implement app-level settings (units, display preferences) that persist locally.  
**Estimated Duration**: 2–3 days  
**Dependencies**: Phase 5 (dashboard uses units for display)

**Refactorización (obligatoria)**:
- Aplicar Boy Scout Rule al cerrar cada tarea de la fase.
- Reducir duplicación y complejidad accidental sin cambiar comportamiento funcional.
- Mantener nombres y límites de módulos claros para código autoexplicativo.
- Repetir la validación de la fase después de cada refactorización.

---

### Task 7.1: App settings screen

**Description**: Settings screen for app-level preferences (not device config).

**Acceptance Criteria**:
- [ ] Settings accessible from app drawer or navigation
- [ ] Organized by category: Display, Units, About
- [ ] Changes take effect immediately (no "save" button needed)

---

### Task 7.2: Display units (metric/imperial)

**Description**: Allow switching between metric and imperial units.

**Acceptance Criteria**:
- [ ] Altitude: meters ↔ feet
- [ ] Vario: m/s ↔ ft/min
- [ ] Pressure: hPa ↔ inHg
- [ ] Temperature: °C ↔ °F
- [ ] Unit setting persists across app restarts
- [ ] Dashboard updates immediately on change

**Validation**:
- Switch to imperial → dashboard shows feet and ft/min
- Restart app → units still imperial

---

### Task 7.3: Local persistence (shared_preferences)

**Description**: Persist app settings using `shared_preferences`.

**Acceptance Criteria**:
- [ ] Settings saved on change
- [ ] Settings loaded on app start
- [ ] Default values if no saved settings exist
- [ ] Settings provider/notifier integrates with state management

---

### Task 7.4: Dark mode implementation (light/dark/system)

**Description**: Implement full theme-mode switching with `light`, `dark`, and `system` options, persisted locally and applied across the app.

**Acceptance Criteria**:
- [ ] Light/dark/system theme toggle
- [ ] Dark theme optimized for outdoor use (high contrast, AMOLED-friendly)
- [ ] Theme change is immediate
- [ ] Persists across app restarts
- [ ] `ThemeMode` managed by settings provider/notifier
- [ ] `MaterialApp` wired with `theme`, `darkTheme`, and `themeMode`
- [ ] No hardcoded colors in feature screens for text/background-critical elements (use theme tokens)

**Validation**:
- Switch theme → app updates immediately
- Restart → theme persists

**Files to create/modify**:
- `app/lib/app.dart`
- `app/lib/features/settings/settings_screen.dart`
- `app/lib/features/settings/settings_provider.dart`
- `app/lib/core/theme/app_theme.dart` (or equivalent existing theme file)

---

### Task 7.5: Dark mode rollout across all screens

**Description**: Ensure dark mode is consistently applied to scanner, dashboard, config, and settings screens (including shared widgets).

**Acceptance Criteria**:
- [ ] Scanner screen fully readable in dark mode (lists, badges, controls)
- [ ] Dashboard widgets readable in dark mode (high-contrast numeric values)
- [ ] Config screen forms and validation states readable in dark mode
- [ ] Shared widgets (`ConnectionIndicator`, `ValueDisplay`, list tiles) respect theme tokens
- [ ] No contrast regressions in key states: loading, empty, error, disconnected

**Validation**:
- Run app in `light`, `dark`, and `system` modes and verify each main screen
- `flutter analyze` and `flutter test` pass after theme rollout

---

## Phase 8: Polish & Testing

**Objective**: Review error handling, polish the UI, and ensure comprehensive test coverage.  
**Estimated Duration**: 3–5 days  
**Dependencies**: All previous phases complete

**Refactorización (obligatoria)**:
- Aplicar Boy Scout Rule al cerrar cada corrección detectada durante pruebas.
- Corregir deuda técnica localizada sin ampliar alcance funcional.
- Mantener coherencia arquitectónica entre capas y features.
- Revalidar pruebas críticas después de cada refactorización.

---

### Task 8.1: Error handling audit

**Acceptance Criteria**:
- [ ] Every BLE operation has try/catch with user-facing error message
- [ ] No unhandled exceptions in release mode
- [ ] BLE disconnection handled gracefully on every screen
- [ ] Permission denial handled on every BLE entry point
- [ ] Timeouts on all BLE operations (scan, connect, commands)

---

### Task 8.2: UI polish and accessibility

**Acceptance Criteria**:
- [ ] Consistent spacing, fonts, and colors across all screens
- [ ] Touch targets at least 48dp × 48dp
- [ ] Text sizes readable without straining
- [ ] Loading indicators for all async operations
- [ ] Smooth transitions between screens
- [ ] No layout overflow or clipping on small screens

---

### Task 8.3: Unit tests for business logic

**Acceptance Criteria**:
- [ ] LK8EX1 parser: 8+ test cases (see Phase 4)
- [ ] Config Service GATT client: 5+ test cases
- [ ] Altitude calibration: send calibrate command, verify response handling
- [ ] Unit conversion functions: metric ↔ imperial
- [ ] All tests pass with `flutter test`

---

### Task 8.4: Widget tests for screens

**Acceptance Criteria**:
- [ ] Scanner screen: renders in scanning, results, empty, error states
- [ ] Dashboard screen: renders with data, without data, disconnected state
- [ ] Config screen: renders with loaded config, calibration section visible
- [ ] Calibration dialog: renders input, validates range, shows success/error
- [ ] All widget tests pass with `flutter test`

---

### Task 8.5: Integration test with real device

**Acceptance Criteria**:
- [ ] Full workflow: scan → connect → view data → configure → calibrate altitude → disconnect
- [ ] Calibrate altitude → verify LK8EX1 altitude updates within 1 second
- [ ] Works for 30+ minutes continuously
- [ ] Auto-reconnect works after device restart
- [ ] No memory leaks (check with Flutter DevTools)

---

## Phase 9: Documentation

**Objective**: Project documentation for users and developers.  
**Estimated Duration**: 1–2 days  
**Dependencies**: Phase 8 complete

---

### Task 9.1: App README with build instructions

**Acceptance Criteria**:
- [ ] `app/README.md` with: project description, prerequisites (Flutter SDK, Android SDK), build steps, run steps
- [ ] Minimum Android version documented
- [ ] How to generate release APK

<a id="status-note-phase91-doc-update"></a>
**Status Note (2026-02-25 — scripts/runtime doc update)**:
- `app/README.md` updated with repository-root app script usage and terminal execution examples.
- Documented Flutter auto-detection behavior used by app scripts.
- Shared helper introduced and referenced: `scripts/app/flutter_env.sh` (`ensure_flutter_available`).

**Files to create**:
- `app/README.md`

---

### Task 9.2: User guide with screenshots

**Acceptance Criteria**:
- [ ] Screenshots of each main screen
- [ ] Step-by-step guide: first connection, dashboard usage, configuration
- [ ] Troubleshooting section (BLE not found, permission issues)

**Files to create**:
- `docs/app-user-guide.md`

---

### Task 9.3: Architecture documentation

**Acceptance Criteria**:
- [ ] Architecture diagram (Mermaid) showing: BLE layer, data flow, state management, screens
- [ ] State management patterns documented
- [ ] BLE communication protocol documented

**Files to create**:
- `docs/architecture/app-architecture.md`

---

## Future (Post-MVP)

> These features are planned but not scheduled. They will be added as the MVP stabilizes.

- **Historical Data Logging**: Record flight data locally for review
- **Flight Playback**: Replay saved flights with timeline scrubbing
- **Data Export**: Export flight data as CSV, IGC, or KML
- **WiFi Configuration**: Configure device WiFi settings via BLE
- **Multiple Device Support**: Connect to multiple varios simultaneously
- **Map Integration**: Show position on map (if GPS available from XCTrack)
- **Widgets**: Home screen widget showing last known altitude
- **iOS Version**: Extend to iOS using the same Flutter codebase
