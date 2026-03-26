
#ifndef MPU6050_H
#define MPU6050_H

#include "sensor.h"

#ifdef __cplusplus
extern "C"
{
#endif

    const sensor_imu_t *get_mpu6050_sensor(void);

#ifdef __cplusplus
}
#endif

#endif // MPU6050_H
