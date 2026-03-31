#ifndef FLIGHT_DATA_H
#define FLIGHT_DATA_H

#include "esp_err.h"
#include "sensor.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct flight_data_s
    {
        float altitude_m;
        float vario_ms;
        int32_t pressure_pa;
        int32_t temperature_mc;
        float reference_pressure_pa;
        int64_t timestamp_us;
        bool sensor_valid;
    } flight_data_t;

    typedef struct calibration_request_s
    {
        float known_altitude_m;
    } calibration_request_t;

    /**
     * @brief Create internal mutex and queues. Call once before any task starts.
     */
    esp_err_t flight_data_init(void);

    /**
     * @brief Thread-safe publish of flight data (writer: fusion_task).
     */
    esp_err_t flight_data_publish(const flight_data_t *data);

    /**
     * @brief Thread-safe snapshot read of flight data (readers: ble_sender, sound).
     */
    esp_err_t flight_data_read(flight_data_t *snapshot);

    /**
     * @brief Overwrite the baro queue with a new sample (writer: baro_task).
     */
    esp_err_t baro_queue_send(const data_baro_t *data);

    /**
     * @brief Blocking receive from baro queue (reader: fusion_task).
     */
    esp_err_t baro_queue_receive(data_baro_t *out, uint32_t timeout_ms);

    /**
     * @brief Enqueue a calibration request.
     */
    esp_err_t calibration_queue_send(const calibration_request_t *req);

    /**
     * @brief Non-blocking poll for a calibration request (reader: fusion_task).
     */
    esp_err_t calibration_queue_receive(calibration_request_t *out);

#ifdef __cplusplus
}
#endif

#endif
