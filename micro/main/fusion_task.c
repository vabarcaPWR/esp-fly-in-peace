#include "fusion_task.h"

#include "ekf.h"
#include "flight_data.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <esp_timer.h>
#include <stdio.h>

#define FUSION_BARO_PERIOD_MS 100U

static const char *TAG = "fusion";

static void publish_flight_data(const ekf_state_t *ekf, const ekf_cfg_t *ekf_cfg, int32_t pressure_pa,
                                int32_t temperature_mc, bool sensor_ok)
{
    if (xSemaphoreTake(g_flight_data_mutex, pdMS_TO_TICKS(FLIGHT_DATA_MUTEX_TIMEOUT_MS)) != pdTRUE)
        return;

    g_flight_data.altitude_m = ekf->altitude_m;
    g_flight_data.vario_ms = ekf->vario_ms;
    g_flight_data.pressure_pa = pressure_pa;
    g_flight_data.temperature_mc = temperature_mc;
    g_flight_data.reference_pressure_pa = ekf_cfg->reference_pressure_pa;
    g_flight_data.vertical_accel_ms2 = 0.0f;
    g_flight_data.timestamp_us = ekf->last_predict_us;
    g_flight_data.sensor_valid = sensor_ok;
    g_flight_data.imu_valid = false;

    xSemaphoreGive(g_flight_data_mutex);
}

static void handle_calibration_request(ekf_cfg_t *ekf_cfg, ekf_state_t *ekf_state, int32_t last_pressure_pa)
{
    calibration_request_t req;
    if (xQueueReceive(g_calibration_queue, &req, 0) != pdTRUE)
        return;

    esp_err_t ret = ekf_calibrate(ekf_cfg, ekf_state, req.known_altitude_m, (float)last_pressure_pa);
    if (ret == ESP_OK)
        ESP_LOGI(TAG, "calibrated: alt=%.1f m, P0=%.0f Pa", req.known_altitude_m, ekf_cfg->reference_pressure_pa);
    else
        ESP_LOGW(TAG, "calibration failed: 0x%x", ret);
}

void fusion_task_fn(void *param)
{
    (void)param;

    ekf_cfg_t ekf_cfg = {.q_altitude = EKF_DEFAULT_Q_ALTITUDE,
                         .q_vario = EKF_DEFAULT_Q_VARIO,
                         .q_accel_bias = EKF_DEFAULT_Q_ACCEL_BIAS,
                         .r_altitude = EKF_DEFAULT_R_ALTITUDE,
                         .reference_pressure_pa = EKF_DEFAULT_REFERENCE_PA};
    ekf_state_t ekf_state;
    ekf_init(&ekf_state, &ekf_cfg);

    TickType_t last_wake = xTaskGetTickCount();
    int32_t last_pressure_pa = 0;
    int32_t last_temperature_mc = 0;
    bool sensor_valid = false;

    ESP_LOGI(TAG, "baro-only mode");

    while (true)
    {
        handle_calibration_request(&ekf_cfg, &ekf_state, last_pressure_pa);

        data_baro_t baro_data;
        if (xQueueReceive(g_baro_queue, &baro_data, pdMS_TO_TICKS(FUSION_BARO_PERIOD_MS)) == pdTRUE)
        {
            last_pressure_pa = baro_data.pressure_pa;
            last_temperature_mc = baro_data.temperature_mc;
            sensor_valid = true;
            ekf_predict(&ekf_state, &ekf_cfg, 0.0f, baro_data.timestamp_us);
            ekf_update_baro(&ekf_state, &ekf_cfg, (float)baro_data.pressure_pa, baro_data.timestamp_us);

            float raw_alt = ekf_pressure_to_altitude((float)baro_data.pressure_pa, ekf_cfg.reference_pressure_pa);
            printf("\r  P_raw=%6ld Pa\talt_raw=%+8.2fm\talt_ekf=%+8.2fm\tvario=%+6.2fm/s\tT=%5.2fC",
                   (long)baro_data.pressure_pa, raw_alt, ekf_state.altitude_m, ekf_state.vario_ms,
                   (float)last_temperature_mc / 100.0f);
            fflush(stdout);
        }

        publish_flight_data(&ekf_state, &ekf_cfg, last_pressure_pa, last_temperature_mc, sensor_valid);

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(FUSION_BARO_PERIOD_MS));
    }
}
