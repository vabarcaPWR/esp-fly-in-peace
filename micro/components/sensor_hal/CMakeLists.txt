set(SRCS "src/sensor_hal.c")
set(REQUIRES "esp_timer" "esp_driver_i2c")

if(CONFIG_SENSOR_MS5611)
    list(APPEND REQUIRES "sensor_ms5611")
endif()

idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "include"
    REQUIRES ${REQUIRES}
)
