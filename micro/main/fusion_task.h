#ifndef FUSION_TASK_H
#define FUSION_TASK_H

#include "sensor.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct fusion_task_ctx_s
    {
        const sensor_imu_t *imu;
    } fusion_task_ctx_t;

    void fusion_task_fn(void *param);

#ifdef __cplusplus
}
#endif

#endif
