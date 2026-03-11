# New Sensor Driver

Create a sensor driver that plugs into the sensor factory in `micro/components/sensors` for the esp-fly-in-peace variometer.

## Input

- Sensor name: {{SENSOR_NAME}} (e.g., `ms5611`, `bmp390`)
- Bus: {{BUS}} (e.g., `I2C`, `SPI`)
- I2C address: {{I2C_ADDR}} (e.g., `0x77`)
- Datasheet reference: {{DATASHEET_URL}}

## Output Structure

```
micro/components/sensors/
├── CMakeLists.txt
├── sensor.h
└── src/
    ├── sensor.c
    └── {{SENSOR_NAME}}/
        ├── CMakeLists.txt
        ├── inc/
        │   └── {{SENSOR_NAME}}.h
        └── src/
            ├── {{SENSOR_NAME}}_conductor.c
            ├── {{SENSOR_NAME}}_model.c
            ├── {{SENSOR_NAME}}_model.h
            ├── {{SENSOR_NAME}}_hardware.c
            └── {{SENSOR_NAME}}_hardware.h
```

Test: `micro/test/test_sensor_{{SENSOR_NAME}}.c`

## IA instructions

1. Do not modify `micro/components/sensors/sensor.h` — implement the API defined there.
2. Register the new driver in `micro/components/sensors/src/sensor.c` through `get_sensor(const char *sensor_name)`.
3. Keep the implementation inside `micro/components/sensors/src/{{SENSOR_NAME}}/`.

## Requirements

1. **Implement the `sensor_t` API** defined in `micro/components/sensors/sensor.h`:

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

6. **Apply `.clang-format`** after generating all files by running `pe-code-format <folder>` from `micro` folder.
7. **CMake integration**:
   - Update `micro/components/sensors/CMakeLists.txt` to compile the new source files.
   - Keep public headers in `inc/` and implementation in `src/` inside the sensor folder.

## Notes

- The factory in `micro/components/sensors/src/sensor.c` allows swapping sensors without changing the data pipeline.
- Sensor drivers should NOT create FreeRTOS tasks — the pipeline task calls `read()`.
- Keep the driver stateless except for calibration data and last reading.
