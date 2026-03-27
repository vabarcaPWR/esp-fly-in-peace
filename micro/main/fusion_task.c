#include "fusion_task.h"

#include "ahrs.h"
#include "ekf.h"
#include "flight_data.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_heap_caps.h>
#include <esp_log.h>
#include <esp_timer.h>

#define FUSION_IMU_PERIOD_MS 10U
#define FUSION_BARO_ONLY_PERIOD_MS 100U
#define FUSION_MAX_CONSECUTIVE_ERRORS 3U
#define DIAG_INTERVAL_CYCLES 500U
#define AHRS_WARMUP_CYCLES 300U

static const char *TAG = "fusion";

static void publish_flight_data(const ekf_state_t *ekf, const ekf_cfg_t *ekf_cfg, float vert_accel, int32_t pressure_pa,
                                int32_t temperature_mc, bool sensor_ok, bool imu_ok)
{
    if (xSemaphoreTake(g_flight_data_mutex, pdMS_TO_TICKS(FLIGHT_DATA_MUTEX_TIMEOUT_MS)) != pdTRUE)
        return;

    g_flight_data.altitude_m = ekf->altitude_m;
    g_flight_data.vario_ms = ekf->vario_ms;
    g_flight_data.pressure_pa = pressure_pa;
    g_flight_data.temperature_mc = temperature_mc;
    g_flight_data.reference_pressure_pa = ekf_cfg->reference_pressure_pa;
    g_flight_data.vertical_accel_ms2 = vert_accel;
    g_flight_data.timestamp_us = ekf->last_predict_us;
    g_flight_data.sensor_valid = sensor_ok;
    g_flight_data.imu_valid = imu_ok;

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

static void run_imu_fusion_loop(const sensor_imu_t *imu)
{
    ahrs_cfg_t ahrs_cfg = {.beta = AHRS_DEFAULT_BETA, .sample_rate_hz = AHRS_DEFAULT_SAMPLE_RATE};
    ekf_cfg_t ekf_cfg = {.q_altitude = EKF_DEFAULT_Q_ALTITUDE,
                         .q_vario = EKF_DEFAULT_Q_VARIO,
                         .q_accel_bias = EKF_DEFAULT_Q_ACCEL_BIAS,
                         .r_altitude = EKF_DEFAULT_R_ALTITUDE,
                         .reference_pressure_pa = EKF_DEFAULT_REFERENCE_PA};
    ahrs_state_t ahrs_state;
    ekf_state_t ekf_state;
    ahrs_init(&ahrs_state, &ahrs_cfg);
    ekf_init(&ekf_state, &ekf_cfg);

    TickType_t last_wake = xTaskGetTickCount();
    uint32_t imu_errors = 0;
    int32_t last_pressure_pa = 0;
    int32_t last_temperature_mc = 0;
    bool sensor_valid = false;
    float vert_accel = 0.0f;
    uint32_t cycle_count = 0;
    uint32_t baro_update_count = 0;
    int64_t diag_start_us = esp_timer_get_time();

    while (true)
    {
        int64_t cycle_start_us = esp_timer_get_time();

        handle_calibration_request(&ekf_cfg, &ekf_state, last_pressure_pa);

        data_imu_t imu_data = {0};
        bool imu_ok = (imu->read(&imu_data) == ESP_OK);
        imu_ok ? imu_errors = 0 : imu_errors++;
        bool imu_valid = (imu_errors < FUSION_MAX_CONSECUTIVE_ERRORS);

        if (imu_ok)
        {
            ahrs_update(&ahrs_state, &ahrs_cfg, &imu_data);
            ahrs_get_vertical_accel(&ahrs_state, &imu_data, &vert_accel);
            if (cycle_count >= AHRS_WARMUP_CYCLES)
                ekf_predict(&ekf_state, &ekf_cfg, vert_accel, imu_data.timestamp_us);
        }

        data_baro_t baro_data;
        if (xQueueReceive(g_baro_queue, &baro_data, 0) == pdTRUE)
        {
            last_pressure_pa = baro_data.pressure_pa;
            last_temperature_mc = baro_data.temperature_mc;
            sensor_valid = true;
            baro_update_count++;
            ekf_update_baro(&ekf_state, &ekf_cfg, (float)baro_data.pressure_pa, baro_data.timestamp_us);
        }

        publish_flight_data(&ekf_state, &ekf_cfg, vert_accel, last_pressure_pa, last_temperature_mc, sensor_valid,
                            imu_valid);

        cycle_count++;
        int64_t cycle_us = esp_timer_get_time() - cycle_start_us;

        if (cycle_count % DIAG_INTERVAL_CYCLES == 0)
        {
            int64_t elapsed_us = esp_timer_get_time() - diag_start_us;
            float imu_hz = (float)cycle_count / ((float)elapsed_us / 1e6f);
            float baro_hz = (float)baro_update_count / ((float)elapsed_us / 1e6f);
            UBaseType_t fusion_stack = uxTaskGetStackHighWaterMark(NULL);
            ESP_LOGI(TAG,
                     "DIAG imu=%.1fHz baro=%.1fHz cycle=%lldus alt=%.1fm vario=%.2fm/s va=%.2f bias=%.3f "
                     "P=%ld stk=%u heap=%lu",
                     imu_hz, baro_hz, (long long)cycle_us, ekf_state.altitude_m, ekf_state.vario_ms, vert_accel,
                     ekf_state.accel_bias_ms2, (long)last_pressure_pa, (unsigned)fusion_stack,
                     (unsigned long)esp_get_free_heap_size());
        }

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(FUSION_IMU_PERIOD_MS));
    }
}

static void run_baro_only_loop(void)
{
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

    ESP_LOGI(TAG, "baro-only mode (no IMU)");

    while (true)
    {
        handle_calibration_request(&ekf_cfg, &ekf_state, last_pressure_pa);

        data_baro_t baro_data;
        if (xQueueReceive(g_baro_queue, &baro_data, pdMS_TO_TICKS(FUSION_BARO_ONLY_PERIOD_MS)) == pdTRUE)
        {
            last_pressure_pa = baro_data.pressure_pa;
            last_temperature_mc = baro_data.temperature_mc;
            sensor_valid = true;
            ekf_predict(&ekf_state, &ekf_cfg, 0.0f, baro_data.timestamp_us);
            ekf_update_baro(&ekf_state, &ekf_cfg, (float)baro_data.pressure_pa, baro_data.timestamp_us);
        }

        publish_flight_data(&ekf_state, &ekf_cfg, 0.0f, last_pressure_pa, last_temperature_mc, sensor_valid, false);

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(FUSION_BARO_ONLY_PERIOD_MS));
    }
}

void fusion_task_fn(void *param)
{
    fusion_task_ctx_t *ctx = (fusion_task_ctx_t *)param;

    if (ctx && ctx->imu)
        run_imu_fusion_loop(ctx->imu);
    else
        run_baro_only_loop();
}
