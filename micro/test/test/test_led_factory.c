#include "led.h"
#include "unity.h"

TEST_SOURCE_FILE("../components/leds/src/led.c")

void setUp(void) {}
void tearDown(void) {}

void test_get_led_returns_single_backend(void)
{
    const led_t *led = get_led("single");
    TEST_ASSERT_NOT_NULL(led);
    TEST_ASSERT_NOT_NULL(led->init);
    TEST_ASSERT_NOT_NULL(led->set_state);
    TEST_ASSERT_NOT_NULL(led->get_state);
    TEST_ASSERT_NOT_NULL(led->get_name);
}

void test_get_led_returns_null_for_unknown_backend(void)
{
    TEST_ASSERT_NULL(get_led("unknown"));
}

void test_single_backend_state_roundtrip(void)
{
    const led_t *led = get_led("single");
    TEST_ASSERT_NOT_NULL(led);
    TEST_ASSERT_EQUAL(ESP_OK, led->init());
    TEST_ASSERT_EQUAL(ESP_OK, led->set_state(LED_STATE_BLE_CONNECTED));
    TEST_ASSERT_EQUAL(LED_STATE_BLE_CONNECTED, led->get_state());
}

void test_single_backend_rejects_invalid_state(void)
{
    const led_t *led = get_led("single");
    TEST_ASSERT_NOT_NULL(led);
    TEST_ASSERT_EQUAL(ESP_OK, led->init());
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, led->set_state(LED_STATE_COUNT));
}
