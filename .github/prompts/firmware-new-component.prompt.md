# New Firmware Component

Create a new ESP-IDF component for the esp-fly-in-peace firmware.

## Input

- Component name: {{COMPONENT_NAME}}
- Purpose: {{PURPOSE}}
- Dependencies: {{DEPENDENCIES}}
- Has multiple backends: {{HAS_BACKENDS}} (yes/no)
- First backend name: {{BACKEND_NAME}} (if HAS_BACKENDS=yes)

## Output Structure

### Simple component (no backends)

```
micro/components/{{COMPONENT_NAME}}/
├── CMakeLists.txt
├── inc/
│   └── {{COMPONENT_NAME}}.h        # Public API header
└── src/
    ├── {{COMPONENT_NAME}}.c        # Implementation
    └── {{COMPONENT_NAME}}_types.h   # Private type definitions
```

### Factory component (with backends — mandatory for components with multiple implementations)

Follow the factory-backend pattern defined in `copilot-instructions.md` §Component Architecture:

```
micro/components/{{COMPONENT_NAME}}/
├── CMakeLists.txt                           # Conditional backend compilation via Kconfig
├── Kconfig                                  # choice with backend options
├── inc/
│   └── {{COMPONENT_NAME}}.h                # Public contract: {{COMPONENT_NAME}}_t + factory function
└── src/
    ├── {{COMPONENT_NAME}}.c                # Factory dispatch
    └── {{BACKEND_NAME}}/
        ├── inc/
        │   ├── {{BACKEND_NAME}}.h           # Backend API: get_{{BACKEND_NAME}}_{{COMPONENT_NAME}}()
        │   ├── {{BACKEND_NAME}}_types.h     # Backend-specific types
        │   ├── {{BACKEND_NAME}}_model.h     # Model layer API (pure logic, no ESP-IDF deps)
        │   └── {{BACKEND_NAME}}_hardware.h  # Hardware layer API
        └── src/
            ├── {{BACKEND_NAME}}.c           # Conductor (orchestrates model + hardware)
            ├── {{BACKEND_NAME}}_model.c     # Model implementation (testable with Ceedling)
            └── {{BACKEND_NAME}}_hardware.c  # Hardware implementation
```

Test files:
- `micro/test/test/test_{{COMPONENT_NAME}}.c` (factory tests)
- `micro/test/test/test_{{BACKEND_NAME}}_model.c` (model unit tests)

## Rules

1. **Public header** (`inc/{{COMPONENT_NAME}}.h`):
   - Include guard: `#ifndef {{COMPONENT_NAME_UPPER}}_H`
   - `extern "C"` wrapper for C++ compatibility
   - Doxygen comments for all public functions
   - For factory components: define `{{COMPONENT_NAME}}_t` struct with function pointers
   - Factory function: `const {{COMPONENT_NAME}}_t *get_{{COMPONENT_NAME}}(const char *name)`

2. **Implementation** (`src/{{COMPONENT_NAME}}.c`):
   - Validate all pointer parameters at function entry
   - Use `esp_err_t` return values for operations
   - Use `ESP_LOGx` macros for logging (define `TAG` at file top)
   - `static` for all private functions
   - No forward declarations — reorder definitions instead
   - Prefer static allocation

3. **Backend conductor-model-hardware split** (factory components only):
   - **Model** (`_model.c`): pure logic, zero ESP-IDF deps, testable with Ceedling
   - **Hardware** (`_hardware.c`): peripheral drivers, GPIO, I2C, PWM
   - **Conductor** (`<backend>.c`): orchestrates model + hardware, implements contract

4. **CMakeLists.txt**:
   ```cmake
   idf_component_register(
       SRCS "src/{{COMPONENT_NAME}}.c"
       INCLUDE_DIRS "inc"
       REQUIRES {{DEPENDENCIES}}
   )
   ```

5. **Test** (`test_{{COMPONENT_NAME}}.c`):
   - At least 3 test cases: init success, init with null params, core functionality
   - Use Unity assertions
   - Use CMock for mocking dependencies

6. **Apply `.clang-format`** after generating all files.

## Reference

See `copilot-instructions.md` §Component Architecture for the full factory-backend pattern.
Reference implementations: `sensors/`, `leds/`, `sound/`.
