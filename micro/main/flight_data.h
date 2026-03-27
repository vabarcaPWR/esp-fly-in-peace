#ifndef FLIGHT_DATA_H
#define FLIGHT_DATA_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "sensor.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define FLIGHT_DATA_MUTEX_TIMEOUT_MS 10U

    typedef struct flight_data_s
    {
        float altitude_m;
        float vario_ms;
        int32_t pressure_pa;
        int32_t temperature_mc;
        float reference_pressure_pa;
        float vertical_accel_ms2;
        int64_t timestamp_us;
        bool sensor_valid;
        bool imu_valid;
    } flight_data_t;

    typedef struct calibration_request_s
    {
        float known_altitude_m;
    } calibration_request_t;

    extern SemaphoreHandle_t g_flight_data_mutex;
    extern flight_data_t g_flight_data;
    extern QueueHandle_t g_baro_queue;
    extern QueueHandle_t g_calibration_queue;

#ifdef __cplusplus
}
#endif

#endif
