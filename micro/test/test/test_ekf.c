#include "ekf.h"
#include "unity.h"
#include <math.h>
#include <string.h>

TEST_SOURCE_FILE("../components/ekf/src/ekf.c")

#define FLOAT_TOL 0.05f
#define ALTITUDE_TOL 1.0f
#define VARIO_TOL 0.2f
#define BIAS_TOL 0.1f

static ekf_cfg_t default_cfg(void)
{
    ekf_cfg_t cfg = {
        .q_altitude = EKF_DEFAULT_Q_ALTITUDE,
        .q_vario = EKF_DEFAULT_Q_VARIO,
        .q_accel_bias = EKF_DEFAULT_Q_ACCEL_BIAS,
        .r_altitude = EKF_DEFAULT_R_ALTITUDE,
        .reference_pressure_pa = EKF_DEFAULT_REFERENCE_PA,
    };
    return cfg;
}

void setUp(void)
{
}

void tearDown(void)
{
}

void test_ekf_init_null_state_returns_invalid_arg(void)
{
    ekf_cfg_t cfg = default_cfg();
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ekf_init(NULL, &cfg));
}

void test_ekf_init_null_cfg_returns_invalid_arg(void)
{
    ekf_state_t state;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ekf_init(&state, NULL));
}

void test_ekf_predict_null_state_returns_invalid_arg(void)
{
    ekf_cfg_t cfg = default_cfg();
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ekf_predict(NULL, &cfg, 0.0f, 0));
}

void test_ekf_update_baro_null_state_returns_invalid_arg(void)
{
    ekf_cfg_t cfg = default_cfg();
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ekf_update_baro(NULL, &cfg, 101325.0f, 0));
}

void test_ekf_reset_null_state_returns_invalid_arg(void)
{
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ekf_reset(NULL));
}

void test_ekf_calibrate_null_cfg_returns_invalid_arg(void)
{
    ekf_state_t state;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ekf_calibrate(NULL, &state, 0.0f, 101325.0f));
}

void test_ekf_calibrate_null_state_returns_invalid_arg(void)
{
    ekf_cfg_t cfg = default_cfg();
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ekf_calibrate(&cfg, NULL, 0.0f, 101325.0f));
}

void test_ekf_init_succeeds(void)
{
    ekf_state_t state;
    ekf_cfg_t cfg = default_cfg();
    TEST_ASSERT_EQUAL(ESP_OK, ekf_init(&state, &cfg));
}

void test_ekf_init_clears_state(void)
{
    ekf_state_t state;
    ekf_cfg_t cfg = default_cfg();
    ekf_init(&state, &cfg);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_TOL, 0.0f, state.altitude_m);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_TOL, 0.0f, state.vario_ms);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_TOL, 0.0f, state.accel_bias_ms2);
    TEST_ASSERT_FALSE(state.initialized);
}

void test_ekf_reset_clears_state(void)
{
    ekf_state_t state;
    ekf_cfg_t cfg = default_cfg();
    ekf_init(&state, &cfg);
    state.altitude_m = 500.0f;
    state.vario_ms = 2.0f;
    state.initialized = true;
    TEST_ASSERT_EQUAL(ESP_OK, ekf_reset(&state));
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_TOL, 0.0f, state.altitude_m);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_TOL, 0.0f, state.vario_ms);
    TEST_ASSERT_FALSE(state.initialized);
}

void test_pressure_to_altitude_sea_level(void)
{
    float alt = ekf_pressure_to_altitude(101325.0f, 101325.0f);
    TEST_ASSERT_FLOAT_WITHIN(ALTITUDE_TOL, 0.0f, alt);
}

void test_pressure_to_altitude_1000m(void)
{
    float alt = ekf_pressure_to_altitude(89876.0f, 101325.0f);
    TEST_ASSERT_FLOAT_WITHIN(ALTITUDE_TOL, 1000.0f, alt);
}

void test_pressure_to_altitude_2000m(void)
{
    float alt = ekf_pressure_to_altitude(79501.0f, 101325.0f);
    TEST_ASSERT_FLOAT_WITHIN(ALTITUDE_TOL, 2000.0f, alt);
}

void test_ekf_constant_pressure_zero_accel_stable(void)
{
    ekf_state_t state;
    ekf_cfg_t cfg = default_cfg();
    ekf_init(&state, &cfg);

    int64_t t = 0;
    ekf_update_baro(&state, &cfg, 101325.0f, t);
    TEST_ASSERT_TRUE(state.initialized);

    for (int i = 1; i <= 1000; i++)
    {
        t = (int64_t)i * 10000;
        ekf_predict(&state, &cfg, 0.0f, t);

        if (i % 10 == 0)
        {
            ekf_update_baro(&state, &cfg, 101325.0f, t);
        }
    }

    TEST_ASSERT_FLOAT_WITHIN(ALTITUDE_TOL, 0.0f, state.altitude_m);
    TEST_ASSERT_FLOAT_WITHIN(VARIO_TOL, 0.0f, state.vario_ms);
}

void test_ekf_upward_accel_positive_vario(void)
{
    ekf_state_t state;
    ekf_cfg_t cfg = default_cfg();
    ekf_init(&state, &cfg);

    int64_t t = 0;
    ekf_update_baro(&state, &cfg, 101325.0f, t);

    for (int i = 1; i <= 100; i++)
    {
        t = (int64_t)i * 10000;
        ekf_predict(&state, &cfg, 2.0f, t);
    }

    TEST_ASSERT_TRUE(state.vario_ms > 0.5f);
}

