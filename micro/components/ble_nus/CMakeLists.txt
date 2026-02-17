idf_component_register(
    SRCS "src/ble_nus.c"
    INCLUDE_DIRS "include"
    REQUIRES bt nvs_flash freertos
)
