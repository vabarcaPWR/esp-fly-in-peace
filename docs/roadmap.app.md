# Roadmap — Mobile App (Flutter / Android)

> **Project**: esp-fly-in-peace  
> **Component**: Mobile App (`app/`)  
> **Technology**: Flutter (Dart) — Android only (MVP)  
> **BLE Plugin**: `flutter_blue_plus`  
> **Master reference**: `.github/PRE-PROMPT.md`

---

## Summary Checklist

- [ ] **Phase 0: Project Bootstrap**
  - [ ] Task 0.1: Create Flutter project
  - [ ] Task 0.2: Configure dependencies (BLE, state management, etc.)
  - [ ] Task 0.3: Choose and set up state management
  - [ ] Task 0.4: Define project structure and architecture
  - [ ] Task 0.5: Create app theme and common widgets
  - [ ] Task 0.6: Verify build and run on Android device/emulator
- [ ] **Phase 1: BLE Scanner**
  - [ ] Task 1.1: Android BLE permissions handling
  - [ ] Task 1.2: BLE scan functionality with NUS UUID filter
  - [ ] Task 1.3: Device list UI (name, RSSI, connect button)
  - [ ] Task 1.4: Pull-to-refresh and scan timeout
- [ ] **Phase 2: BLE Connection**
  - [ ] Task 2.1: Connect to device
  - [ ] Task 2.2: Connection state management
  - [ ] Task 2.3: Auto-reconnect logic
  - [ ] Task 2.4: Disconnect handling and UI feedback
- [ ] **Phase 3: NUS Communication**
  - [ ] Task 3.1: Discover NUS service and characteristics
  - [ ] Task 3.2: Subscribe to TX notifications (receive data)
  - [ ] Task 3.3: Write to RX characteristic (reserved for future use)
  - [ ] Task 3.4: Raw data debug screen
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
  - [ ] Task 7.4: Theme configuration (light/dark)
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
- [ ] Flutter project created at `app/`
- [ ] `android/app/build.gradle` sets `minSdkVersion 21` (for BLE), `targetSdkVersion 34`
- [ ] Project name: `fly_in_peace`
- [ ] Package/application ID: `com.flyinpeace.app` (or similar)
- [ ] `flutter run` launches the default counter app on Android device/emulator

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
- [ ] `flutter_blue_plus` added for BLE communication
- [ ] State management package added (see Task 0.3)
- [ ] `shared_preferences` added for local app settings
- [ ] `permission_handler` added for BLE permissions
- [ ] `flutter_lints` or `very_good_analysis` for lint rules
- [ ] `flutter pub get` succeeds with no dependency conflicts

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
- [ ] State management choice confirmed by developer
- [ ] Package installed and basic provider structure created
- [ ] A sample provider works (e.g., counter or connection state)

**Validation**:
- App compiles and runs with state management integrated

---

### Task 0.4: Define project structure and architecture

**Description**: Create the directory structure following a feature-based architecture that keeps BLE, data models, and UI separate.

**Acceptance Criteria**:
- [ ] Directory structure created:
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
- [ ] Each directory has a placeholder file or `.gitkeep`

**Validation**:
- Project compiles with empty structure

---

### Task 0.5: Create app theme and common widgets

**Description**: Set up the app's visual theme (Material 3) and create reusable base widgets.

**Acceptance Criteria**:
- [ ] Material 3 theme configured in `app.dart`
- [ ] Color scheme defined (flight/outdoor theme: blues, greens, grays)
- [ ] Text theme defined (readable at a glance, large data values)
- [ ] Common widget: `ConnectionIndicator` (green dot = connected, red dot = disconnected)
- [ ] Common widget: `ValueDisplay` (label + large value + unit, reusable for altitude/vario/pressure)
- [ ] Dark mode support (optional for MVP, but theme structure should allow it)

**Validation**:
- App displays themed widgets correctly

---

### Task 0.6: Verify build and run on Android device/emulator

**Description**: End-to-end verification: the Flutter app builds, installs, and runs on an Android device.