void test_ekf_downward_accel_negative_vario(void)
{
    ekf_state_t state;
    ekf_cfg_t cfg = default_cfg();
    ekf_init(&state, &cfg);

    int64_t t = 0;
    ekf_update_baro(&state, &cfg, 101325.0f, t);

    for (int i = 1; i <= 100; i++)
    {
        t = (int64_t)i * 10000;
        ekf_predict(&state, &cfg, -2.0f, t);
    }

    TEST_ASSERT_TRUE(state.vario_ms < -0.5f);
}

void test_ekf_predict_and_baro_update_converges(void)
{
    ekf_state_t state;
    ekf_cfg_t cfg = default_cfg();
    ekf_init(&state, &cfg);

    float target_pressure = 95461.0f;
    float target_alt = ekf_pressure_to_altitude(target_pressure, cfg.reference_pressure_pa);

    int64_t t = 0;
    ekf_update_baro(&state, &cfg, target_pressure, t);

    for (int i = 1; i <= 1000; i++)
    {
        t = (int64_t)i * 10000;
        ekf_predict(&state, &cfg, 0.0f, t);

        if (i % 10 == 0)
        {
            ekf_update_baro(&state, &cfg, target_pressure, t);
        }
    }

    TEST_ASSERT_FLOAT_WITHIN(ALTITUDE_TOL, target_alt, state.altitude_m);
    TEST_ASSERT_FLOAT_WITHIN(VARIO_TOL, 0.0f, state.vario_ms);
}

void test_ekf_accel_bias_converges(void)
{
    ekf_state_t state;
    ekf_cfg_t cfg = default_cfg();
    ekf_init(&state, &cfg);

    float bias = 0.3f;
    int64_t t = 0;
    ekf_update_baro(&state, &cfg, 101325.0f, t);

    for (int i = 1; i <= 6000; i++)
    {
        t = (int64_t)i * 10000;
        ekf_predict(&state, &cfg, bias, t);

        if (i % 10 == 0)
        {
            ekf_update_baro(&state, &cfg, 101325.0f, t);
        }
    }

    TEST_ASSERT_FLOAT_WITHIN(BIAS_TOL, bias, state.accel_bias_ms2);
}

void test_ekf_innovation_gating_rejects_spurious_baro(void)
{
    ekf_state_t state;
    ekf_cfg_t cfg = default_cfg();
    ekf_init(&state, &cfg);

    int64_t t = 0;
    ekf_update_baro(&state, &cfg, 101325.0f, t);

    for (int i = 1; i <= 100; i++)
    {
        t = (int64_t)i * 10000;
        ekf_predict(&state, &cfg, 0.0f, t);
        if (i % 10 == 0)
        {
            ekf_update_baro(&state, &cfg, 101325.0f, t);
        }
    }

    float alt_before = state.altitude_m;

    t += 10000;
    ekf_predict(&state, &cfg, 0.0f, t);
    ekf_update_baro(&state, &cfg, 35000.0f, t);

    TEST_ASSERT_FLOAT_WITHIN(ALTITUDE_TOL, alt_before, state.altitude_m);
}

void test_ekf_calibrate_sea_level(void)
{
    ekf_cfg_t cfg = default_cfg();
    ekf_state_t state;
    ekf_init(&state, &cfg);

    TEST_ASSERT_EQUAL(ESP_OK, ekf_calibrate(&cfg, &state, 0.0f, 101325.0f));
    TEST_ASSERT_FLOAT_WITHIN(50.0f, 101325.0f, cfg.reference_pressure_pa);
}

void test_ekf_calibrate_500m(void)
{
    ekf_cfg_t cfg = default_cfg();
    ekf_state_t state;
    ekf_init(&state, &cfg);

    TEST_ASSERT_EQUAL(ESP_OK, ekf_calibrate(&cfg, &state, 500.0f, 95461.0f));
    TEST_ASSERT_FLOAT_WITHIN(100.0f, 101325.0f, cfg.reference_pressure_pa);
}

void test_ekf_calibrate_rejects_altitude_too_low(void)
{
    ekf_cfg_t cfg = default_cfg();
    ekf_state_t state;
    float p0_before = cfg.reference_pressure_pa;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ekf_calibrate(&cfg, &state, -600.0f, 101325.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, p0_before, cfg.reference_pressure_pa);
}

void test_ekf_calibrate_rejects_altitude_too_high(void)
{
    ekf_cfg_t cfg = default_cfg();
    ekf_state_t state;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ekf_calibrate(&cfg, &state, 11000.0f, 101325.0f));
}

void test_ekf_calibrate_rejects_pressure_too_low(void)
{
    ekf_cfg_t cfg = default_cfg();
    ekf_state_t state;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ekf_calibrate(&cfg, &state, 0.0f, 15000.0f));
}

void test_ekf_calibrate_rejects_pressure_too_high(void)
{
    ekf_cfg_t cfg = default_cfg();
    ekf_state_t state;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ekf_calibrate(&cfg, &state, 0.0f, 130000.0f));
}

void test_ekf_calibrate_resets_filter_state(void)
{
    ekf_cfg_t cfg = default_cfg();
    ekf_state_t state;
    ekf_init(&state, &cfg);
    state.altitude_m = 100.0f;
    state.vario_ms = 5.0f;
    state.initialized = true;

    ekf_calibrate(&cfg, &state, 0.0f, 101325.0f);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_TOL, 0.0f, state.altitude_m);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_TOL, 0.0f, state.vario_ms);
    TEST_ASSERT_FALSE(state.initialized);
}
