idf_component_register(
    SRCS
        "src/led.c"
        "src/led_single/led_single.c"
        "src/led_single/led_single.h"
    INCLUDE_DIRS
        "inc"
    REQUIRES driver freertos
)
