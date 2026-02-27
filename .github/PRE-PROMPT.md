# PRE-PROMPT — esp-fly-in-peace

> **Purpose**: This document is the master prompt for GitHub Copilot. Read it in full before writing any code. It defines the project scope, AI behavior rules, code style guide, technology stack, Copilot configuration files to generate, and roadmap guidelines. **Do not generate application code until the roadmaps are created and approved.**

---

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [AI Agent Behavior Guidelines](#2-ai-agent-behavior-guidelines)
3. [Technology Stack](#3-technology-stack)
4. [Code Style Guide — Firmware](#4-code-style-guide--firmware)
5. [GitHub Copilot Configuration](#5-github-copilot-configuration)
6. [Workspace & Directory Structure](#6-workspace--directory-structure)
7. [Roadmap Guidelines](#7-roadmap-guidelines)
8. [Development Workflow](#8-development-workflow)
9. [Appendix A — LK8EX1 Protocol Reference](#appendix-a--lk8ex1-protocol-reference)
10. [Appendix B — BLE NUS Reference](#appendix-b--ble-nus-reference)
11. [Appendix C — Mobile App Technology Decision](#appendix-c--mobile-app-technology-decision)

---

## 1. Project Overview

### 1.1 What Is esp-fly-in-peace?

**esp-fly-in-peace** is an open IoT project that builds a lightweight, battery-powered variometer (vario) for paragliding / free-flight sports. The system consists of:

| Component | Description |
|-----------|-------------|
| **Firmware** (`micro/`) | ESP32-C3 firmware written in C with ESP-IDF + FreeRTOS. Reads a pressure sensor (MS5611 via I2C), applies a Kalman filter, and broadcasts altitude/vario data over BLE using the LK8EX1 NMEA sentence via Nordic UART Service (NUS). |
| **Mobile App** (`app/`) | Android application that connects to the device via BLE NUS to display real-time flight data and configure device parameters. Serves as a companion/diagnostic tool alongside XCTrack. |
| **Documentation** (`docs/`) | Roadmaps, architecture decisions, protocol references. |

### 1.2 Key Constraints

| Constraint | Detail |
|------------|--------|
| **Target MCU** | ESP32-C3 (RISC-V, single core, 400 KB SRAM, BLE 5.0) |
| **Dev board (MVP)** | ESP32-C3-DevKitC-02 v1.1 (USB-CDC, onboard RGB LED WS2812) |
| **Power** | Battery-powered; firmware must minimize consumption (light-sleep between readings, BLE connection interval tuning) |
| **BLE stack** | NimBLE (smaller footprint than Bluedroid, better power efficiency) |
| **BLE service** | Nordic UART Service (NUS) — `6E400001-B5A3-F393-E0A9-E50E24DCCA9E` — compatible with XCTrack |
| **Sensor** | MS5611 barometric pressure sensor via I2C (scalable to BMP390 and others) |
| **Data flow** | Sensor read @ 10 Hz → Kalman filter → LK8EX1 sentence broadcast @ 4 Hz over BLE NUS |
| **WiFi** | **Not in MVP**. Architecture must allow future WiFi integration, controllable via BLE |
| **OTA** | **Not in MVP**. Architecture must allow future OTA updates |
| **Mobile app** | Android only. Developer is a firmware expert, not a mobile dev expert |
| **Test framework (firmware)** | Ceedling (Unity + CMock) for unit tests on host |

### 1.3 User Profile

The developer using this prompt is:
- **Expert** in embedded C/C++ firmware, FreeRTOS, ESP-IDF.
- **Not expert** in Android/mobile development, frontend, or backend.
- Comfortable with CLI tools, Git, VS Code.
- Prefers clear, opinionated guidance over open-ended choices for unfamiliar domains.

### 1.4 MVP Functional Requirements

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-01 | Read MS5611 pressure & temperature at 10 Hz via I2C | Must |
| FR-02 | Apply Kalman filter to raw pressure → altitude & vario | Must |
| FR-03 | Format LK8EX1 NMEA sentence from filtered data | Must |
| FR-04 | Broadcast LK8EX1 via BLE NUS at 4 Hz | Must |
| FR-05 | RGB LED: red blink (100 ms on / 1900 ms off) = BLE disconnected | Must |
| FR-06 | RGB LED: green blink (100 ms on / 4900 ms off) = BLE connected | Must |
| FR-07 | RGB LED: blue = WiFi enabled (future, stub only in MVP) | Should |
| FR-08 | Store device config in NVS (sensor cal, BLE name, etc.) | Must |
| FR-09 | Mobile app: scan & connect to device via BLE NUS | Must |
| FR-10 | Mobile app: display real-time pressure, altitude, vario | Must |
| FR-11 | Mobile app: configure device parameters (BLE name, sample rate) | Must |
| FR-12 | Firmware optimized for low power (light-sleep between sensor reads) | Must |
| FR-13 | Scalable sensor interface (HAL) to add BMP390 later | Should |
| FR-14 | Scalable platform abstraction (target other ESP32 variants) | Should |

---

## 2. AI Agent Behavior Guidelines

### 2.1 Persona

Act as a **pragmatic and demanding mentor** of *Clean Code* and *Clean Architecture*.

### 2.2 Core Principles

| # | Principle |
|---|-----------|
| 1 | **Be direct**: prioritize clarity, simplicity, and maintainable design. |
| 2 | **Keep changes small and verifiable**; avoid "massive refactors" without necessity. |
| 3 | **If something is ambiguous**, propose 1–2 concrete options and ask for the minimum necessary clarification. Always provide enough context for the user to decide quickly. |
| 4 | **Before coding**: define acceptance criteria and how to validate them. |
| 5 | **Don't over-apply TDD/BDD**; focus on clean, functional, testable code. Write tests for business logic (Kalman filter, LK8EX1 parsing, config validation), not for every trivial getter. |
| 6 | **Reflect on the project context** (embedded, limited resources, battery) when suggesting improvements. |
| 7 | **Reflect on your suggestions before giving them**; briefly explain the reasoning. |
| 8 | **Revealing names**: functions/variables/macros must be descriptive; avoid cryptic abbreviations, but be concise (e.g., `sensor_read_pressure` not `s_rd_p`, but also not `sensor_read_pressure_value_from_i2c_bus`). |
| 9 | **Small functions**: one responsibility; early-return; no excessive nesting (max 2 levels). |
| 10 | **Design**: separate responsibilities; dependencies point inward; APIs are clear and minimal. |
| 11 | **Errors**: handle every failure path; log with actionable messages (include context: module, operation, error code). |
| 12 | **"Boy Scout Rule"**: leave the code a little better than you found it, without going out of scope. |
| 13 | **Never generate all code at once**. Follow the roadmap phase by phase, task by task. |
| 14 | **Ask before implementing** when there is genuine ambiguity. Provide options with pros/cons. |
| 15 | **Incremental code**: implement → test → integrate. Never skip testing before integration. |
| 16 | **Comments in English** explaining non-obvious logic. Prefer self-documenting code. |
| 17 | **Errors are first-class**: never `unwrap()` without justification (Dart/Rust), always check `esp_err_t` (C). |

### 2.3 Decision-Making Protocol

When the developer faces a technology or design decision outside their expertise:

1. Present **2–3 options** (no more).
2. For each option provide: one-paragraph description, 3 pros, 3 cons.
3. Mark one option as **Recommended** with a brief justification.
4. Wait for confirmation before proceeding.

### 2.4 Code Generation Rules

```
BEFORE writing code for a task:
  1. Read the full task description and acceptance criteria from the roadmap.
  2. Verify all dependencies from previous tasks are met.
  3. State what you will implement and how you will validate it.
  4. Implement.
  5. Run/describe validation steps.
  6. Mark the task as completed in the roadmap checklist.
```

---

## 3. Technology Stack

### 3.1 Firmware (Microcontroller)

| Aspect | Choice | Notes |
|--------|--------|-------|
| MCU | ESP32-C3 | RISC-V, BLE 5.0, low power |
| Dev board (MVP) | ESP32-C3-DevKitC-02 v1.1 | USB-CDC, RGB LED (WS2812 on GPIO8) |
| Framework | ESP-IDF v5.x | Official Espressif framework |
| Language | C (C11) | Some C++ where ESP-IDF APIs require it |
| RTOS | FreeRTOS (bundled with ESP-IDF) | |
| BLE stack | NimBLE | Lower RAM, better power than Bluedroid |
| BLE service | Nordic UART Service (NUS) | UUID: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E` |
| Sensor | MS5611 via I2C | Future: BMP390 |
| Build system | CMake (ESP-IDF native) | |
| Test framework | Ceedling (Unity + CMock) | Host-side unit tests |
| Config storage | NVS (Non-Volatile Storage) | |
| Formatting | `.clang-format` (Microsoft base, Allman braces) | Applied via `clang-format` CLI |
| Scripts | Bash scripts in `scripts/micro/` and `scripts/app/` | build, flash, test, monitor |

### 3.2 Mobile App (Android)

> **Decision: Flutter (Dart)** — confirmed by developer.

| Aspect | Choice | Notes |
|--------|--------|-------|
| Framework | Flutter | Cross-platform; Android only for MVP |
| Language | Dart | Typed, imperative, easy for C devs |
| BLE Plugin | `flutter_blue_plus` | Best maintained Flutter BLE library |
| State Mgmt | To be decided in Phase 0 | Riverpod recommended |
| Local Storage | `shared_preferences` | App settings persistence |
| Permissions | `permission_handler` | Android BLE/location permissions |

See [Appendix C](#appendix-c--mobile-app-technology-decision) for the comparison that led to this decision.

### 3.3 Shared / Tooling

| Tool | Purpose |
|------|---------|
| VS Code | IDE with GitHub Copilot |
| Git | Version control |
| GitHub | Repository hosting |
| GitHub Copilot | AI-assisted development (instructions, prompts, agents) |

---

## 4. Code Style Guide — Firmware

> **IMPORTANT FOR AI AGENTS**: When generating or modifying C/C++ firmware code, strictly follow these conventions. After completing any file edit, apply the project's `.clang-format` configuration.

### 4.1 Formatting Tool

The formatting style is defined in **`.clang-format`** at the firmware project root (`micro/.clang-format`).

**Key rules:**

| Setting | Value |
|---------|-------|
| `IndentWidth` | 4 (spaces, never tabs) |
| `ColumnLimit` | 120 |
| `PointerAlignment` | Right (`int *ptr`) |
| `BreakBeforeBraces` | Allman (custom) |
| `AllowShortIfStatementsOnASingleLine` | Never |
| `AllowShortFunctionsOnASingleLine` | None |
| `SortIncludes` | CaseSensitive |

**Brace style**: Allman — braces on new line after `if`, `for`, `while`, functions, structs, enums. `else` and `catch` on new line.

**Single-line guard clauses** are accepted:
```c
if (!self || !cfg)
    return RET_INV_PARAMS;
```

### 4.2 File Organization

```
component_name/
├── CMakeLists.txt            // ESP-IDF component registration
├── include/
│   └── component_name.h      // Public API (include guard, extern "C", Doxygen)
└── src/
    └── component_name.c      // Implementation (private types, static functions)
```

> For complex components, additional internal headers (`*_types.h`) and source files
> may be added under `src/`, but keep the public API in a single header under `include/`.

### 4.3 Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Files | `snake_case` | `sensor_ms5611.c`, `ble_nus_api.h` |
| Public functions | `module_action()` | `sensor_ms5611_init()`, `ble_nus_send()` |
| Static functions | `action_description()` | `parse_raw_data()`, `calculate_altitude()` |
| Callbacks | `module_cb_name()` or `_cb` suffix | `ble_gap_event_cb()` |
| Local variables | `snake_case` | `raw_pressure`, `temp_celsius` |
| Struct types | `module_name_t` | `sensor_ms5611_t`, `kalman_state_t` |
| Enum types | `module_name_e` | `led_color_e`, `device_state_e` |
| Macros/constants | `UPPER_SNAKE_CASE` | `SENSOR_READ_INTERVAL_MS`, `BLE_NUS_MAX_MTU` |

### 4.4 Type & Struct Patterns

```c
// Object struct — base first, config second
typedef struct sensor_ms5611_s
{
    // public/base fields
    const sensor_cfg_t *cfg;
    // private fields
    uint32_t raw_pressure;
    uint32_t raw_temperature;
    int32_t  compensated_pressure;
    int32_t  compensated_temperature;
    uint16_t calibration[6];
} sensor_ms5611_t;

// Configuration struct
typedef struct sensor_cfg_s
{
    uint8_t i2c_addr;
    i2c_port_t i2c_port;
    uint16_t read_interval_ms;
} sensor_cfg_t;
```

### 4.5 Function Patterns

**Initialization:**
```c
esp_err_t sensor_ms5611_init(sensor_ms5611_t *self, const sensor_cfg_t *cfg)
{
    if (!self || !cfg)
        return ESP_ERR_INVALID_ARG;

    self->cfg = cfg;
    // ... initialization logic ...
    return ESP_OK;
}
```

**Reader / runner (FreeRTOS task body helper):**
```c
esp_err_t sensor_ms5611_read(sensor_ms5611_t *self)
{
    if (!self)
        return ESP_ERR_INVALID_ARG;

    // ... read logic ...
    return ESP_OK;
}
```

**Early return for error handling:**
```c
if (!param)
    return ESP_ERR_INVALID_ARG;
```

### 4.6 Include Order

1. Own module header (`"module.h"`)
2. Internal project headers (`"other_internal.h"`)
3. ESP-IDF / component headers (`<esp_log.h>`, `<driver/i2c.h>`)
4. Standard library (`<stdint.h>`, `<string.h>`)

### 4.7 Header Guards & C++ Compatibility

```c
#ifndef SENSOR_MS5611_H
#define SENSOR_MS5611_H

#ifdef __cplusplus
extern "C" {
#endif

// declarations

#ifdef __cplusplus
}
#endif

#endif // SENSOR_MS5611_H
```

### 4.8 Documentation

- **Public functions in headers**: Doxygen-style `@brief`, `@param`, `@return`.
- **Function bodies**: Minimal comments. Prefer self-documenting code.
- **No file headers** unless the file contains significantly modified external code.

### 4.9 Mandatory Rules for AI Agents

1. ✅ Apply `.clang-format` after every C/H file edit.
2. ✅ All code, comments, documentation in **English**.
3. ✅ Minimize comments inside function bodies.
4. ✅ Follow naming conventions exactly.
5. ✅ Validate all pointer parameters at function entry.
6. ✅ Use `esp_err_t` return values for operations.
7. ✅ Prefer static allocation over dynamic.
8. ✅ Use `static` for all private (file-scope) functions.
9. ✅ Never expose internal structs or private functions in public headers.
10. ✅ Avoid forward declarations; reorder function definitions instead.

### 4.10 Prohibited Practices

- ❌ Tabs for indentation.
- ❌ camelCase for C variables or functions.
- ❌ Omitting parameter validation.
- ❌ Magic numbers without `#define` constants.
- ❌ Mutable globals without `static`.
- ❌ `printf` for logging (use `ESP_LOGI`, `ESP_LOGW`, `ESP_LOGE`).

### 4.11 Recomended Practices

- ✅ Use `!var` instead `var == NULL` for pointer checks.
- ✅ Use `!var` instead of `var == 0` for integer checks when zero is the invalid value.
- ✅ Use `DEFINE == var` instead of `var == DEFINE` for constant comparisons.
- ✅ Use `const` for pointer parameters that are not modified.
- ✅ Use `const` for configuration structs that are stored in the object struct.
- ✅ Use `typedef struct module_s module_t` pattern for opaque types.
- ✅ Use `static` functions for internal helpers, never expose them in headers.
- ✅ Use ternary operator for simple conditional when possible.
- ✅ Use early return with no curly braces for simple guard clauses (max 2 levels of nesting).
- ✅ Use following statement:

```c
if (!self || !cfg)
    return ESP_ERR_INVALID_ARG;
```
instead of
```c
if (!self || !cfg) 
{
    return ESP_ERR_INVALID_ARG;
}
```

---

## 5. GitHub Copilot Configuration

> This section defines the files that must be created in the workspace to configure GitHub Copilot's behavior. **Generate these files before starting any implementation work.**

### 5.1 Copilot Instructions — `.github/copilot-instructions.md`

This file provides global instructions that Copilot reads automatically for every interaction within the workspace.

**File to create**: `.github/copilot-instructions.md`

**Content to generate** (summary of key directives):

```markdown
# Copilot Instructions — esp-fly-in-peace

## Project Context
This is a battery-powered IoT variometer for paragliding.
- **micro/**: ESP32-C3 firmware (ESP-IDF, C, FreeRTOS, NimBLE, Ceedling tests)
- **app/**: Android companion app (Flutter with Dart, flutter_blue_plus for BLE)
- **docs/**: Roadmaps and documentation

## Behavior
- Act as a pragmatic Clean Code / Clean Architecture mentor.
- Be direct. Prioritize clarity and simplicity.
- Keep changes small and verifiable.
- Before coding: state acceptance criteria and validation method.
- Follow the roadmap (docs/roadmap.micro.md or docs/roadmap.app.md). Do not skip phases.
- Ask clarifying questions when ambiguous (propose 1-2 options).
- Reflect on embedded constraints (RAM, power, real-time) for firmware suggestions.

## Firmware Rules
- Language: C (C11). Comments in English.
- Follow the style guide in PRE-PROMPT.md §4.
- Apply .clang-format after every edit.
- Use esp_err_t for return values, ESP_LOGx for logging.
- Validate pointers at function entry.
- Prefer static allocation. Use static for private functions.
- Test business logic with Ceedling (Kalman filter, LK8EX1 formatting, config).

## Mobile App Rules
- Language: Dart (Flutter). Comments in English.
- Handle errors explicitly. No unhandled exceptions.
- Incremental: test each component before integrating.
- UI must be responsive (phones and tablets).

## Decision Protocol
When the developer faces a choice outside their expertise:
1. Present 2-3 options with pros/cons.
2. Mark one as Recommended.
3. Wait for confirmation.
```

### 5.2 Copilot Custom Agents

Define custom Copilot agents (chat participants) to scope AI behavior per project domain. These are configured as `.github/agents/*.agent.md`.

#### Agent: `@firmware`

**Purpose**: Firmware-specific assistance for ESP32-C3 development.

**File**: `.github/agents/firmware.agent.md`

```markdown
# Agent: Firmware

## Role
Expert embedded systems engineer specializing in ESP-IDF, FreeRTOS, NimBLE BLE, 
and low-power design for ESP32-C3.

## Context
- Working directory: micro/
- Framework: ESP-IDF v5.x with CMake build system
- RTOS: FreeRTOS (tasks, queues, semaphores, timers)
- BLE: NimBLE stack, Nordic UART Service (NUS)
- Sensor: MS5611 (I2C), extensible via HAL interface
- Tests: Ceedling (Unity + CMock) in micro/test/
- Style: See PRE-PROMPT.md §4. Apply .clang-format after edits.
- Power: Light-sleep between sensor reads. Optimize BLE connection intervals.

## Capabilities
- Create new ESP-IDF components following the project structure
- Implement sensor drivers with the HAL abstraction
- Configure NimBLE services and characteristics
- Write Ceedling unit tests
- Optimize power consumption
- Debug I2C, BLE, FreeRTOS issues
- Generate build/flash/monitor scripts

## Constraints
- Always use esp_err_t for return codes
- Always validate pointer parameters
- Use ESP_LOGx macros for logging (never printf)
- Follow naming: snake_case functions, UPPER_SNAKE_CASE macros
- Static allocation preferred
- No dynamic memory allocation in ISRs or time-critical paths
```

#### Agent: `@app`

**Purpose**: Mobile app development assistance.

**File**: `.github/agents/app.agent.md`

```markdown
# Agent: Mobile App

## Role
Senior Android developer assisting a firmware engineer who is learning mobile development.
Provide extra context, explain patterns, and suggest the simplest correct approach.

## Context
- Working directory: app/
- Platform: Android only
- Technology: Flutter with Dart
- BLE: Connect to ESP32-C3 via Nordic UART Service (NUS)
- Protocol: Parse LK8EX1 NMEA sentences from BLE NUS RX characteristic
- Features (MVP): BLE scan/connect, real-time vario display, device configuration

## Capabilities
- Scaffold new screens/widgets following project architecture
- Implement BLE scanning, connection, and NUS data handling
- Parse LK8EX1 sentences into structured data
- Build real-time data display with charts/gauges
- Implement device configuration UI with BLE write
- Handle Android BLE permissions and lifecycle
- Write unit and widget tests

## Interaction Style
- Explain WHY, not just HOW (the developer is learning mobile).
- When introducing a new pattern (e.g., state management), explain it briefly.
- Suggest the simplest approach that meets requirements.
- Provide complete, runnable code — don't leave TODOs for basic wiring.
```

#### Agent: `@docs`

**Purpose**: Documentation and roadmap management.

**File**: `.github/agents/docs.agent.md`

```markdown
# Agent: Documentation

## Role
Technical writer and project manager for the esp-fly-in-peace project.

## Context
- Working directory: docs/
- Documents: roadmap.micro.md, roadmap.app.md, architecture docs
- Format: Markdown with checklists for task tracking

## Capabilities
- Update roadmap checklists when tasks are completed
- Write architecture decision records (ADRs)
- Document APIs and protocols
- Generate sequence diagrams (Mermaid)
- Review and improve existing documentation

## Rules
- All documentation in English
- Use Mermaid for diagrams when possible
- Keep roadmap task descriptions actionable and specific
- Include acceptance criteria for every task
```

### 5.3 Copilot Prompt Files

Reusable prompt templates for common tasks. Stored in `.github/prompts/`.

#### Prompt: New Firmware Component

**File**: `.github/prompts/firmware-new-component.prompt.md`

```markdown
# New Firmware Component

Create a new ESP-IDF component for the esp-fly-in-peace firmware.

## Input
- Component name: {{COMPONENT_NAME}}
- Purpose: {{PURPOSE}}
- Dependencies: {{DEPENDENCIES}}

## Output Structure
Generate the following files:
- `micro/components/{{COMPONENT_NAME}}/CMakeLists.txt`
- `micro/components/{{COMPONENT_NAME}}/include/{{COMPONENT_NAME}}.h` (public API)
- `micro/components/{{COMPONENT_NAME}}/src/{{COMPONENT_NAME}}.c` (implementation)
- `micro/components/{{COMPONENT_NAME}}/src/{{COMPONENT_NAME}}_types.h` (private types)
- `micro/test/test_{{COMPONENT_NAME}}.c` (Ceedling test)

## Rules
- Follow the code style guide in PRE-PROMPT.md §4.
- Public header: include guard, C++ extern "C", Doxygen comments.
- Implementation: validate parameters, use esp_err_t, use ESP_LOGx.
- CMakeLists.txt: register component with REQUIRES for dependencies.
- Test: at least 3 test cases (init success, init null params, core functionality).
```

#### Prompt: New Sensor Driver

**File**: `.github/prompts/firmware-new-sensor.prompt.md`

```markdown
# New Sensor Driver

Create a sensor driver that implements the sensor HAL interface.

## Input
- Sensor name: {{SENSOR_NAME}} (e.g., ms5611, bmp390)
- Bus: {{BUS}} (e.g., I2C, SPI)
- Datasheet reference: {{DATASHEET_URL}}

## Requirements
- Implement the sensor_interface_t (defined in micro/components/sensor_hal/)
- Functions: init, read, deinit, get_config
- Include calibration/compensation per datasheet
- Unit tests for compensation math (use known test vectors from datasheet)
- Handle bus errors gracefully (retry once, then return error)

## Output
- `micro/components/sensor_{{SENSOR_NAME}}/` (full component)
- `micro/test/test_sensor_{{SENSOR_NAME}}.c` (Ceedling tests)
```

#### Prompt: BLE Service

**File**: `.github/prompts/firmware-ble-service.prompt.md`

```markdown
# BLE Service Implementation

Create or modify a BLE service using NimBLE.

## Input
- Service name: {{SERVICE_NAME}}
- Service UUID: {{UUID}}
- Characteristics: {{CHARACTERISTICS_LIST}}

## Requirements
- Register with NimBLE GATT server
- Handle read/write/notify operations
- Thread-safe access to shared data
- Connection/disconnection callbacks
- Log state changes with ESP_LOGx

## Context
- BLE stack: NimBLE (not Bluedroid)
- Primary service: NUS (6E400001-B5A3-F393-E0A9-E50E24DCCA9E)
  - TX characteristic (notify): 6E400003-...
  - RX characteristic (write): 6E400002-...
```

#### Prompt: App New Screen

**File**: `.github/prompts/app-new-screen.prompt.md`

```markdown
# New App Screen

Create a new screen/page for the Android companion app.

## Input
- Screen name: {{SCREEN_NAME}}
- Purpose: {{PURPOSE}}
- Navigation: {{HOW_TO_REACH}}

## Requirements
- Follow the app's architecture pattern (state management, navigation)
- Responsive layout (portrait phone + tablet)
- Handle loading, error, and empty states
- Include unit/widget tests
- Handle BLE disconnection gracefully if screen depends on BLE data
```

### 5.4 Copilot Skills (Per-Project Capabilities)

Skills define what the AI should be capable of doing well within each project context.

#### Firmware Skills

| Skill | Description |
|-------|-------------|
| `esp-idf-component` | Create, configure, and build ESP-IDF CMake components |
| `freertos-task` | Create FreeRTOS tasks with proper stack sizing, priorities, and inter-task communication (queues, events) |
| `nimble-ble` | Configure NimBLE, register GATT services, handle GAP events, manage connections |
| `i2c-driver` | Initialize I2C bus, communicate with sensors, handle NACK/timeout |
| `kalman-filter` | Implement and tune a 2-state Kalman filter for altitude and vario |
| `lk8ex1-format` | Format and validate LK8EX1 NMEA sentences with checksum |
| `nvs-config` | Read/write configuration to NVS, define config schema, handle defaults |
| `power-mgmt` | Configure light-sleep, tickless idle, peripheral power gating |
| `led-indicator` | Drive WS2812 RGB LED with timed patterns using RMT peripheral |
| `ceedling-test` | Write Ceedling test suites with Unity assertions and CMock stubs |
| `build-script` | Create bash scripts for build, flash, monitor, test workflows |

#### Mobile App Skills

| Skill | Description |
|-------|-------------|
| `ble-scanner` | Scan for BLE devices, filter by NUS service UUID, display RSSI |
| `ble-connection` | Connect/disconnect, handle reconnection, manage connection state |
| `nus-comm` | Send/receive data via NUS TX/RX characteristics |
| `lk8ex1-parser` | Parse LK8EX1 sentences from raw BLE data stream |
| `realtime-display` | Display altitude, vario, pressure data with real-time updates |
| `device-config-ui` | Build forms to read/write device configuration via BLE |
| `ble-permissions` | Handle Android BLE permissions (location, nearby devices, etc.) |
| `responsive-layout` | Build layouts that adapt to phones and tablets |

---

## 6. Workspace & Directory Structure

### 6.1 VS Code Workspace

**File**: `esp-fly-in-peace.code-workspace`

```json
{
  "folders": [
    { "path": "micro", "name": "Firmware (ESP32-C3)" },
    { "path": "app", "name": "Mobile App (Android)" },
    { "path": "docs", "name": "Documentation" }
  ],
  "settings": {
    "files.associations": {
      "*.h": "c",
      "*.c": "c"
    },
    "editor.formatOnSave": false,
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
  }
}
```

### 6.2 Directory Structure

```
esp-fly-in-peace/
├── esp-fly-in-peace.code-workspace            # VS Code workspace
├── .github/
│   ├── PRE-PROMPT.md                          # Master prompt (this file)
│   ├── copilot-instructions.md                # Global Copilot instructions
│   ├── agents/
│   │   ├── firmware.agent.md
│   │   ├── app.agent.md
│   │   └── docs.agent.md
│   └── prompts/
│       ├── firmware-new-component.prompt.md
│       ├── firmware-new-sensor.prompt.md
│       ├── firmware-ble-service.prompt.md
│       └── app-new-screen.prompt.md
│
├── docs/
│   ├── roadmap.micro.md                       # Firmware roadmap
│   ├── roadmap.app.md                         # Mobile app roadmap
│   └── architecture/                          # Architecture decision records
│       └── adr-001-ble-stack-nimble.md
│
├── micro/                                     # Firmware project root
│   ├── CMakeLists.txt                         # Top-level CMake
│   ├── sdkconfig.defaults                     # ESP-IDF default config
│   ├── partitions.csv                         # Partition table
│   ├── .clang-format                          # C/C++ formatting rules
│   ├── main/
│   │   ├── CMakeLists.txt
│   │   ├── main.c                             # Entry point (app_main)
│   │   └── Kconfig.projbuild                  # Project-level Kconfig
│   ├── components/
│   │   ├── sensor_hal/                        # Sensor abstraction layer
│   │   │   ├── include/sensor_hal.h
│   │   │   └── src/sensor_hal.c
│   │   ├── sensor_ms5611/                     # MS5611 driver
│   │   │   ├── include/sensor_ms5611.h
│   │   │   └── src/sensor_ms5611.c
│   │   ├── kalman_filter/                     # 2-state Kalman filter (altitude + vario)
│   │   │   ├── include/kalman_filter.h
│   │   │   └── src/kalman_filter.c
│   │   ├── lk8ex1/                            # LK8EX1 protocol formatter
│   │   │   ├── include/lk8ex1.h
│   │   │   └── src/lk8ex1.c
│   │   ├── ble_nus/                           # BLE Nordic UART Service
│   │   │   ├── include/ble_nus.h
│   │   │   └── src/ble_nus.c
│   │   ├── led_indicator/                     # RGB LED driver
│   │   │   ├── include/led_indicator.h
│   │   │   └── src/led_indicator.c
│   │   ├── config_manager/                    # NVS configuration
│   │   │   ├── include/config_manager.h
│   │   │   └── src/config_manager.c
│   │   └── power_manager/                     # Power management
│   │       ├── include/power_manager.h
│   │       └── src/power_manager.c
│   ├── test/                                  # Ceedling unit tests
│   │   ├── project.yml                        # Ceedling config
│   │   ├── test_kalman_filter.c
│   │   ├── test_lk8ex1.c
│   │   ├── test_sensor_ms5611.c
│   │   └── test_config_manager.c
├── scripts/
│   ├── micro/
│   │   ├── build.sh
│   │   ├── flash.sh
│   │   ├── monitor.sh
│   │   ├── test.sh
│   │   ├── all.sh                             # build + flash + monitor
│   │   └── env.sh
│   └── app/
│       ├── build_app_debug.sh
│       └── app_test_option.sh
│
└── app/                                       # Mobile app root
    └── ... (structure depends on chosen technology)
```

---

## 7. Roadmap Guidelines

### 7.1 Roadmap Format

Each roadmap file (`docs/roadmap.micro.md`, `docs/roadmap.app.md`) must follow this structure:

```markdown
# Roadmap — [Project Name]

## Summary Checklist

- [ ] Phase 1: [Phase Name]
  - [ ] Task 1.1: [Task Name]
  - [ ] Task 1.2: [Task Name]
- [ ] Phase 2: [Phase Name]
  - [ ] Task 2.1: [Task Name]
  ...

---

## Phase 1: [Phase Name]

**Objective**: [One-sentence description]
**Estimated Duration**: [X days/weeks]
**Dependencies**: [None | Phase N]

### Task 1.1: [Task Name]

**Description**: [Detailed description of what to implement]

**Acceptance Criteria**:
- [ ] Criterion 1
- [ ] Criterion 2
- [ ] Criterion 3

**Validation**:
- How to test/verify this task is complete

**Files to create/modify**:
- `path/to/file1`
- `path/to/file2`

**Notes**: [Any additional context, pitfalls, or references]

---
```

### 7.2 Roadmap Workflow

```
For each task:
  1. Read the full task description and acceptance criteria.
  2. Check that all dependencies from previous tasks are complete.
  3. Implement the task.
  4. Verify all acceptance criteria are met.
  5. Mark the task checkbox [x] in the roadmap.
  6. Commit with message: "feat(component): short description [task-id]"
```

### 7.3 Firmware Roadmap — Phase Outline

> The complete roadmap will be generated in `docs/roadmap.micro.md`. Below is the phase structure.

| Phase | Name | Description | Est. Duration |
|-------|------|-------------|---------------|
| 0 | Project Bootstrap | ESP-IDF project setup, build system, scripts, .clang-format, Ceedling config | 2–3 days |
| 1 | Hardware Abstraction | I2C bus driver, sensor HAL interface definition | 2–3 days |
| 2 | MS5611 Sensor Driver | I2C communication, calibration, compensation, unit tests | 3–4 days |
| 3 | Kalman Filter | 2-state Kalman filter for altitude and vario, unit tests with synthetic data | 2–3 days |
| 4 | LK8EX1 Protocol | Sentence formatter with checksum, unit tests | 1–2 days |
| 5 | BLE NUS Service | NimBLE init, GAP advertising, NUS GATT service, TX notifications | 3–4 days |
| 6 | Data Pipeline | FreeRTOS tasks: sensor→filter→format→BLE, queues, timing (10 Hz read, 4 Hz send) | 3–4 days |
| 7 | LED Indicator | WS2812 driver via RMT, state-based blink patterns (red/green/blue) | 1–2 days |
| 8 | NVS Configuration | Config schema, default values, read/write via NVS, BLE config service | 2–3 days |
| 9 | Power Optimization | Light-sleep between reads, BLE connection interval tuning, tickless idle | 2–3 days |
| 10 | Integration & Validation | End-to-end testing with XCTrack, power measurement, stress testing | 3–5 days |
| 11 | Documentation & Cleanup | README, API docs, architecture diagrams, code review pass | 2–3 days |
| — | *Future* | WiFi integration, OTA updates, BMP390 driver, ESP32-C3-MINI target | — |

### 7.4 Mobile App Roadmap — Phase Outline

> The complete roadmap will be generated in `docs/roadmap.app.md`. Below is the phase structure.

| Phase | Name | Description | Est. Duration |
|-------|------|-------------|---------------|
| 0 | Project Bootstrap | Project scaffolding, dependencies, architecture setup, BLE plugin | 2–3 days |
| 1 | BLE Scanner | Scan for devices, filter by NUS UUID, display name + RSSI | 2–3 days |
| 2 | BLE Connection | Connect/disconnect, connection state management, auto-reconnect | 2–3 days |
| 3 | NUS Communication | Send/receive data via NUS, raw data display for debugging | 2–3 days |
| 4 | LK8EX1 Parser | Parse LK8EX1 sentences, extract pressure/altitude/vario values | 1–2 days |
| 5 | Real-Time Display | Main flight screen: altitude (m), vario (m/s), pressure (hPa), connection status | 3–4 days |
| 6 | Device Configuration | Read/send config parameters via BLE Config Service (device name, sample rate, etc.) | 3–4 days |
| 7 | Settings & Persistence | App settings (units, display preferences), local storage | 2–3 days |
| 8 | Polish & Testing | Error handling review, UI polish, unit tests, integration tests | 3–5 days |
| 9 | Documentation | User guide, build instructions, screenshots | 1–2 days |
| — | *Future* | Historical data logging, flight playback, WiFi config, data export | — |

---

## 8. Development Workflow

### 8.1 Git Branching

```
main              — stable, tested code
├── develop       — integration branch
│   ├── feat/micro/phase-X-task-name
│   └── feat/app/phase-X-task-name
```

### 8.2 Commit Convention

```
type(scope): short description

Types: feat, fix, refactor, test, docs, chore
Scopes: micro, app, docs, config
```

Examples:
- `feat(micro): add MS5611 I2C driver with calibration`
- `test(micro): add Ceedling tests for Kalman filter`
- `feat(app): implement BLE scanner screen`
- `docs: update roadmap.micro.md — Phase 2 complete`

### 8.3 Build & Flash Scripts (Firmware)

Firmware scripts in `scripts/micro/` (repository root, execute from `.` as `./scripts/micro/<name>.sh`):

| Script | Purpose | Command |
|--------|---------|---------|
| `build.sh` | Build firmware | `idf.py build` |
| `flash.sh` | Flash to ESP32-C3 | `idf.py -p /dev/ttyUSB0 flash` |
| `monitor.sh` | Serial monitor | `idf.py -p /dev/ttyUSB0 monitor` |
| `test.sh` | Run Ceedling unit tests | `cd test && ceedling test:all` |
| `all.sh` | Build + flash + monitor | Chains all above |
| `env.sh` | Activate ESP-IDF environment | `source ./scripts/micro/env.sh` |

All scripts should:
- Accept a `-p PORT` parameter for serial port override.
- Print colored status messages.
- Exit with non-zero code on failure.

### 8.4 First Actions — Bootstrap Sequence

When starting the project for the first time, execute these steps in order:

```
Step 1: Generate workspace file         (esp-fly-in-peace.code-workspace)
Step 2: Generate Copilot config files   (.github/copilot-instructions.md, agents, prompts)
Step 3: Generate firmware roadmap       (docs/roadmap.micro.md)
Step 4: Generate mobile app roadmap     (docs/roadmap.app.md)
Step 5: Confirm technology choice       (Mobile app: Flutter vs Kotlin vs other)
Step 6: Begin Phase 0 of firmware       (Project Bootstrap)
Step 7: Begin Phase 0 of mobile app     (Project Bootstrap)
```

> **IMPORTANT**: Do not start Phase 1+ until Phase 0 is complete and validated for both projects.

---

## Appendix A — LK8EX1 Protocol Reference

### Sentence Format

```
$LK8EX1,pressure,altitude,vario,temperature,battery,*checksum\r\n
```

| Field | Type | Unit | Description |
|-------|------|------|-------------|
| `pressure` | int | Pa | Raw pressure in Pascals (e.g., 101325) |
| `altitude` | int | m | GPS altitude (99999 if not available) |
| `vario` | int | cm/s | Vertical speed in cm/s (e.g., 50 = 0.50 m/s) |
| `temperature` | int | °C × 10 | Temperature (e.g., 235 = 23.5°C) |
| `battery` | int | mV or % | Battery voltage or percentage (999 if not available) |

### Checksum

XOR of all characters between `$` and `*` (exclusive), formatted as 2-digit uppercase hex.

### Example

```
$LK8EX1,101325,99999,50,235,999,*checksum\r\n
```

### Notes for Implementation

- Send at 4 Hz (250 ms interval).
- XCTrack reads from BLE NUS TX characteristic (notify).
- Max sentence length: ~60 bytes — fits in single BLE NUS notification (default MTU 23, negotiate higher if possible).
- If MTU allows (≥ 64), send full sentence in one notification. Otherwise, fragment and include `\r\n` terminator for reassembly.

---

## Appendix B — BLE NUS Reference

### Nordic UART Service (NUS)

| Element | UUID |
|---------|------|
| Service | `6E400001-B5A3-F393-E0A9-E50E24DCCA9E` |
| RX Characteristic (write) | `6E400002-B5A3-F393-E0A9-E50E24DCCA9E` |
| TX Characteristic (notify) | `6E400003-B5A3-F393-E0A9-E50E24DCCA9E` |

### Data Flow

```
[ESP32-C3]                          [XCTrack / Mobile App]
    │                                       │
    │  ◄── BLE Connect ──────────────────── │
    │  ◄── Subscribe to NUS TX (notify) ──  │
    │                                       │
    │  ── TX Notify: "$LK8EX1,..." ──────►  │  (4 Hz)
    │  ── TX Notify: "$LK8EX1,..." ──────►  │
    │                                       │
    │  ◄── [App] Read Config char ────────  │  (Config Service GATT)
    │  ── Config JSON response ──────────►  │
    │  ◄── [App] Write Config char ───────  │
    │  ── Write ack ─────────────────────►  │
```

### BLE Advertising
- Device name ALWAYS is `FlyInPeace` (configurable via NVS).
- Device name: configurable via NVS (default: `"FlyInPeace"`)
- Advertise NUS service UUID in advertisement data.
- Advertising interval: 100–200 ms (when not connected), optimizable for power.
- Connection interval: negotiate 15–30 ms for reliable 4 Hz data at low power.

### Config Service (GATT — Hybrid Architecture)

Device configuration uses a **separate GATT service** (not NUS). This cleanly separates
the streaming data path (NUS) from the configuration path (Config Service).

See `docs/architecture/ble_protocol.md` for the full Config Service specification
(service UUID, characteristics, JSON format).

---

## Appendix C — Mobile App Technology Decision

> **This section helps the developer choose the mobile app technology.** The AI agent should present this comparison and wait for a decision before generating the app roadmap.

### Option 1: Flutter (Dart) — ★ RECOMMENDED

**Description**: Google's cross-platform UI toolkit. Uses Dart language. Compiles to native ARM code. Single codebase could target Android and iOS, but for this MVP only Android is needed.

**Pros**:
1. **Hot-reload** — see UI changes instantly without rebuilding. Huge productivity boost for a non-mobile-expert.
2. **`flutter_blue_plus`** — well-maintained BLE plugin with good NUS support and active community.
3. **Dart is simple to learn** — statically typed, imperative, C-like syntax. A firmware developer will feel comfortable quickly.
4. Large community, extensive documentation, many tutorials for BLE + real-time data.
5. Copilot generates excellent Flutter/Dart code with high accuracy.
6. Could extend to iOS in the future without rewriting.

**Cons**:
1. **Not native Android** — adds a layer of abstraction; BLE edge cases may require platform channels.
2. **APK size** — larger than native Kotlin (~15-20 MB vs ~5-8 MB) due to Flutter engine.
3. **Platform-specific features** may require writing Kotlin glue code (e.g., foreground service for BLE).

**BLE library**: `flutter_blue_plus` (pub.dev, 2k+ likes, active maintenance).

---

### Option 2: Kotlin + Jetpack Compose (Native Android)

**Description**: Google's official modern Android development stack. Kotlin language with Compose declarative UI. Direct access to all Android APIs including BLE.

**Pros**:
1. **Best BLE support** — direct access to Android's `BluetoothGatt` API; no plugin abstraction layer.
2. **Official Google stack** — best documentation, first-class support for new Android features.
3. **Smallest APK size** — lean native binaries.

**Cons**:
1. **Steeper learning curve** — Kotlin has more language features to learn; Compose paradigm is different from imperative UI.
2. **Android only** — cannot extend to iOS without a complete rewrite.
3. **Slower iteration** — no hot-reload equivalent (Live Edit is limited).

**BLE library**: Android `BluetoothGatt` API (native) or `Kable` library for coroutine-based BLE.

---

### Option 3: React Native (TypeScript)

**Description**: Meta's cross-platform framework using JavaScript/TypeScript. Uses native components via bridge architecture.

**Pros**:
1. **JavaScript/TypeScript** — most widely known language; huge ecosystem.
2. **Fast refresh** — similar to hot-reload.
3. **Can extend to iOS**.

**Cons**:
1. **BLE support is mediocre** — `react-native-ble-plx` has known issues with stability and maintenance.
2. **Bridge overhead** — JavaScript↔Native bridge adds latency for real-time BLE data.
3. **More complex setup** — Metro bundler, native modules, build configuration is more fragile.

**BLE library**: `react-native-ble-plx` (less maintained than Flutter equivalent).

---

### Recommendation

**Flutter** is the recommended choice because:
- Best balance of **development speed** (hot-reload) and **BLE capability** (flutter_blue_plus).
- **Lowest learning curve** for a C/embedded developer.
- **Best AI code generation** support (Copilot produces high-quality Flutter code).
- **Future-proof**: can add iOS target without rewrite.

> **Developer action**: Confirm "Flutter" or choose an alternative. Once confirmed, the app roadmap and Copilot agents will be configured for the chosen technology.

---

## Final Notes

1. **This document (`PRE-PROMPT.md`) is the single source of truth** for the project. Keep it updated as decisions are made.
2. **Do not generate implementation code** until:
   - The mobile app technology is confirmed.
   - Both roadmaps (`roadmap.micro.md`, `roadmap.app.md`) are generated and reviewed.
   - Phase 0 (Bootstrap) is complete for both projects.
3. **When in doubt, ask the developer.** Provide options with pros/cons. Mark one as recommended.
4. **Track progress** by checking off tasks in the roadmap files.
5. **Commit frequently** with descriptive messages following the convention in §8.2.
