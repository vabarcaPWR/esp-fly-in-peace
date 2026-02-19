#ifndef LK8EX1_H
#define LK8EX1_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** Maximum sentence length including $, checksum, \r\n, and null terminator. */
#define LK8EX1_MAX_SENTENCE_LEN 64

    /** @brief Input data for LK8EX1 sentence formatting. All integer, no floats. */
    typedef struct lk8ex1_data_s
    {
        int32_t pressure_pa;    /**< Pressure in Pascals (e.g., 101325) */
        int32_t altitude_m;     /**< Altitude in meters (99999 = not available) */
        int32_t vario_cms;      /**< Vertical speed in cm/s (e.g., 50 = 0.50 m/s) */
        int32_t temperature_dc; /**< Temperature in °C × 10 (e.g., 235 = 23.5°C) */
        int32_t battery_mv;     /**< Battery percentage (0-100, 999 for unavailable) */
    } lk8ex1_data_t;

    /** @brief Format LK8EX1 NMEA sentence into buffer. Requires buffer_size >= LK8EX1_MAX_SENTENCE_LEN. */
    esp_err_t lk8ex1_format(const lk8ex1_data_t *data, char *buffer, size_t buffer_size);

    /** @brief Compute NMEA XOR checksum of chars between '$' and '*' (exclusive). Returns 0 on invalid input. */
    uint8_t lk8ex1_checksum(const char *sentence, size_t len);

    /** @brief Validate sentence by recomputing and comparing its embedded checksum. */
    bool lk8ex1_validate(const char *sentence);

#ifdef __cplusplus
}
#endif

#endif /* LK8EX1_H */
