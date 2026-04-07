#include "unity.h"
#include "led_rgb_model.h"

void setUp(void) {}
void tearDown(void) {}

void test_boot_state_returns_blue(void)
{
    led_rgb_pattern_t p = led_rgb_pattern_for_state(LED_STATE_BOOT);
    TEST_ASSERT_EQUAL_UINT8(0, p.color.r);
    TEST_ASSERT_EQUAL_UINT8(0, p.color.g);
    TEST_ASSERT_GREATER_THAN_UINT8(0, p.color.b);
}

void test_boot_pattern_is_solid(void)
{
    led_rgb_pattern_t p = led_rgb_pattern_for_state(LED_STATE_BOOT);
    TEST_ASSERT_EQUAL_UINT16(p.on_ticks, p.period_ticks);
}

void test_disconnected_state_returns_blue(void)
{
    led_rgb_pattern_t p = led_rgb_pattern_for_state(LED_STATE_BLE_DISCONNECTED);
    TEST_ASSERT_EQUAL_UINT8(0, p.color.r);
    TEST_ASSERT_EQUAL_UINT8(0, p.color.g);
    TEST_ASSERT_GREATER_THAN_UINT8(0, p.color.b);
}

void test_disconnected_blinks_slow(void)
{
    led_rgb_pattern_t p = led_rgb_pattern_for_state(LED_STATE_BLE_DISCONNECTED);
    TEST_ASSERT_LESS_THAN_UINT16(p.period_ticks, p.on_ticks);
    TEST_ASSERT_GREATER_THAN_UINT16(100, p.period_ticks);
}

void test_connected_state_returns_green(void)
{
    led_rgb_pattern_t p = led_rgb_pattern_for_state(LED_STATE_BLE_CONNECTED);
    TEST_ASSERT_EQUAL_UINT8(0, p.color.r);
    TEST_ASSERT_GREATER_THAN_UINT8(0, p.color.g);
    TEST_ASSERT_EQUAL_UINT8(0, p.color.b);
}

void test_connected_blinks_slower_than_disconnected(void)
{
    led_rgb_pattern_t disc = led_rgb_pattern_for_state(LED_STATE_BLE_DISCONNECTED);
    led_rgb_pattern_t conn = led_rgb_pattern_for_state(LED_STATE_BLE_CONNECTED);
    TEST_ASSERT_GREATER_THAN_UINT16(disc.period_ticks, conn.period_ticks);
}

void test_wifi_state_returns_cyan(void)
{
    led_rgb_pattern_t p = led_rgb_pattern_for_state(LED_STATE_WIFI_ENABLED);
    TEST_ASSERT_EQUAL_UINT8(0, p.color.r);
    TEST_ASSERT_GREATER_THAN_UINT8(0, p.color.g);
    TEST_ASSERT_GREATER_THAN_UINT8(0, p.color.b);
}

void test_error_state_returns_red(void)
{
    led_rgb_pattern_t p = led_rgb_pattern_for_state(LED_STATE_ERROR);
    TEST_ASSERT_GREATER_THAN_UINT8(0, p.color.r);
    TEST_ASSERT_EQUAL_UINT8(0, p.color.g);
    TEST_ASSERT_EQUAL_UINT8(0, p.color.b);
}

void test_error_blinks_faster_than_disconnected(void)
{
    led_rgb_pattern_t err = led_rgb_pattern_for_state(LED_STATE_ERROR);
    led_rgb_pattern_t disc = led_rgb_pattern_for_state(LED_STATE_BLE_DISCONNECTED);
    TEST_ASSERT_LESS_THAN_UINT16(disc.period_ticks, err.period_ticks);
}

void test_invalid_state_returns_off(void)
{
    led_rgb_pattern_t p = led_rgb_pattern_for_state(LED_STATE_COUNT);
    TEST_ASSERT_EQUAL_UINT8(0, p.color.r);
    TEST_ASSERT_EQUAL_UINT8(0, p.color.g);
    TEST_ASSERT_EQUAL_UINT8(0, p.color.b);
}

void test_all_valid_states_have_nonzero_period(void)
{
    for (int s = LED_STATE_BOOT; s < LED_STATE_COUNT; s++)
    {
        led_rgb_pattern_t p = led_rgb_pattern_for_state((led_state_e)s);
        TEST_ASSERT_GREATER_THAN_UINT16(0, p.period_ticks);
    }
}
