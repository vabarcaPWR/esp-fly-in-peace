set(LED_SRCS "src/led.c")
set(LED_INCLUDE_DIRS "inc")

include(${CMAKE_CURRENT_LIST_DIR}/src/led_single/CMakeLists.txt)

idf_component_register(
    SRCS ${LED_SRCS}
    INCLUDE_DIRS ${LED_INCLUDE_DIRS}
    REQUIRES driver freertos
)
