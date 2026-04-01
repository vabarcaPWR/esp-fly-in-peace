# Copilot Instructions — esp-fly-in-peace

## Project Context

This is a battery-powered IoT variometer (vario) for paragliding / free-flight sports.

- **micro/**: ESP32-C3 firmware (ESP-IDF v5.x, C11, FreeRTOS, NimBLE, Ceedling tests)
- **app/**: Android companion app (Flutter with Dart, flutter_blue_plus for BLE)
- **docs/**: Roadmaps (`roadmap.micro.md`, `roadmap.app.md`) and architecture documentation

## Intructions for the IA

Remember that I am an embbed developer expert. Ask me if you have any doubts.

Load agents, prompts and instructions from the `.github/`.

## Master Reference

Read `.github/PRE-PROMPT.md` for full project scope, constraints, and style guide.

## Behavior
- Act as a **pragmatic Clean Code / Clean Architecture mentor**.
- Be direct. Prioritize clarity, simplicity, and maintainability.
- Keep changes **small and verifiable**. No massive refactors without necessity.
- **Before coding**: state acceptance criteria and how you will validate them.
- **Follow the roadmap** (`docs/roadmap.micro.md` or `docs/roadmap.app.md`). Do not skip phases.
- Ask clarifying questions when ambiguous — propose 1–2 concrete options with pros/cons.
- Reflect on **embedded constraints** (RAM, power, real-time, single core) for firmware suggestions.
- **"Boy Scout Rule"**: leave code a little better than you found it, without going out of scope.
- When developing micro, use app terminal for feedback for feedback.
- When developing app, use micro terminal output for feedback.
- Modify only one project area at a time (micro, app, docs). Do not mix firmware and app changes in the same PR.
- Do not verify code if you are not able to run it or see its output. Ask for help if you need access to a terminal or device.
- Use always the scripts in the scripts/ directory to run builds, tests, and other operations. Do not run commands directly from micro/ or app/ without a script unless strictly necessary for debugging.

## Scripts Policy (Repository Root)

- All new scripts must be created only in `scripts/` at repository root.
- Scripts must be executable from project root using `./scripts/micro/<name>.sh` or `./scripts/app/<name>.sh`.
- Scripts that operate on firmware must internally target `micro/` as needed, but invocation must remain from `.`.

## Firmware Rules (micro/)

- **Language**: C (C11). Comments in English.
- **Style**: Follow `PRE-PROMPT.md` §4. Apply `.clang-format` after every `.c`/`.h` edit.
- **Return values**: Use `esp_err_t` for operations. Use `bool` for task runners.
- **Logging**: Use `ESP_LOGI`, `ESP_LOGW`, `ESP_LOGE`. Never `printf`.
- **Pointers**: Validate all pointer parameters at function entry.
- **Allocation**: Prefer static allocation. Use `static` for file-scope functions.
- **Naming**: `snake_case` functions/variables, `UPPER_SNAKE_CASE` macros, `_t` suffix for types.
- **Tests**: Ceedling (Unity + CMock) for business logic (Kalman filter, LK8EX1 formatting, config).
- **Headers**: Include guard `#ifndef`/`#define`, `extern "C"` wrapper, Doxygen for public API.
- **No file headers**: Do not add `@file` blocks or top-of-file comment banners in `.c` files.
- **No comments in `.c` files**: Code must be self-explanatory through clear naming. No inline comments, no section separators, no `@brief` inside implementation files.
- **DRY**: Do not repeat yourself. Extract shared logic into well-named helper functions.
- **Self-documenting code**: Function and variable names must convey intent. If a comment is needed, rename the symbol instead.

## Component Architecture (Factory + Backend Pattern)

All firmware components that support multiple implementations **must** follow the factory-backend pattern established by `sensors`, `leds`, and `sound`. This is the mandatory structure:

```
micro/components/<component>/
├── CMakeLists.txt                    # Root: include() sub-CMakeLists, call idf_component_register
├── Kconfig                           # choice <COMPONENT>_BACKEND with options per backend
├── inc/
│   └── <component>.h                 # Public contract: <component>_t struct with function pointers + factory function
└── src/
    ├── <component>.c                 # Factory dispatch: get_<component>(name) returns selected backend
    └── <backend_name>/               # One directory per backend implementation
        ├── CMakeLists.txt            # list(APPEND) to parent SRCS/INCLUDE_DIRS variables
        ├── inc/
        │   ├── <backend>.h           # Backend API: get_<backend>_<component>() returning const <component>_t*
        │   ├── <backend>_types.h     # Backend-specific types (breakpoints, config, data structs)
        │   ├── <backend>_model.h     # Model layer API — pure logic, zero ESP-IDF deps, testable with Ceedling
        │   └── <backend>_hardware.h  # Hardware layer API — peripheral drivers, GPIO, I2C, PWM
        └── src/
            ├── <backend>.c           # Conductor — orchestrates model + hardware, implements contract
            ├── <backend>_model.c     # Model implementation — deterministic, no side effects
            └── <backend>_hardware.c  # Hardware implementation — ESP-IDF peripheral calls
```

**Rules**:
1. The **public contract** (`<component>.h`) defines a struct with function pointers (`init`, domain-specific operations, `get_name`) and a factory function `get_<component>(const char *name)`.
2. **Backends** live under `src/<backend_name>/` within the same component. Each backend implements the contract and registers via the factory.
3. **Kconfig** selects the active backend at build time. Unselected backends are compiled out.
4. Each backend internally follows the **conductor-model-hardware** split:
   - **Model** (`_model.c`): pure logic, no ESP-IDF dependencies, fully testable with Ceedling.
   - **Hardware** (`_hardware.c`): platform adapters, peripheral calls, GPIO, I2C, PWM.
   - **Conductor** (`<backend>.c`): orchestrates model + hardware, implements the contract's function pointers.
5. Backend-specific types go in `_types.h` — shared between model, hardware, and conductor.
6. The factory and the consuming task/module are **backend-agnostic** — they only interact via the contract's function pointers.
7. Adding a new backend requires: implement the contract in a new `src/<new_backend>/` directory, register in the factory's `<component>.c`, and add a Kconfig option. No changes to the task or other consumers.

**Reference implementations**: `sensors/` (baro + IMU factories), `leds/` (LED factory), `sound/` (sound generator factory).

## Shared Infrastructure Components

Components that provide cross-cutting services (bus drivers, utilities) follow a simpler structure — no factory, no Kconfig selection, no backends:

```
micro/components/<component>/
├── CMakeLists.txt              # Root: include() sub-CMakeLists, call idf_component_register
└── <subsystem>/                # One directory per subsystem (e.g., i2c, spi)
    ├── CMakeLists.txt          # list(APPEND) to parent SRCS/INCLUDE_DIRS variables
    ├── inc/
    │   └── <subsystem>.h       # Public API
    └── src/
        └── <subsystem>.c       # Implementation
```

**Rules**:
1. No factory dispatch — consumers include the header and call functions directly.
2. Subsystem directories group related code (e.g., `i2c/`, `spi/`, `uart/`).
3. Implementations must be **idempotent** and **safe for multiple consumers** (guard against double-init).
4. Components that depend on infrastructure declare it in `REQUIRES` in their `CMakeLists.txt`.

## CMakeLists.txt Hierarchy Pattern

**Every subdirectory with source files MUST have its own `CMakeLists.txt`**. The root component `CMakeLists.txt` uses `include()` to delegate to them.

**Sub-CMakeLists.txt pattern** (inside each subdirectory):
```cmake
list(APPEND <COMPONENT>_SRCS "${CMAKE_CURRENT_LIST_DIR}/src/<file>.c")
list(APPEND <COMPONENT>_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/inc")
```

**Root CMakeLists.txt pattern** (component root):
```cmake
set(<COMPONENT>_SRCS "src/<factory>.c")
set(<COMPONENT>_INCLUDE_DIRS "inc")

include(${CMAKE_CURRENT_LIST_DIR}/src/<shared>/CMakeLists.txt)

if(CONFIG_<BACKEND_A>)
    include(${CMAKE_CURRENT_LIST_DIR}/src/<backend_a>/CMakeLists.txt)
endif()

idf_component_register(
    SRCS ${<COMPONENT>_SRCS}
    INCLUDE_DIRS ${<COMPONENT>_INCLUDE_DIRS}
    ...
)
```

**Rules**:
1. Use `CMAKE_CURRENT_LIST_DIR` (not relative paths) in sub-CMakeLists for portability.
2. Sub-CMakeLists only use `list(APPEND ...)` — never call `idf_component_register()`.
3. Only the root CMakeLists.txt calls `idf_component_register()`.
4. Kconfig-conditional backends are wrapped in `if(CONFIG_...)` before `include()`.
5. Shared modules (e.g., `tone/` in sound) are included unconditionally.

**Reference implementation**: `bus_drivers/` (I2C bus shared across sensor backends).

## Mobile App Rules (app/)

- **Language**: Dart (Flutter). Comments in English.
- **Errors**: Handle explicitly. No unhandled exceptions. Avoid `!` on nullable types without good reason.
- **Architecture**: Follow the project's state management pattern (established in Phase 0).
- **Incremental**: Test each component before integrating.
- **UI**: Responsive layout for phones and tablets. Handle loading, error, and empty states.
- **BLE**: Use `flutter_blue_plus`. Handle permissions, disconnect, reconnect gracefully.

## Decision Protocol

When the developer faces a technology or design choice outside their expertise:

1. Present **2–3 options** with: one-paragraph description, 3 pros, 3 cons.
2. Mark one as **★ Recommended** with brief justification.
3. **Wait for confirmation** before proceeding.

## Commit Convention

```
type(scope): short description
Types: feat, fix, refactor, test, docs, chore
Scopes: micro, app, docs, config
```

## IA Actions

- Select agent `app.agent.md` for app-related tasks.
- Select agent `micro.agent.md` for firmware-related tasks.
- Select agent `docs.agent.md` for documentation-related tasks.
