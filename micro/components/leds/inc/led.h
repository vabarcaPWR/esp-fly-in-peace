#ifndef LED_H
#define LED_H

#include "esp_err.h"
#include <stdint.h>

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
        LED_STATE_COUNT
    } led_state_e;

    typedef struct led_s
    {
        esp_err_t (*init)(void);
        esp_err_t (*set_state)(led_state_e state);
        led_state_e (*get_state)(void);
        const char *(*get_name)(void);
    } led_t;

    const led_t *get_led(const char *led_name);

#ifdef __cplusplus
}
#endif

#endif // LED_H
