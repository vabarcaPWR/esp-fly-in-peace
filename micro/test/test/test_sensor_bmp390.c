#include "bmp390_model.h"
#include "unity.h"

#include <math.h>

TEST_SOURCE_FILE("../components/sensors/src/bmp390/src/bmp390_model.c")

static void build_known_calib_raw(uint8_t raw[21])
{
    uint16_t t1 = 27504;
    uint16_t t2 = 18887;
    int8_t t3 = -10;
    int16_t p1 = 20000;
    int16_t p2 = -496;
    int8_t p3 = 9;
    int8_t p4 = 0;
    uint16_t p5 = 11500;
    uint16_t p6 = 29970;
    int8_t p7 = -30;
    int8_t p8 = -13;
    int16_t p9 = 7244;
    int8_t p10 = 27;
    int8_t p11 = -9;

    raw[0] = (uint8_t)(t1 & 0xFF);
    raw[1] = (uint8_t)(t1 >> 8);
    raw[2] = (uint8_t)(t2 & 0xFF);
    raw[3] = (uint8_t)(t2 >> 8);
    raw[4] = (uint8_t)t3;
    raw[5] = (uint8_t)(((uint16_t)p1) & 0xFF);
    raw[6] = (uint8_t)(((uint16_t)p1) >> 8);
    raw[7] = (uint8_t)(((uint16_t)p2) & 0xFF);
    raw[8] = (uint8_t)(((uint16_t)p2) >> 8);
    raw[9] = (uint8_t)p3;
    raw[10] = (uint8_t)p4;
    raw[11] = (uint8_t)(p5 & 0xFF);
    raw[12] = (uint8_t)(p5 >> 8);
    raw[13] = (uint8_t)(p6 & 0xFF);
    raw[14] = (uint8_t)(p6 >> 8);
    raw[15] = (uint8_t)p7;
    raw[16] = (uint8_t)p8;
    raw[17] = (uint8_t)(((uint16_t)p9) & 0xFF);
    raw[18] = (uint8_t)(((uint16_t)p9) >> 8);
    raw[19] = (uint8_t)p10;
    raw[20] = (uint8_t)p11;
}

void setUp(void)
{
}
void tearDown(void)
{
}

void test_parse_calib_sets_par_t1(void)
{
    uint8_t raw[21];
    bmp390_calib_t calib;
    build_known_calib_raw(raw);
    bmp390_parse_calib(raw, &calib);
    TEST_ASSERT_DOUBLE_WITHIN(1.0, 27504.0 * 256.0, calib.par_t1);
}

void test_parse_calib_sets_par_t2(void)
{
    uint8_t raw[21];
    bmp390_calib_t calib;
    build_known_calib_raw(raw);
    bmp390_parse_calib(raw, &calib);
    TEST_ASSERT_DOUBLE_WITHIN(1e-8, 18887.0 / 1073741824.0, calib.par_t2);
}

void test_parse_calib_sets_par_t3_negative(void)
{
    uint8_t raw[21];
    bmp390_calib_t calib;
    build_known_calib_raw(raw);
    bmp390_parse_calib(raw, &calib);
    TEST_ASSERT_TRUE(calib.par_t3 < 0.0);
}

void test_parse_calib_sets_par_p5(void)
{
    uint8_t raw[21];
    bmp390_calib_t calib;
    build_known_calib_raw(raw);
    bmp390_parse_calib(raw, &calib);
    TEST_ASSERT_DOUBLE_WITHIN(1.0, 11500.0 * 8.0, calib.par_p5);
}

void test_parse_calib_sets_par_p1_with_offset(void)
{
    uint8_t raw[21];
    bmp390_calib_t calib;
    build_known_calib_raw(raw);
    bmp390_parse_calib(raw, &calib);
    TEST_ASSERT_DOUBLE_WITHIN(1e-4, (20000.0 - 16384.0) / 1048576.0, calib.par_p1);
}

void test_parse_calib_null_raw_does_not_crash(void)
{
    bmp390_calib_t calib;
    bmp390_parse_calib(NULL, &calib);
}

void test_parse_calib_null_calib_does_not_crash(void)
{
    uint8_t raw[21] = {0};
    bmp390_parse_calib(raw, NULL);
}

