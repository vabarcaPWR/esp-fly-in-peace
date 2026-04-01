set(BUS_SRCS "")
set(BUS_INCLUDE_DIRS "")

include(${CMAKE_CURRENT_LIST_DIR}/i2c/CMakeLists.txt)

idf_component_register(
    SRCS ${BUS_SRCS}
    INCLUDE_DIRS ${BUS_INCLUDE_DIRS}
    REQUIRES esp_timer
    PRIV_REQUIRES esp_driver_i2c
)
