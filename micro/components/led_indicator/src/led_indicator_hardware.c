#include "led_indicator_hardware.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"

#define LED_INDICATOR_GPIO GPIO_NUM_8
#define LED_INDICATOR_PIXEL_COUNT 1U
#define LED_INDICATOR_RMT_RESOLUTION_HZ 10000000U

static const char *TAG = "led_hw";

static led_strip_handle_t s_led_strip = NULL;

esp_err_t led_indicator_hardware_init(void)
{
    if (s_led_strip)
        return ESP_ERR_INVALID_STATE;

    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_INDICATOR_GPIO,
        .max_leds = LED_INDICATOR_PIXEL_COUNT,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
        .flags =
            {
                .invert_out = false,
            },
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = LED_INDICATOR_RMT_RESOLUTION_HZ,
        .mem_block_symbols = 64,
        .flags =
            {
                .with_dma = false,
            },
    };

    esp_err_t create_result = led_strip_new_rmt_device(&strip_config, &rmt_config, &s_led_strip);
    if (ESP_OK != create_result)
        return create_result;

    esp_err_t off_result = led_indicator_hardware_off();
    if (ESP_OK != off_result)
    {
        led_strip_del(s_led_strip);
        s_led_strip = NULL;
        return off_result;
    }

    ESP_LOGI(TAG, "LED hardware initialized on GPIO %d", LED_INDICATOR_GPIO);
    return ESP_OK;
}

esp_err_t led_indicator_hardware_deinit(void)
{
    if (!s_led_strip)
        return ESP_ERR_INVALID_STATE;

    led_indicator_hardware_off();
    esp_err_t delete_result = led_strip_del(s_led_strip);
    s_led_strip = NULL;
    return delete_result;
}

esp_err_t led_indicator_hardware_set_rgb(uint8_t red, uint8_t green, uint8_t blue)
{
    if (!s_led_strip)
        return ESP_ERR_INVALID_STATE;

    esp_err_t set_result = led_strip_set_pixel(s_led_strip, 0, red, green, blue);
    if (ESP_OK != set_result)
        return set_result;

    return led_strip_refresh(s_led_strip);
}

esp_err_t led_indicator_hardware_off(void)
{
    if (!s_led_strip)
        return ESP_ERR_INVALID_STATE;

    esp_err_t clear_result = led_strip_clear(s_led_strip);
    if (ESP_OK != clear_result)
        return clear_result;

    return led_strip_refresh(s_led_strip);
}
