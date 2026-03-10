# New Firmware Component

Create a new ESP-IDF component for the esp-fly-in-peace firmware.

## Input

- Component name: {{COMPONENT_NAME}}
- Purpose: {{PURPOSE}}
- Dependencies: {{DEPENDENCIES}}

## Output Structure

Generate the following files:

```
micro/components/{{COMPONENT_NAME}}/
├── CMakeLists.txt
├── inc/
│   └── {{COMPONENT_NAME}}.h        # Public API header
└── src/
    ├── {{COMPONENT_NAME}}.c        # Implementation
    └── {{COMPONENT_NAME}}_types.h   # Private type definitions
```

And the test file:
- `micro/test/test_{{COMPONENT_NAME}}.c`

## Rules

1. **Public header** (`inc/{{COMPONENT_NAME}}.h`):
   - Include guard: `#ifndef {{COMPONENT_NAME_UPPER}}_H`
   - `extern "C"` wrapper for C++ compatibility
   - Doxygen comments for all public functions
   - Only expose opaque types or minimal public structs

2. **Implementation** (`src/{{COMPONENT_NAME}}.c`):
   - Validate all pointer parameters at function entry
   - Use `esp_err_t` return values for operations
   - Use `ESP_LOGx` macros for logging (define `TAG` at file top)
   - `static` for all private functions
   - No forward declarations — reorder definitions instead
   - Prefer static allocation

3. **CMakeLists.txt**:
   ```cmake
   idf_component_register(
       SRCS "src/{{COMPONENT_NAME}}.c"
       INCLUDE_DIRS "inc"
       REQUIRES {{DEPENDENCIES}}
   )
   ```

4. **Test** (`test_{{COMPONENT_NAME}}.c`):
   - At least 3 test cases: init success, init with null params, core functionality
   - Use Unity assertions
   - Use CMock for mocking dependencies

5. **Apply `.clang-format`** after generating all files.

## Example

For `COMPONENT_NAME=led_indicator`, `PURPOSE=Drive WS2812 RGB LED for status indication`,
`DEPENDENCIES=driver`:

- `micro/components/led_indicator/inc/led_indicator.h`
- `micro/components/led_indicator/src/led_indicator.c`
- `micro/components/led_indicator/src/led_indicator_types.h`
- `micro/components/led_indicator/CMakeLists.txt`
- `micro/test/test_led_indicator.c`
