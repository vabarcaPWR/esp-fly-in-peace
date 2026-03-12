#include "led.h"
#include "mock_led_single.h"
#include "unity.h"

TEST_SOURCE_FILE("../components/leds/src/led.c")

static int single_led_provider_call_count;

static esp_err_t fake_led_init(void)
{
    return ESP_OK;
}

static esp_err_t fake_led_set_state(led_state_e state)
{
    (void)state;
    return ESP_OK;
}

static led_state_e fake_led_get_state(void)
{
    return LED_STATE_BOOT;
}

static const char *fake_led_get_name(void)
{
    return "single";
}

static const led_t *stub_get_single_led(int cmock_num_calls)
{
    (void)cmock_num_calls;
    single_led_provider_call_count++;

    static const led_t fake_led = {
        .init = fake_led_init,
        .set_state = fake_led_set_state,
        .get_state = fake_led_get_state,
        .get_name = fake_led_get_name,
    };

    return &fake_led;
}

void setUp(void)
{
    single_led_provider_call_count = 0;
    get_single_led_StubWithCallback(stub_get_single_led);
}

void tearDown(void)
{
}

void test_get_led_when_name_is_null_returns_null(void)
{
    TEST_ASSERT_NULL(get_led(NULL));
}

void test_get_led_when_name_is_null_does_not_query_single_backend(void)
{
    (void)get_led(NULL);
    TEST_ASSERT_EQUAL_INT(0, single_led_provider_call_count);
}

void test_get_led_when_name_is_single_returns_single_backend(void)
{
    TEST_ASSERT_EQUAL_PTR(stub_get_single_led(0), get_led("single"));
}

void test_get_led_when_name_is_single_queries_single_backend_once(void)
{
    (void)get_led("single");
    TEST_ASSERT_EQUAL_INT(1, single_led_provider_call_count);
}

void test_get_led_when_name_is_unknown_returns_null(void)
{
    TEST_ASSERT_NULL(get_led("unknown"));
}

void test_get_led_when_name_is_unknown_does_not_query_single_backend(void)
{
    (void)get_led("unknown");
    TEST_ASSERT_EQUAL_INT(0, single_led_provider_call_count);
}
