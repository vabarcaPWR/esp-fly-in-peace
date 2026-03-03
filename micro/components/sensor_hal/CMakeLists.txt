set(SRCS "src/sensor_hal.c")
set(REQUIRES "esp_timer" "esp_driver_i2c")

idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "include"
    REQUIRES ${REQUIRES}
)
