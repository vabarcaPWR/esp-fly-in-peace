#ifndef LED_INDICATOR_H
#define LED_INDICATOR_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum led_state_e
    {
        LED_STATE_BOOT = 0,
        LED_STATE_BLE_DISCONNECTED,
        LED_STATE_BLE_CONNECTED,
        LED_STATE_WIFI_ENABLED,
        LED_STATE_ERROR,
    } led_state_e;

    esp_err_t led_indicator_init(void);
    esp_err_t led_indicator_set_state(led_state_e state);
    led_state_e led_indicator_get_state(void);

#ifdef __cplusplus
}
#endif

#endif
