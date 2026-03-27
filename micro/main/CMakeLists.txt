idf_component_register(
    SRCS
        "main.c"
        "baro_task.c"
        "fusion_task.c"
        "ble_sender_task.c"
    INCLUDE_DIRS "."
    REQUIRES ahrs ble_nus ekf leds lk8ex1 sensors
)
