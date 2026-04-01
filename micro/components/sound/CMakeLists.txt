set(SOUND_SRCS "src/sound.c")
set(SOUND_INCLUDE_DIRS "inc")

include(${CMAKE_CURRENT_LIST_DIR}/src/tone/CMakeLists.txt)

if(CONFIG_SOUND_PIEZO)
    include(${CMAKE_CURRENT_LIST_DIR}/src/piezo/CMakeLists.txt)
endif()

if(CONFIG_SOUND_MAX98357)
    include(${CMAKE_CURRENT_LIST_DIR}/src/max98357/CMakeLists.txt)
endif()

idf_component_register(
    SRCS ${SOUND_SRCS}
    INCLUDE_DIRS ${SOUND_INCLUDE_DIRS}
    PRIV_INCLUDE_DIRS "src"
    REQUIRES freertos
    PRIV_REQUIRES esp_driver_ledc esp_driver_i2s esp_driver_gpio
)
