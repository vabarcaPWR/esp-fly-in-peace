set(LED_SRCS "src/led.c")
set(LED_INCLUDE_DIRS "inc")

if(CONFIG_LED_SINGLE)
    include(${CMAKE_CURRENT_LIST_DIR}/src/led_single/CMakeLists.txt)
endif()

if(CONFIG_LED_RGB)
    include(${CMAKE_CURRENT_LIST_DIR}/src/led_rgb/CMakeLists.txt)
endif()

if(CONFIG_LED_RGB)
    idf_component_register(
        SRCS ${LED_SRCS}
        INCLUDE_DIRS ${LED_INCLUDE_DIRS}
        REQUIRES driver freertos
        PRIV_REQUIRES espressif__led_strip esp_driver_rmt
    )
else()
    idf_component_register(
        SRCS ${LED_SRCS}
        INCLUDE_DIRS ${LED_INCLUDE_DIRS}
        REQUIRES driver freertos
    )
endif()
