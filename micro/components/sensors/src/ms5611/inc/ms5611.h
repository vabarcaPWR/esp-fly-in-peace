
#ifndef MS5611_H
#define MS5611_H

#include "sensor.h"

#ifdef __cplusplus
extern "C"
{
#endif

    const sensor_baro_t *get_ms5611_sensor(void);

#ifdef __cplusplus
}
#endif

#endif // MS5611_H
