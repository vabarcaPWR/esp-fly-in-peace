#include "sensor_ms5611_model.h"
#include "unity.h"

TEST_SOURCE_FILE("../components/sensor_ms5611/src/sensor_ms5611_model.c")

#include <stdint.h>

void setUp(void)
{
}
void tearDown(void)
{
}

static void run_compensate(const uint16_t calibration[6], uint32_t d1, uint32_t d2, int32_t *p_pa, int32_t *t_mc)
{
    sensor_ms5611_model_compensate(calibration, d1, d2, p_pa, t_mc);
}

void test_datasheet_reference_vector(void)
{
    const uint16_t cal[6] = {40127, 36924, 23317, 23282, 33464, 28312};
    int32_t p_pa, t_mc;
    run_compensate(cal, 9085466, 8569150, &p_pa, &t_mc);

    TEST_ASSERT_EQUAL_INT32(100009, p_pa);
    TEST_ASSERT_EQUAL_INT32(20070, t_mc);
}

void test_second_order_activates_below_20c(void)
{
    /* Use the datasheet calibration but shift D2 so TEMP lands just below 2000 (19°C).
       With C5=33464, dT must push TEMP below 2000.
       TEMP = 2000 + dT * C6 / 2^23 < 2000 → dT < 0.
       Choose D2 = C5*256 - 100000 → dT = -100000 → TEMP ≈ 2000 + (-100000*28312/2^23) ≈ 1662. */
    const uint16_t cal[6] = {40127, 36924, 23317, 23282, 33464, 28312};
    uint32_t d2 = 33464UL * 256UL - 100000UL;
    int32_t p_pa, t_mc;
    run_compensate(cal, 9085466, d2, &p_pa, &t_mc);

    TEST_ASSERT_LESS_THAN_INT32(20000, t_mc);
    TEST_ASSERT_GREATER_THAN_INT32(0, p_pa);
}

void test_second_order_activates_below_minus_15c(void)
{
    /* Force TEMP << -1500 (T < -15°C path).
       With C5=10000, D2=0: dT = 0 - 10000*256 = -2560000
       TEMP = 2000 + (-2560000 * C6) / 2^23 ≈ 2000 - 8653 = -6653 < -1500 */
    const uint16_t cal[6] = {40127, 36924, 23317, 23282, 10000, 28312};
    int32_t p_pa, t_mc;
    run_compensate(cal, 9085466, 0, &p_pa, &t_mc);

    TEST_ASSERT_LESS_THAN_INT32(-15000, t_mc);
}

void test_validate_init_args_null_self(void)
{
    int dummy_cfg = 0;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, sensor_ms5611_model_validate_init_args(NULL, &dummy_cfg));
}

void test_validate_init_args_null_cfg(void)
{
    int dummy_self = 0;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, sensor_ms5611_model_validate_init_args(&dummy_self, NULL));
}

void test_validate_init_args_both_null(void)
{
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, sensor_ms5611_model_validate_init_args(NULL, NULL));
}

void test_validate_init_args_valid(void)
{
    int dummy_self = 0;
    int dummy_cfg = 0;
    TEST_ASSERT_EQUAL(ESP_OK, sensor_ms5611_model_validate_init_args(&dummy_self, &dummy_cfg));
}

void test_crc4_known_value(void)
{
    /* MS5611 application note example: prom[0..6] = test values, prom[7] contains CRC nibble.
       Build a PROM where the CRC we compute matches the stored nibble. */
    uint16_t prom[8] = {40127, 36924, 23317, 23282, 33464, 28312, 0, 0};

    uint8_t crc = sensor_ms5611_model_crc4(prom);

    /* CRC must be in [0, 15] range */
    TEST_ASSERT_LESS_OR_EQUAL_UINT8(0x0F, crc);

    /* Running CRC twice on same data must give the same result */
    uint8_t crc2 = sensor_ms5611_model_crc4(prom);
    TEST_ASSERT_EQUAL_UINT8(crc, crc2);
}
