idf_component_register(
    SRCS "i2c/src/i2c_bus.c"
    INCLUDE_DIRS "i2c/inc"
    REQUIRES esp_timer
    PRIV_REQUIRES esp_driver_i2c
)
