idf_component_register(
    SRCS "src/ble_nus.c"
         "src/ble_nus_conductor.c"
         "src/ble_nus_model.c"
         "src/ble_nus_hardware.c"
    INCLUDE_DIRS "include"
    REQUIRES bt nvs_flash freertos
)
