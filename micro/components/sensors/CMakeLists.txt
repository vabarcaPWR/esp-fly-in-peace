set(SENSOR_SRCS "src/sensor.c")
set(SENSOR_INCLUDE_DIRS "inc")

if(CONFIG_SENSOR_MS5611)
    list(APPEND SENSOR_SRCS "src/ms5611/src/ms5611.c")
    list(APPEND SENSOR_INCLUDE_DIRS "src/ms5611/inc")
endif()

if(CONFIG_SENSOR_BMP390)
    list(APPEND SENSOR_SRCS "src/bmp390/src/bmp390.c"
                            "src/bmp390/src/bmp390_model.c")
    list(APPEND SENSOR_INCLUDE_DIRS "src/bmp390/inc")
endif()

if(CONFIG_IMU_MPU6050)
    list(APPEND SENSOR_SRCS "src/mpu6050/src/mpu6050.c")
    list(APPEND SENSOR_INCLUDE_DIRS "src/mpu6050/inc")
endif()

idf_component_register(
    SRCS ${SENSOR_SRCS}
    INCLUDE_DIRS ${SENSOR_INCLUDE_DIRS}
    PRIV_INCLUDE_DIRS "src"
    REQUIRES esp_timer bus_drivers
    PRIV_REQUIRES esp_driver_i2c
)