**Acceptance Criteria**:
- [ ] `flutter build apk --debug` succeeds
- [ ] App installs and launches on Android device/emulator
- [ ] Basic navigation between placeholder screens works
- [ ] No crash or build errors

**Validation**:
- Install APK on physical device, navigate between screens

**Notes**: This is the gate for Phase 0. Do not proceed to Phase 1 until this passes.

---

## Phase 1: BLE Scanner

**Objective**: Implement BLE device scanning, filtering by NUS service UUID, and displaying discovered devices.  
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
- [ ] Request `BLUETOOTH_SCAN` and `BLUETOOTH_CONNECT` on Android 12+ (API 31+)
- [ ] Request `ACCESS_FINE_LOCATION` on Android 11 and below
- [ ] Handle "permission denied" gracefully — show explanation dialog
- [ ] Handle "permission permanently denied" — direct user to app settings
- [ ] Check if Bluetooth adapter is enabled — prompt to enable if off
- [ ] Check if Location services are enabled (required for BLE scan on some Android versions)

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

### Task 1.2: BLE scan functionality with NUS UUID filter

**Description**: Implement BLE scanning using `flutter_blue_plus`, filtered to show only devices advertising the NUS service UUID.

**Acceptance Criteria**:
- [ ] Start/stop scanning via `FlutterBluePlus.startScan()` / `stopScan()`
- [ ] Filter by NUS service UUID: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- [ ] Also show devices with name containing "FlyInPeace" (for devices that don't advertise service UUID)
- [ ] Scan timeout: 10 seconds (configurable)
- [ ] Deduplicate results (same device MAC)
- [ ] Expose scan results as a stream/provider for the UI

**Validation**:
- Turn on ESP32-C3 with BLE firmware → device appears in scan results

**Files to create/modify**:
- `app/lib/core/ble/ble_service.dart`
- `app/lib/features/scanner/scanner_provider.dart`

---

### Task 1.3: Device list UI (name, RSSI, connect button)

**Description**: Build the scanner screen UI showing discovered BLE devices.

**Acceptance Criteria**:
- [ ] List view showing each device: name (or "Unknown"), MAC address, RSSI signal indicator
- [ ] "Scan" FAB button to start/stop scanning
- [ ] Scanning indicator (spinner or animation) while scan is active
- [ ] Tap on device → navigate to connection / dashboard
- [ ] Empty state: "No devices found. Make sure your vario is powered on."
- [ ] RSSI shown as signal bars or dBm value

**Validation**:
- Scan shows ESP32-C3 device with correct name and signal strength

**Files to create/modify**:
- `app/lib/features/scanner/scanner_screen.dart`
- `app/lib/widgets/device_list_tile.dart`

---

### Task 1.4: Pull-to-refresh and scan timeout

**Description**: Add pull-to-refresh gesture and automatic scan timeout with "rescan" button.

**Acceptance Criteria**:
- [ ] Pull down on device list → restart scan
- [ ] Scan automatically stops after timeout (10 seconds)
- [ ] "Scan again" button appears after scan completes
- [ ] Scan progress indicator (e.g., linear progress bar showing time remaining)

**Validation**:
- Pull down → scan restarts
- Wait 10 seconds → scan stops, "Scan again" visible

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
- [ ] `BleService.connect(BluetoothDevice device)` connects to the specified device
- [ ] Connection timeout: 10 seconds
- [ ] Negotiate MTU to maximum (request 512, accept whatever the device supports)
- [ ] Discover services after connection
- [ ] Verify NUS service is present — error if not

**Validation**:
- Tap device in scanner → connection established, services discovered

**Files to modify**:
- `app/lib/core/ble/ble_service.dart`

---

### Task 2.2: Connection state management

**Description**: Track and expose BLE connection state throughout the app.

**Acceptance Criteria**:
- [ ] Connection states: `disconnected`, `connecting`, `connected`, `disconnecting`
- [ ] State exposed via provider/stream (accessible from any screen)
- [ ] State updates immediately on connect/disconnect events
- [ ] All screens can react to connection state changes

**Validation**:
- Connection indicator widget shows correct state at all times

**Files to create/modify**:
- `app/lib/core/ble/ble_service.dart`
- `app/lib/features/scanner/scanner_provider.dart` (or a connection_provider)

---

### Task 2.3: Auto-reconnect logic

**Description**: Automatically attempt to reconnect when the device disconnects unexpectedly.

**Acceptance Criteria**:
- [ ] On unexpected disconnect, wait 2 seconds then retry
- [ ] Retry up to 5 times with exponential backoff (2s, 4s, 8s, 16s, 30s)
- [ ] Show "Reconnecting..." in UI during retries
- [ ] Stop retrying if user manually disconnects
- [ ] Stop retrying if device is out of range for all attempts

**Validation**:
- Turn off ESP32-C3 briefly → app attempts reconnection
- Turn back on → app reconnects automatically

---

### Task 2.4: Disconnect handling and UI feedback

**Description**: Handle disconnection gracefully in the UI.

**Acceptance Criteria**:
- [ ] Disconnect button available when connected
- [ ] On disconnect: show snackbar or banner notification
- [ ] Dashboard screen handles disconnect (show "Disconnected" overlay, don't crash)
- [ ] Navigate back to scanner if manual disconnect
- [ ] Show reconnecting state during auto-reconnect attempts

**Validation**:
- Manual disconnect → returns to scanner
- Unexpected disconnect → reconnecting overlay appears

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
- [ ] Find NUS service by UUID `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- [ ] Find TX characteristic `6E400003-...` (notify)
- [ ] Find RX characteristic `6E400002-...` (write)
- [ ] Error handling if service/characteristics not found
- [ ] Store references for use by send/receive functions

**Validation**:
- Connect to device → NUS service and chars found and logged

---

### Task 3.2: Subscribe to TX notifications (receive data)

**Description**: Subscribe to the TX characteristic to receive LK8EX1 data from the device.

**Acceptance Criteria**:
- [ ] Enable notifications on TX characteristic
- [ ] Receive notification callbacks with data bytes
- [ ] Reassemble fragmented messages (buffer until `\r\n` delimiter)
- [ ] Expose received lines as a `Stream<String>` for consumers
- [ ] Handle subscription errors

**Validation**:
- Connect to device → LK8EX1 sentences appear in stream

---

### Task 3.3: Write to RX characteristic (reserved for future use)

**Description**: Implement sending data to the device via the NUS RX characteristic. Reserved for future firmware commands. Device config uses the separate Config Service GATT (Phase 6).

**Acceptance Criteria**:
- [ ] Function: `Future<void> sendCommand(String command)` — writes to NUS RX characteristic
- [ ] Appends `\n` delimiter if not present
- [ ] Handles MTU fragmentation (split long messages)
- [ ] Returns error if not connected
- [ ] Logs sent commands at debug level

**Validation**:
- Send text via NUS RX → device receives (verify in firmware logs)

---

### Task 3.4: Raw data debug screen

**Description**: Create a debug screen showing raw BLE NUS data for development and troubleshooting.

**Acceptance Criteria**:
- [ ] Screen shows raw TX data (line by line, scrolling)
- Text input field to send arbitrary data via NUS RX
- Timestamp for each received line
- "Clear" button to reset log
- [ ] Accessible from app drawer/menu (developer tool, not user-facing)

**Validation**:
- Connect to device → raw LK8EX1 sentences visible
- Type text in input → sent via NUS RX

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

### Task 7.4: Theme configuration (light/dark)

**Description**: Allow switching between light and dark themes.

**Acceptance Criteria**:
- [ ] Light/dark/system theme toggle
- [ ] Dark theme optimized for outdoor use (high contrast, AMOLED-friendly)
- [ ] Theme change is immediate
- [ ] Persists across app restarts

**Validation**:
- Switch theme → app updates immediately
- Restart → theme persists

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
