set(SENSOR_SRCS "src/sensor.c"
                "src/i2c_bus.c")
set(SENSOR_INCLUDE_DIRS "inc")

if(CONFIG_SENSOR_MS5611)
    list(APPEND SENSOR_SRCS "src/ms5611/src/ms5611.c")
    list(APPEND SENSOR_INCLUDE_DIRS "src/ms5611/inc")
endif()

if(CONFIG_IMU_MPU6050)
    list(APPEND SENSOR_SRCS "src/mpu6050/src/mpu6050.c")
    list(APPEND SENSOR_INCLUDE_DIRS "src/mpu6050/inc")
endif()

idf_component_register(
    SRCS ${SENSOR_SRCS}
    INCLUDE_DIRS ${SENSOR_INCLUDE_DIRS}
    PRIV_INCLUDE_DIRS "src"
    REQUIRES esp_timer
    PRIV_REQUIRES esp_driver_i2c
)
