#include "max98357_hardware.h"
#include "driver/gpio.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include <string.h>

#define MAX98357_SAMPLE_RATE 16000
#define MAX98357_DMA_DESC_NUM 4
#define MAX98357_DMA_FRAME_NUM 256
#define MAX98357_WRITE_TIMEOUT_MS 100
#define MAX98357_SILENCE_SAMPLES 256

static const char *TAG = "max98357_hw";
typedef struct max98357_hw_context_s
{
    bool initialized;
    i2s_chan_handle_t tx_chan;
    uint8_t sd_gpio;
    int16_t silence_buf[MAX98357_SILENCE_SAMPLES];
} max98357_hw_context_t;

static max98357_hw_context_t s_self;

esp_err_t max98357_hw_init(const max98357_gpio_cfg_t *cfg)
{
    if (!cfg)
        return ESP_ERR_INVALID_ARG;

    if (s_self.initialized)
        return ESP_OK;

    gpio_config_t sd_cfg = {
        .pin_bit_mask = (1ULL << cfg->sd),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t ret = gpio_config(&sd_cfg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "SD GPIO config failed: 0x%x", ret);
        return ret;
    }
    s_self.sd_gpio = cfg->sd;
    gpio_set_level(s_self.sd_gpio, 1);

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.dma_desc_num = MAX98357_DMA_DESC_NUM;
    chan_cfg.dma_frame_num = MAX98357_DMA_FRAME_NUM;

    ret = i2s_new_channel(&chan_cfg, &s_self.tx_chan, NULL);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2S channel create failed: 0x%x", ret);
        return ret;
    }

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(MAX98357_SAMPLE_RATE),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg =
            {
                .mclk = I2S_GPIO_UNUSED,
                .bclk = cfg->bclk,
                .ws = cfg->ws,
                .dout = cfg->dout,
                .din = I2S_GPIO_UNUSED,
                .invert_flags =
                    {
                        .mclk_inv = false,
                        .bclk_inv = false,
                        .ws_inv = false,
                    },
            },
    };

    ret = i2s_channel_init_std_mode(s_self.tx_chan, &std_cfg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2S std mode init failed: 0x%x", ret);
        i2s_del_channel(s_self.tx_chan);
        s_self.tx_chan = NULL;
        return ret;
    }

    ret = i2s_channel_enable(s_self.tx_chan);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2S channel enable failed: 0x%x", ret);
        i2s_del_channel(s_self.tx_chan);
        s_self.tx_chan = NULL;
        return ret;
    }

    memset(s_self.silence_buf, 0, sizeof(s_self.silence_buf));
    s_self.initialized = true;
    ESP_LOGI(TAG, "initialized (bclk=%d ws=%d dout=%d sd=%d)", cfg->bclk, cfg->ws, cfg->dout, cfg->sd);
    return ESP_OK;
}

esp_err_t max98357_hw_write(const int16_t *buf, size_t samples)
{
    if (!s_self.initialized || !s_self.tx_chan)
        return ESP_ERR_INVALID_STATE;

    if (!buf || samples == 0)
        return ESP_ERR_INVALID_ARG;

    size_t bytes_written = 0;
    return i2s_channel_write(s_self.tx_chan, buf, samples * sizeof(int16_t), &bytes_written,
                             pdMS_TO_TICKS(MAX98357_WRITE_TIMEOUT_MS));
}

void max98357_hw_mute(void)
{
    if (!s_self.initialized || !s_self.tx_chan)
        return;

    size_t bytes_written = 0;
    i2s_channel_write(s_self.tx_chan, s_self.silence_buf, sizeof(s_self.silence_buf), &bytes_written,
                      pdMS_TO_TICKS(MAX98357_WRITE_TIMEOUT_MS));
}

void max98357_hw_shutdown(bool enable)
{
    if (!s_self.initialized)
        return;

    gpio_set_level(s_self.sd_gpio, enable ? 1 : 0);
}
