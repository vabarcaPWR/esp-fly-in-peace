#include "sensor.h"
#include "unity.h"

TEST_SOURCE_FILE("../test/fakes/fake_ms5611.c")
TEST_SOURCE_FILE("../components/sensors/src/sensor.c")
TEST_SOURCE_FILE("../test/fakes/fake_mpu6050.c")

void setUp(void) {}
void tearDown(void) {}

void test_get_ms5611_sensor_returns_valid_driver(void)
{
    const sensor_baro_t *sensor = get_baro_sensor("ms5611");
    TEST_ASSERT_NOT_NULL(sensor);
    TEST_ASSERT_NOT_NULL(sensor->init);
    TEST_ASSERT_NOT_NULL(sensor->read);
    TEST_ASSERT_NOT_NULL(sensor->get_name);
    TEST_ASSERT_EQUAL(ESP_OK, sensor->init());
}

void test_get_baro_sensor_returns_null_for_unknown_sensor(void)
{
    const sensor_baro_t *sensor = get_baro_sensor("unknown");
    TEST_ASSERT_NULL(sensor);
}

void test_ms5611_read_accepts_non_null_out(void)
{
    const sensor_baro_t *sensor = get_baro_sensor("ms5611");
    data_baro_t data = {0};
    TEST_ASSERT_NOT_NULL(sensor);
    TEST_ASSERT_EQUAL(ESP_OK, sensor->read(&data));
}
