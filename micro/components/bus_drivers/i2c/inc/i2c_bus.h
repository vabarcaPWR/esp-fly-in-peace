#ifndef I2C_BUS_H
#define I2C_BUS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "esp_err.h"

#if __has_include("driver/i2c_master.h")
#include "driver/i2c_master.h"
    i2c_master_bus_handle_t sensor_i2c_bus_get_handle(void);
#endif

    esp_err_t sensor_i2c_bus_init(void);

#ifdef __cplusplus
}
#endif

#endif
