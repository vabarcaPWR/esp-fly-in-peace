#include "ahrs.h"
#include "unity.h"

#include <math.h>
#include <string.h>

TEST_SOURCE_FILE("../components/ahrs/src/ahrs.c")

#define FLOAT_TOL 0.05f
#define ACCEL_TOL 0.15f

static ahrs_cfg_t default_cfg(void)
{
    ahrs_cfg_t cfg = {
        .beta = AHRS_DEFAULT_BETA,
        .sample_rate_hz = AHRS_DEFAULT_SAMPLE_RATE,
    };
    return cfg;
}

static data_imu_t stationary_imu(void)
{
    data_imu_t imu = {0};
    imu.accel_x = 0.0f;
    imu.accel_y = 0.0f;
    imu.accel_z = -AHRS_GRAVITY_MS2;
    imu.gyro_x = 0.0f;
    imu.gyro_y = 0.0f;
    imu.gyro_z = 0.0f;
    imu.timestamp_us = 0;
    return imu;
}

void setUp(void)
{
}
void tearDown(void)
{
}

void test_ahrs_init_null_state_returns_invalid_arg(void)
{
    ahrs_cfg_t cfg = default_cfg();
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ahrs_init(NULL, &cfg));
}

void test_ahrs_init_null_cfg_returns_invalid_arg(void)
{
    ahrs_state_t state;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ahrs_init(&state, NULL));
}

void test_ahrs_update_null_state_returns_invalid_arg(void)
{
    ahrs_cfg_t cfg = default_cfg();
    data_imu_t imu = stationary_imu();
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ahrs_update(NULL, &cfg, &imu));
}

void test_ahrs_update_null_imu_returns_invalid_arg(void)
{
    ahrs_state_t state;
    ahrs_cfg_t cfg = default_cfg();
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ahrs_update(&state, &cfg, NULL));
}

void test_ahrs_reset_null_state_returns_invalid_arg(void)
{
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ahrs_reset(NULL));
}

void test_ahrs_get_vertical_accel_null_state_returns_invalid_arg(void)
{
    data_imu_t imu = stationary_imu();
    float v;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ahrs_get_vertical_accel(NULL, &imu, &v));
}

void test_ahrs_get_vertical_accel_null_imu_returns_invalid_arg(void)
{
    ahrs_state_t state;
    float v;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ahrs_get_vertical_accel(&state, NULL, &v));
}

void test_ahrs_get_vertical_accel_null_output_returns_invalid_arg(void)
{
    ahrs_state_t state;
    data_imu_t imu = stationary_imu();
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ahrs_get_vertical_accel(&state, &imu, NULL));
}

void test_ahrs_init_sets_quaternion_to_identity(void)
{
    ahrs_state_t state;
    ahrs_cfg_t cfg = default_cfg();
    TEST_ASSERT_EQUAL(ESP_OK, ahrs_init(&state, &cfg));
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_TOL, 1.0f, state.q[0]);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_TOL, 0.0f, state.q[1]);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_TOL, 0.0f, state.q[2]);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_TOL, 0.0f, state.q[3]);
}

void test_ahrs_init_sets_initialized_flag(void)
{
    ahrs_state_t state;
    ahrs_cfg_t cfg = default_cfg();
    ahrs_init(&state, &cfg);
    TEST_ASSERT_TRUE(state.initialized);
}

void test_ahrs_init_sets_identity_rotation_matrix(void)
{
    ahrs_state_t state;
    ahrs_cfg_t cfg = default_cfg();
    ahrs_init(&state, &cfg);
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            TEST_ASSERT_FLOAT_WITHIN(FLOAT_TOL, (i == j) ? 1.0f : 0.0f, state.r[i][j]);
}

void test_ahrs_reset_clears_initialized(void)
{
    ahrs_state_t state;
    ahrs_cfg_t cfg = default_cfg();
    ahrs_init(&state, &cfg);
    TEST_ASSERT_EQUAL(ESP_OK, ahrs_reset(&state));
    TEST_ASSERT_FALSE(state.initialized);
}

