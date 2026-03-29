#include "sensor.h"
#include "mock_bmp390.h"
#include "mock_ms5611.h"
#include "mock_mpu6050.h"
#include "unity.h"

TEST_SOURCE_FILE("../components/sensors/src/sensor.c")

static int bmp390_provider_call_count;
static int ms5611_provider_call_count;
static int mpu6050_provider_call_count;

static esp_err_t stub_baro_init(void)
{
    return ESP_OK;
}

static esp_err_t stub_baro_read(data_baro_t *out)
{
    (void)out;
    return ESP_OK;
}

static const char *stub_baro_get_name(void)
{
    return "MS5611";
}

static const sensor_baro_t *stub_get_ms5611_sensor(int cmock_num_calls)
{
    (void)cmock_num_calls;
    ms5611_provider_call_count++;

    static const sensor_baro_t sensor = {
        .init = stub_baro_init,
        .read = stub_baro_read,
        .get_name = stub_baro_get_name,
    };

    return &sensor;
}

static const char *stub_bmp390_get_name(void)
{
    return "BMP390";
}

static const sensor_baro_t *stub_get_bmp390_sensor(int cmock_num_calls)
{
    (void)cmock_num_calls;
    bmp390_provider_call_count++;

    static const sensor_baro_t sensor = {
        .init = stub_baro_init,
        .read = stub_baro_read,
        .get_name = stub_bmp390_get_name,
    };

    return &sensor;
}

static esp_err_t stub_imu_init(void)
{
    return ESP_OK;
}

static esp_err_t stub_imu_read(data_imu_t *out)
{
    (void)out;
    return ESP_OK;
}

static const char *stub_imu_get_name(void)
{
    return "MPU6050";
}

static const sensor_imu_t *stub_get_mpu6050_sensor(int cmock_num_calls)
{
    (void)cmock_num_calls;
    mpu6050_provider_call_count++;

    static const sensor_imu_t sensor = {
        .init = stub_imu_init,
        .read = stub_imu_read,
        .get_name = stub_imu_get_name,
    };

    return &sensor;
}

void setUp(void)
{
    bmp390_provider_call_count = 0;
    ms5611_provider_call_count = 0;
    mpu6050_provider_call_count = 0;
    get_bmp390_sensor_StubWithCallback(stub_get_bmp390_sensor);
    get_ms5611_sensor_StubWithCallback(stub_get_ms5611_sensor);
    get_mpu6050_sensor_StubWithCallback(stub_get_mpu6050_sensor);
}

void tearDown(void)
{
}

void test_returns_null_when_baro_name_is_null(void)
{
    TEST_ASSERT_NULL(get_baro_sensor(NULL));
}

void test_does_not_query_ms5611_when_baro_name_is_null(void)
{
    (void)get_baro_sensor(NULL);
    TEST_ASSERT_EQUAL_INT(0, ms5611_provider_call_count);
}

void test_returns_ms5611_backend_when_baro_name_is_ms5611(void)
{
    TEST_ASSERT_EQUAL_PTR(stub_get_ms5611_sensor(0), get_baro_sensor("ms5611"));
}

void test_queries_ms5611_once_when_baro_name_is_ms5611(void)
{
    (void)get_baro_sensor("ms5611");
    TEST_ASSERT_EQUAL_INT(1, ms5611_provider_call_count);
}

void test_returns_null_when_baro_name_is_unknown(void)
{
    TEST_ASSERT_NULL(get_baro_sensor("unknown"));
}

void test_does_not_query_ms5611_when_baro_name_is_unknown(void)
{
    (void)get_baro_sensor("unknown");
    TEST_ASSERT_EQUAL_INT(0, ms5611_provider_call_count);
}

void test_returns_null_when_imu_name_is_null(void)
{
    TEST_ASSERT_NULL(get_imu_sensor(NULL));
}

void test_does_not_query_mpu6050_when_imu_name_is_null(void)
{
    (void)get_imu_sensor(NULL);
    TEST_ASSERT_EQUAL_INT(0, mpu6050_provider_call_count);
}

void test_returns_mpu6050_backend_when_imu_name_is_mpu6050(void)
{
    TEST_ASSERT_EQUAL_PTR(stub_get_mpu6050_sensor(0), get_imu_sensor("MPU6050"));
}

void test_queries_mpu6050_once_when_imu_name_is_mpu6050(void)
{
    (void)get_imu_sensor("MPU6050");
    TEST_ASSERT_EQUAL_INT(1, mpu6050_provider_call_count);
}

void test_returns_null_when_imu_name_is_unknown(void)
{
    TEST_ASSERT_NULL(get_imu_sensor("unknown"));
}

void test_does_not_query_mpu6050_when_imu_name_is_unknown(void)
{
    (void)get_imu_sensor("unknown");
    TEST_ASSERT_EQUAL_INT(0, mpu6050_provider_call_count);
}

void test_returns_bmp390_backend_when_baro_name_is_bmp390(void)
{
    TEST_ASSERT_EQUAL_PTR(stub_get_bmp390_sensor(0), get_baro_sensor("bmp390"));
}

void test_queries_bmp390_once_when_baro_name_is_bmp390(void)
{
    (void)get_baro_sensor("bmp390");
    TEST_ASSERT_EQUAL_INT(1, bmp390_provider_call_count);
}

void test_does_not_query_bmp390_when_baro_name_is_unknown(void)
{
    (void)get_baro_sensor("unknown");
    TEST_ASSERT_EQUAL_INT(0, bmp390_provider_call_count);
}

void test_does_not_query_bmp390_when_baro_name_is_ms5611(void)
{
    (void)get_baro_sensor("ms5611");
    TEST_ASSERT_EQUAL_INT(0, bmp390_provider_call_count);
}
