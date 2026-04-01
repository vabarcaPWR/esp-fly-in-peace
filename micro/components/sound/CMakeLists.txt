set(SOUND_SRCS "src/sound.c" "src/tone/src/tone_model.c")
set(SOUND_INCLUDE_DIRS "inc" "src/tone/inc")

if(CONFIG_SOUND_PIEZO)
    list(APPEND SOUND_SRCS
        "src/piezo/src/piezo.c"
        "src/piezo/src/piezo_hardware.c"
    )
    list(APPEND SOUND_INCLUDE_DIRS "src/piezo/inc")
endif()

if(CONFIG_SOUND_MAX98357)
    list(APPEND SOUND_SRCS
        "src/max98357/src/max98357.c"
        "src/max98357/src/max98357_model.c"
        "src/max98357/src/max98357_hardware.c"
    )
    list(APPEND SOUND_INCLUDE_DIRS "src/max98357/inc")
endif()

idf_component_register(
    SRCS ${SOUND_SRCS}
    INCLUDE_DIRS ${SOUND_INCLUDE_DIRS}
    PRIV_INCLUDE_DIRS "src"
    REQUIRES freertos
    PRIV_REQUIRES esp_driver_ledc esp_driver_i2s esp_driver_gpio
)
