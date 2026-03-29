#ifndef BMP390_MODEL_H
#define BMP390_MODEL_H

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct bmp390_calib_s
    {
        double par_t1;
        double par_t2;
        double par_t3;
        double par_p1;
        double par_p2;
        double par_p3;
        double par_p4;
        double par_p5;
        double par_p6;
        double par_p7;
        double par_p8;
        double par_p9;
        double par_p10;
        double par_p11;
    } bmp390_calib_t;

    /**
     * @brief Parse 21 raw calibration bytes into quantized coefficients.
     */
    void bmp390_parse_calib(const uint8_t raw[21], bmp390_calib_t *calib);

    /**
     * @brief Compensate raw ADC values using calibration data.
     * @param raw_pressure  24-bit unsigned raw pressure from registers 0x04-0x06.
     * @param raw_temp      24-bit unsigned raw temperature from registers 0x07-0x09.
     * @param calib         Quantized calibration coefficients.
     * @param pressure_pa   Output: compensated pressure in Pa.
     * @param temperature_mc Output: compensated temperature in milli-°C.
     * @return ESP_OK on success.
     */
    esp_err_t bmp390_compensate(uint32_t raw_pressure, uint32_t raw_temp, const bmp390_calib_t *calib,
                                int32_t *pressure_pa, int32_t *temperature_mc);

#ifdef __cplusplus
}
#endif

#endif // BMP390_MODEL_H