void test_compensate_null_calib_returns_error(void)
{
    int32_t p, t;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, bmp390_compensate(0, 0, NULL, &p, &t));
}

void test_compensate_null_pressure_returns_error(void)
{
    bmp390_calib_t calib = {0};
    int32_t t;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, bmp390_compensate(0, 0, &calib, NULL, &t));
}

void test_compensate_null_temperature_returns_error(void)
{
    bmp390_calib_t calib = {0};
    int32_t p;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, bmp390_compensate(0, 0, &calib, &p, NULL));
}

void test_compensate_returns_ok_with_valid_args(void)
{
    uint8_t raw[21];
    bmp390_calib_t calib;
    build_known_calib_raw(raw);
    bmp390_parse_calib(raw, &calib);

    int32_t p, t;
    uint32_t raw_temp = 8200000;
    uint32_t raw_press = 7200000;

    TEST_ASSERT_EQUAL(ESP_OK, bmp390_compensate(raw_press, raw_temp, &calib, &p, &t));
}

void test_compensate_temperature_in_plausible_range(void)
{
    uint8_t raw[21];
    bmp390_calib_t calib;
    build_known_calib_raw(raw);
    bmp390_parse_calib(raw, &calib);

    int32_t p, t;
    uint32_t raw_temp = 8200000;
    uint32_t raw_press = 7200000;

    bmp390_compensate(raw_press, raw_temp, &calib, &p, &t);
    TEST_ASSERT_INT_WITHIN(30000, 25000, t);
}

void test_compensate_pressure_in_plausible_range(void)
{
    uint8_t raw[21];
    bmp390_calib_t calib;
    build_known_calib_raw(raw);
    bmp390_parse_calib(raw, &calib);

    int32_t p, t;
    uint32_t raw_temp = 8200000;
    uint32_t raw_press = 7200000;

    bmp390_compensate(raw_press, raw_temp, &calib, &p, &t);
    TEST_ASSERT_INT_WITHIN(30000, 101325, p);
}

void test_compensate_temperature_clamps_below_minus_40(void)
{
    bmp390_calib_t calib = {
        .par_t1 = 50000000.0,
        .par_t2 = 0.00001,
        .par_t3 = 0.0,
    };

    int32_t p, t;
    bmp390_compensate(0, 0, &calib, &p, &t);
    TEST_ASSERT_EQUAL(-40000, t);
}

void test_compensate_temperature_clamps_above_85(void)
{
    bmp390_calib_t calib = {
        .par_t1 = 0.0,
        .par_t2 = 1.0,
        .par_t3 = 0.0,
        .par_p5 = 101325.0 * 8.0,
    };

    int32_t p, t;
    bmp390_compensate(0, 100000000, &calib, &p, &t);
    TEST_ASSERT_EQUAL(85000, t);
}

void test_compensate_pressure_clamps_to_minimum(void)
{
    bmp390_calib_t calib = {0};
    calib.par_p5 = 10000.0;

    int32_t p, t;
    bmp390_compensate(0, 0, &calib, &p, &t);
    TEST_ASSERT_EQUAL(30000, p);
}

void test_compensate_higher_raw_temp_gives_higher_temperature(void)
{
    uint8_t raw[21];
    bmp390_calib_t calib;
    build_known_calib_raw(raw);
    bmp390_parse_calib(raw, &calib);

    int32_t p1, t1, p2, t2;
    bmp390_compensate(7200000, 8000000, &calib, &p1, &t1);
    bmp390_compensate(7200000, 8400000, &calib, &p2, &t2);
    TEST_ASSERT_TRUE(t2 > t1);
}

void test_compensate_higher_raw_press_gives_higher_pressure(void)
{
    uint8_t raw[21];
    bmp390_calib_t calib;
    build_known_calib_raw(raw);
    bmp390_parse_calib(raw, &calib);

    int32_t p1, t1, p2, t2;
    bmp390_compensate(7000000, 8200000, &calib, &p1, &t1);
    bmp390_compensate(7400000, 8200000, &calib, &p2, &t2);
    TEST_ASSERT_TRUE(p2 > p1);
}
