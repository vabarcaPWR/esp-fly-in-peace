#ifndef SENSOR_H
#define SENSOR_H

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /** @brief Sensor API structure common to all drivers. */
    typedef struct sensor_s
    {
        esp_err_t (*init)(void);
        int32_t (*get_pressure_pa)(void);
        int32_t (*get_temperature_mc)(void);
        int64_t (*get_timestamp_us)(void);
        const char* (*get_name)(void);
        void (*update)(void);
    } sensor_t;

    sensor_t *get_sensor(const char *sensor_name);

#ifdef __cplusplus
}
#endif

#endif // SENSOR_H