void test_ahrs_reset_sets_quaternion_to_identity(void)
{
    ahrs_state_t state;
    ahrs_cfg_t cfg = default_cfg();
    ahrs_init(&state, &cfg);
    state.q[0] = 0.5f;
    state.q[1] = 0.5f;
    ahrs_reset(&state);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_TOL, 1.0f, state.q[0]);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_TOL, 0.0f, state.q[1]);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_TOL, 0.0f, state.q[2]);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_TOL, 0.0f, state.q[3]);
}

void test_ahrs_update_not_initialized_returns_invalid_state(void)
{
    ahrs_state_t state;
    memset(&state, 0, sizeof(state));
    state.initialized = false;
    ahrs_cfg_t cfg = default_cfg();
    data_imu_t imu = stationary_imu();
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE, ahrs_update(&state, &cfg, &imu));
}

void test_ahrs_stationary_quaternion_stays_near_identity(void)
{
    ahrs_state_t state;
    ahrs_cfg_t cfg = default_cfg();
    ahrs_init(&state, &cfg);
    data_imu_t imu = stationary_imu();

    for (int i = 0; i < 500; i++)
    {
        imu.timestamp_us = (int64_t)i * 10000;
        ahrs_update(&state, &cfg, &imu);
    }

    TEST_ASSERT_FLOAT_WITHIN(FLOAT_TOL, 1.0f, state.q[0]);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_TOL, 0.0f, state.q[1]);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_TOL, 0.0f, state.q[2]);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_TOL, 0.0f, state.q[3]);
}

void test_ahrs_get_vertical_accel_not_initialized_returns_invalid_state(void)
{
    ahrs_state_t state;
    memset(&state, 0, sizeof(state));
    state.initialized = false;
    data_imu_t imu = stationary_imu();
    float v;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE, ahrs_get_vertical_accel(&state, &imu, &v));
}

void test_ahrs_vertical_accel_at_rest_is_zero(void)
{
    ahrs_state_t state;
    ahrs_cfg_t cfg = default_cfg();
    ahrs_init(&state, &cfg);
    data_imu_t imu = stationary_imu();

    for (int i = 0; i < 200; i++)
    {
        imu.timestamp_us = (int64_t)i * 10000;
        ahrs_update(&state, &cfg, &imu);
    }

    float v;
    TEST_ASSERT_EQUAL(ESP_OK, ahrs_get_vertical_accel(&state, &imu, &v));
    TEST_ASSERT_FLOAT_WITHIN(ACCEL_TOL, 0.0f, v);
}

void test_ahrs_vertical_accel_with_30deg_roll_is_near_zero(void)
{
    ahrs_state_t state;
    ahrs_cfg_t cfg = default_cfg();
    ahrs_init(&state, &cfg);

    float roll_rad = 30.0f * 3.14159265f / 180.0f;
    data_imu_t imu = {0};
    imu.accel_x = AHRS_GRAVITY_MS2 * sinf(roll_rad);
    imu.accel_y = 0.0f;
    imu.accel_z = -AHRS_GRAVITY_MS2 * cosf(roll_rad);
    imu.gyro_x = 0.0f;
    imu.gyro_y = 0.0f;
    imu.gyro_z = 0.0f;

    for (int i = 0; i < 2000; i++)
    {
        imu.timestamp_us = (int64_t)i * 10000;
        ahrs_update(&state, &cfg, &imu);
    }

    float v;
    TEST_ASSERT_EQUAL(ESP_OK, ahrs_get_vertical_accel(&state, &imu, &v));
    TEST_ASSERT_FLOAT_WITHIN(ACCEL_TOL, 0.0f, v);
}

void test_ahrs_known_rotation_converges(void)
{
    ahrs_state_t state;
    ahrs_cfg_t cfg = default_cfg();
    ahrs_init(&state, &cfg);

    data_imu_t imu = {0};
    imu.accel_x = -AHRS_GRAVITY_MS2;
    imu.accel_y = 0.0f;
    imu.accel_z = 0.0f;

    for (int i = 0; i < 5000; i++)
    {
        imu.timestamp_us = (int64_t)i * 10000;
        ahrs_update(&state, &cfg, &imu);
    }

    TEST_ASSERT_FLOAT_WITHIN(0.15f, 1.0f, state.r[2][0]);
}
