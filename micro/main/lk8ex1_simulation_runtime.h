#ifndef LK8EX1_SIMULATION_RUNTIME_H
#define LK8EX1_SIMULATION_RUNTIME_H

#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    esp_err_t lk8ex1_simulation_runtime_init(void);
    void lk8ex1_simulation_ble_rx_callback(const uint8_t *data, uint16_t len);
    void lk8ex1_simulation_sender_task(void *param);

#ifdef __cplusplus
}
#endif

#endif
