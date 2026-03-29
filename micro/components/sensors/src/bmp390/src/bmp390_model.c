#include "bmp390_model.h"

#include <stddef.h>

void bmp390_parse_calib(const uint8_t raw[21], bmp390_calib_t *calib)
{
    if (!raw || !calib)
        return;

    uint16_t t1_raw = (uint16_t)((uint16_t)raw[1] << 8U) | raw[0];
    uint16_t t2_raw = (uint16_t)((uint16_t)raw[3] << 8U) | raw[2];
    int8_t t3_raw = (int8_t)raw[4];

    calib->par_t1 = (double)t1_raw * 256.0;
    calib->par_t2 = (double)t2_raw / 1073741824.0;
    calib->par_t3 = (double)t3_raw / 281474976710656.0;

    int16_t p1_raw = (int16_t)((uint16_t)raw[6] << 8U | raw[5]);
    int16_t p2_raw = (int16_t)((uint16_t)raw[8] << 8U | raw[7]);
    int8_t p3_raw = (int8_t)raw[9];
    int8_t p4_raw = (int8_t)raw[10];
    uint16_t p5_raw = (uint16_t)((uint16_t)raw[12] << 8U) | raw[11];
    uint16_t p6_raw = (uint16_t)((uint16_t)raw[14] << 8U) | raw[13];
    int8_t p7_raw = (int8_t)raw[15];
    int8_t p8_raw = (int8_t)raw[16];
    int16_t p9_raw = (int16_t)((uint16_t)raw[18] << 8U | raw[17]);
    int8_t p10_raw = (int8_t)raw[19];
    int8_t p11_raw = (int8_t)raw[20];

    calib->par_p1 = ((double)p1_raw - 16384.0) / 1048576.0;
    calib->par_p2 = ((double)p2_raw - 16384.0) / 536870912.0;
    calib->par_p3 = (double)p3_raw / 4294967296.0;
    calib->par_p4 = (double)p4_raw / 137438953472.0;
    calib->par_p5 = (double)p5_raw * 8.0;
    calib->par_p6 = (double)p6_raw / 64.0;
    calib->par_p7 = (double)p7_raw / 256.0;
    calib->par_p8 = (double)p8_raw / 32768.0;
    calib->par_p9 = (double)p9_raw / 281474976710656.0;
    calib->par_p10 = (double)p10_raw / 281474976710656.0;
    calib->par_p11 = (double)p11_raw / 36893488147419103232.0;
}

esp_err_t bmp390_compensate(uint32_t raw_pressure, uint32_t raw_temp, const bmp390_calib_t *calib, int32_t *pressure_pa,
                            int32_t *temperature_mc)
{
    if (!calib || !pressure_pa || !temperature_mc)
        return ESP_ERR_INVALID_ARG;

    double pd1 = (double)raw_temp - calib->par_t1;
    double pd2 = pd1 * calib->par_t2;
    double t_lin = pd2 + pd1 * pd1 * calib->par_t3;

    if (t_lin < -40.0)
        t_lin = -40.0;
    if (t_lin > 85.0)
        t_lin = 85.0;

    *temperature_mc = (int32_t)(t_lin * 1000.0);

    double t2 = t_lin * t_lin;
    double t3 = t2 * t_lin;
    double out1 = calib->par_p5 + calib->par_p6 * t_lin + calib->par_p7 * t2 + calib->par_p8 * t3;

    double p_raw = (double)raw_pressure;
    double out2 = p_raw * (calib->par_p1 + calib->par_p2 * t_lin + calib->par_p3 * t2 + calib->par_p4 * t3);

    double p2 = p_raw * p_raw;
    double pd_9_10 = calib->par_p9 + calib->par_p10 * t_lin;
    double out3 = p2 * pd_9_10 + p2 * p_raw * calib->par_p11;

    double comp_press = out1 + out2 + out3;

    if (comp_press < 30000.0)
        comp_press = 30000.0;
    if (comp_press > 125000.0)
        comp_press = 125000.0;

    *pressure_pa = (int32_t)comp_press;
    return ESP_OK;
}
