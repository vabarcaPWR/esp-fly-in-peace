set(SOUND_SRCS "src/sound.c")
set(SOUND_INCLUDE_DIRS "inc")

if(CONFIG_SOUND_PIEZO)
    list(APPEND SOUND_SRCS
        "src/piezo/src/piezo.c"
        "src/piezo/src/piezo_model.c"
        "src/piezo/src/piezo_hardware.c"
    )
    list(APPEND SOUND_INCLUDE_DIRS "src/piezo/inc")
endif()

idf_component_register(
    SRCS ${SOUND_SRCS}
    INCLUDE_DIRS ${SOUND_INCLUDE_DIRS}
    PRIV_INCLUDE_DIRS "src"
    REQUIRES esp_driver_ledc freertos
)
