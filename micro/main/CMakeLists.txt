idf_component_register(
    SRCS
        "main.c"
        "lk8ex1_simulation_profile.c"
        "lk8ex1_simulation_runtime.c"
    INCLUDE_DIRS "."
    REQUIRES ble_nus leds lk8ex1 sensors
)
