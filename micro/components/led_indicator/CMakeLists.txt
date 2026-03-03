idf_component_register(
    SRCS "src/led_indicator.c"
         "src/led_indicator_model.c"
         "src/led_indicator_hardware.c"
    INCLUDE_DIRS "include"
    REQUIRES freertos driver led_strip
)
