#include "sensor_ms5611_model.h"

#include <stddef.h>

void sensor_ms5611_model_compensate(const uint16_t calibration[6], uint32_t d1, uint32_t d2, int32_t *pressure_pa,
                                    int32_t *temperature_mc)
{
    int32_t dT = (int32_t)d2 - ((int32_t)calibration[4] << 8);
    int32_t TEMP = 2000 + (int32_t)(((int64_t)dT * calibration[5]) >> 23);

    int64_t OFF = ((int64_t)calibration[1] << 16) + (((int64_t)calibration[3] * dT) >> 7);
    int64_t SENS = ((int64_t)calibration[0] << 15) + (((int64_t)calibration[2] * dT) >> 8);

    int32_t T2 = 0;
    int64_t OFF2 = 0;
    int64_t SENS2 = 0;

    if (TEMP < 2000)
    {
        int64_t diff = (int64_t)(TEMP - 2000);
        T2 = (int32_t)(((int64_t)dT * dT) >> 31);
        OFF2 = 5LL * diff * diff / 2LL;
        SENS2 = 5LL * diff * diff / 4LL;

        if (TEMP < -1500)
        {
            int64_t diff2 = (int64_t)(TEMP + 1500);
            OFF2 += 7LL * diff2 * diff2;
            SENS2 += 11LL * diff2 * diff2 / 2LL;
        }
    }

    TEMP -= T2;
    OFF -= OFF2;
    SENS -= SENS2;

    *pressure_pa = (int32_t)(((((int64_t)d1 * SENS) >> 21) - OFF) >> 15);
    *temperature_mc = TEMP * 10;
}

uint8_t sensor_ms5611_model_crc4(uint16_t prom[8])
{
    uint16_t n_rem = 0x0000;
    uint16_t crc_original = prom[7];
    prom[7] = 0xFF00 & prom[7];

    for (int i = 0; i < 16; i++)
    {
        if (i % 2 == 1)
            n_rem ^= (uint16_t)(prom[i >> 1] & 0x00FF);
        else
            n_rem ^= (uint16_t)(prom[i >> 1] >> 8);

        for (int n_bit = 8; n_bit > 0; n_bit--)
        {
            if (n_rem & 0x8000)
                n_rem = (uint16_t)((n_rem << 1) ^ 0x3000);
            else
                n_rem = (uint16_t)(n_rem << 1);
        }
    }

    prom[7] = crc_original;
    return (uint8_t)((n_rem >> 12) & 0x000F);
}

esp_err_t sensor_ms5611_model_validate_init_args(const void *self, const void *cfg)
{
    if (!self || !cfg)
        return ESP_ERR_INVALID_ARG;
    return ESP_OK;
}
