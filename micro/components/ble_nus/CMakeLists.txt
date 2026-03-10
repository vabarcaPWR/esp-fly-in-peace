idf_component_register(
    SRCS "src/ble_nus.c"
         "src/ble_nus_model.c"
         "src/ble_nus_hardware.c"
    INCLUDE_DIRS "inc"
    REQUIRES bt nvs_flash freertos
)
