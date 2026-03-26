#include "sensor.h"
#include "unity.h"

#include <string.h>

TEST_SOURCE_FILE("../components/sensors/src/mpu6050/src/mpu6050.c")
TEST_SOURCE_FILE("../components/sensors/src/ms5611/src/ms5611.c")
TEST_SOURCE_FILE("../components/sensors/src/sensor.c")

void setUp(void) {}
void tearDown(void) {}

void test_get_imu_sensor_mpu6050_returns_valid_backend(void)
{
    const sensor_imu_t *sensor = get_imu_sensor("MPU6050");
    TEST_ASSERT_NOT_NULL(sensor);
    TEST_ASSERT_NOT_NULL(sensor->init);
    TEST_ASSERT_NOT_NULL(sensor->read);
    TEST_ASSERT_NOT_NULL(sensor->get_name);
}

void test_get_imu_sensor_null_returns_null(void)
{
    TEST_ASSERT_NULL(get_imu_sensor(NULL));
}

void test_get_imu_sensor_unknown_returns_null(void)
{
    TEST_ASSERT_NULL(get_imu_sensor("unknown"));
}

void test_mpu6050_get_name_returns_mpu6050(void)
{
    const sensor_imu_t *sensor = get_imu_sensor("MPU6050");
    TEST_ASSERT_NOT_NULL(sensor);
    TEST_ASSERT_EQUAL_STRING("MPU6050", sensor->get_name());
}

void test_mpu6050_read_null_returns_invalid_arg(void)
{
    const sensor_imu_t *sensor = get_imu_sensor("MPU6050");
    TEST_ASSERT_NOT_NULL(sensor);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, sensor->read(NULL));
}

void test_mpu6050_read_before_init_returns_invalid_state(void)
{
    const sensor_imu_t *sensor = get_imu_sensor("MPU6050");
    TEST_ASSERT_NOT_NULL(sensor);
    data_imu_t data = {0};
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE, sensor->read(&data));
}

void test_mpu6050_init_succeeds(void)
{
    const sensor_imu_t *sensor = get_imu_sensor("MPU6050");
    TEST_ASSERT_NOT_NULL(sensor);
    TEST_ASSERT_EQUAL(ESP_OK, sensor->init());
}

void test_mpu6050_read_after_init_succeeds(void)
{
    const sensor_imu_t *sensor = get_imu_sensor("MPU6050");
    TEST_ASSERT_NOT_NULL(sensor);
    TEST_ASSERT_EQUAL(ESP_OK, sensor->init());

    data_imu_t data = {0};
    TEST_ASSERT_EQUAL(ESP_OK, sensor->read(&data));
}

void test_mpu6050_read_fills_accel_fields(void)
{
    const sensor_imu_t *sensor = get_imu_sensor("MPU6050");
    TEST_ASSERT_NOT_NULL(sensor);
    TEST_ASSERT_EQUAL(ESP_OK, sensor->init());

    data_imu_t data;
    memset(&data, 0xFF, sizeof(data));
    TEST_ASSERT_EQUAL(ESP_OK, sensor->read(&data));

    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, data.accel_x);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, data.accel_y);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, -9.81f, data.accel_z);
}

void test_mpu6050_read_fills_gyro_fields(void)
{
    const sensor_imu_t *sensor = get_imu_sensor("MPU6050");
    TEST_ASSERT_NOT_NULL(sensor);
    TEST_ASSERT_EQUAL(ESP_OK, sensor->init());

    data_imu_t data;
    memset(&data, 0xFF, sizeof(data));
    TEST_ASSERT_EQUAL(ESP_OK, sensor->read(&data));

    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, data.gyro_x);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, data.gyro_y);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, data.gyro_z);
}

void test_mpu6050_read_fills_timestamp(void)
{
    const sensor_imu_t *sensor = get_imu_sensor("MPU6050");
    TEST_ASSERT_NOT_NULL(sensor);
    TEST_ASSERT_EQUAL(ESP_OK, sensor->init());

    data_imu_t data = {0};
    TEST_ASSERT_EQUAL(ESP_OK, sensor->read(&data));
    TEST_ASSERT_EQUAL_INT64(0, data.timestamp_us);
}

void test_baro_factory_still_works_alongside_imu(void)
{
    const sensor_baro_t *baro = get_baro_sensor("ms5611");
    TEST_ASSERT_NOT_NULL(baro);
    TEST_ASSERT_EQUAL(ESP_OK, baro->init());

    const sensor_imu_t *imu = get_imu_sensor("MPU6050");
    TEST_ASSERT_NOT_NULL(imu);
    TEST_ASSERT_EQUAL(ESP_OK, imu->init());
}
