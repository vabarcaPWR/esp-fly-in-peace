#include "piezo_hardware.h"
#include "driver/ledc.h"
#include "esp_log.h"

#define PIEZO_LEDC_TIMER LEDC_TIMER_0
#define PIEZO_LEDC_CHANNEL LEDC_CHANNEL_0
#define PIEZO_LEDC_SPEED LEDC_LOW_SPEED_MODE
#define PIEZO_LEDC_RESOLUTION LEDC_TIMER_13_BIT
#define PIEZO_MAX_DUTY ((1 << 13) - 1)
#define PIEZO_MIN_FREQ_HZ 100U
#define PIEZO_MAX_FREQ_HZ 4000U

static const char *TAG = "piezo_hw";
typedef struct piezo_hw_context_s
{
    bool initialized;
    uint8_t gpio;
} piezo_hw_context_t;

static piezo_hw_context_t s_self;

esp_err_t piezo_hw_init(uint8_t gpio_num)
{
    if (s_self.initialized)
        return ESP_OK;

    ledc_timer_config_t timer_cfg = {
        .speed_mode = PIEZO_LEDC_SPEED,
        .duty_resolution = PIEZO_LEDC_RESOLUTION,
        .timer_num = PIEZO_LEDC_TIMER,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    esp_err_t ret = ledc_timer_config(&timer_cfg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "timer config failed: 0x%x", ret);
        return ret;
    }

    ledc_channel_config_t ch_cfg = {
        .speed_mode = PIEZO_LEDC_SPEED,
        .channel = PIEZO_LEDC_CHANNEL,
        .timer_sel = PIEZO_LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = gpio_num,
        .duty = 0,
        .hpoint = 0,
    };

    ret = ledc_channel_config(&ch_cfg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "channel config failed: 0x%x", ret);
        return ret;
    }

    s_self.gpio = gpio_num;
    s_self.initialized = true;
    return ESP_OK;
}

esp_err_t piezo_hw_set_tone(uint16_t freq_hz, uint8_t duty_pct)
{
    if (!s_self.initialized)
        return ESP_ERR_INVALID_STATE;

    if (freq_hz == 0 || duty_pct == 0)
    {
        piezo_hw_mute();
        return ESP_OK;
    }

    if (freq_hz < PIEZO_MIN_FREQ_HZ)
        freq_hz = PIEZO_MIN_FREQ_HZ;
    if (freq_hz > PIEZO_MAX_FREQ_HZ)
        freq_hz = PIEZO_MAX_FREQ_HZ;
    if (duty_pct > 100)
        duty_pct = 100;

    esp_err_t ret = ledc_set_freq(PIEZO_LEDC_SPEED, PIEZO_LEDC_TIMER, freq_hz);
    if (ret != ESP_OK)
        return ret;

    uint32_t duty = (uint32_t)PIEZO_MAX_DUTY * duty_pct / 100;
    ret = ledc_set_duty(PIEZO_LEDC_SPEED, PIEZO_LEDC_CHANNEL, duty);
    if (ret != ESP_OK)
        return ret;

    return ledc_update_duty(PIEZO_LEDC_SPEED, PIEZO_LEDC_CHANNEL);
}

void piezo_hw_mute(void)
{
    if (!s_self.initialized)
        return;

    ledc_set_duty(PIEZO_LEDC_SPEED, PIEZO_LEDC_CHANNEL, 0);
    ledc_update_duty(PIEZO_LEDC_SPEED, PIEZO_LEDC_CHANNEL);
}
