idf_component_register(
    SRCS "src/sensor.c"
         "src/i2c_bus.c"
         "src/ms5611/src/ms5611.c"
         "src/mpu6050/src/mpu6050.c"
    INCLUDE_DIRS "inc"
                 "src/ms5611/inc"
                 "src/mpu6050/inc"
    PRIV_INCLUDE_DIRS "src"
    REQUIRES esp_timer
    PRIV_REQUIRES esp_driver_i2c
)
