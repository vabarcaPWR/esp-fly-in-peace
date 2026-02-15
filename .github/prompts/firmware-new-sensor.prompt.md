# New Sensor Driver

Create a sensor driver that implements the sensor HAL interface for the esp-fly-in-peace variometer.

## Input

- Sensor name: {{SENSOR_NAME}} (e.g., `ms5611`, `bmp390`)
- Bus: {{BUS}} (e.g., `I2C`, `SPI`)
- I2C address: {{I2C_ADDR}} (e.g., `0x77`)
- Datasheet reference: {{DATASHEET_URL}}

## Output Structure

```
micro/components/sensor_{{SENSOR_NAME}}/
├── CMakeLists.txt
├── include/
│   └── sensor_{{SENSOR_NAME}}.h       # Public API
└── src/
    ├── sensor_{{SENSOR_NAME}}.c       # Implementation
    └── sensor_{{SENSOR_NAME}}_types.h  # Private types (calibration data, raw readings)
```

Test: `micro/test/test_sensor_{{SENSOR_NAME}}.c`

## Requirements

1. **Implement the `sensor_hal_interface_t`** defined in `micro/components/sensor_hal/include/sensor_hal.h`:
   - `init(self, cfg)` → `esp_err_t`
   - `read(self)` → `esp_err_t`
   - `deinit(self)` → `void`
   - `get_pressure(self)` → `int32_t` (Pa)
   - `get_temperature(self)` → `int32_t` (°C × 100)

2. **Calibration**: Read factory calibration data from sensor PROM/registers during `init`.

3. **Compensation**: Apply temperature and pressure compensation per datasheet formulas.

4. **Error handling**:
   - Validate pointers at entry
   - Retry I2C transaction once on failure, then return `ESP_ERR_TIMEOUT`
   - Log errors with sensor name and operation context

5. **Unit tests** (`test_sensor_{{SENSOR_NAME}}.c`):
   - Test compensation math with known test vectors from datasheet
   - Test init with null parameters
   - Test init with I2C failure (CMock)
   - Test read with valid calibration data

6. **Apply `.clang-format`** after generating all files.

## Notes

- The sensor HAL allows swapping sensors without changing the data pipeline.
- Sensor drivers should NOT create FreeRTOS tasks — the pipeline task calls `read()`.
- Keep the driver stateless except for calibration data and last reading.
