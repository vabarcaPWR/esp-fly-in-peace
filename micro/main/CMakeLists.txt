idf_component_register(
    SRCS
        "main.c"
        "flight_data.c"
        "baro_task.c"
        "fusion_task.c"
        "ble_sender_task.c"
        "sound_task.c"
    INCLUDE_DIRS "."
    REQUIRES ble_nus ekf leds lk8ex1 sensors sound
)
