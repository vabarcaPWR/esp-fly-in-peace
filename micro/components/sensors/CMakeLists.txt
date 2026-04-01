set(SENSOR_SRCS "src/sensor.c")
set(SENSOR_INCLUDE_DIRS "inc")

if(CONFIG_SENSOR_MS5611)
    include(${CMAKE_CURRENT_LIST_DIR}/src/ms5611/CMakeLists.txt)
endif()

if(CONFIG_SENSOR_BMP390)
    include(${CMAKE_CURRENT_LIST_DIR}/src/bmp390/CMakeLists.txt)
endif()

if(CONFIG_IMU_MPU6050)
    include(${CMAKE_CURRENT_LIST_DIR}/src/mpu6050/CMakeLists.txt)
endif()

idf_component_register(
    SRCS ${SENSOR_SRCS}
    INCLUDE_DIRS ${SENSOR_INCLUDE_DIRS}
    PRIV_INCLUDE_DIRS "src"
    REQUIRES esp_timer bus_drivers
    PRIV_REQUIRES esp_driver_i2c
)
