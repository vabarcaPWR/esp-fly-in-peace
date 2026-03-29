#ifndef BMP390_H
#define BMP390_H

#include "bmp390_model.h"
#include "sensor.h"

#ifdef __cplusplus
extern "C"
{
#endif

    const sensor_baro_t *get_bmp390_sensor(void);

#ifdef __cplusplus
}
#endif

#endif // BMP390_H
