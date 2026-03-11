idf_component_register(
    SRCS "src/sensor.c"
         "src/ms5611/src/ms5611.c"
    INCLUDE_DIRS "inc"
                 "src/ms5611/inc"
    REQUIRES esp_timer
    PRIV_REQUIRES esp_driver_i2c
)
