set(SRCS
    "src/sensor_ms5611_model.c"
    "src/sensor_ms5611_hardware.c"
    "src/sensor_ms5611_conductor.c"
)

idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "inc"
    PRIV_INCLUDE_DIRS "src"
    REQUIRES "esp_timer" "esp_driver_i2c"
)
