#include "flight_data.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#define MUTEX_TIMEOUT_MS 10U

static SemaphoreHandle_t s_mutex = NULL;
static flight_data_t s_flight_data = {0};
static QueueHandle_t s_baro_queue = NULL;
static QueueHandle_t s_calibration_queue = NULL;

esp_err_t flight_data_init(void)
{
    s_mutex = xSemaphoreCreateMutex();
    if (!s_mutex)
        return ESP_ERR_NO_MEM;

    s_baro_queue = xQueueCreate(1, sizeof(data_baro_t));
    if (!s_baro_queue)
        return ESP_ERR_NO_MEM;

    s_calibration_queue = xQueueCreate(1, sizeof(calibration_request_t));
    if (!s_calibration_queue)
        return ESP_ERR_NO_MEM;

    return ESP_OK;
}

esp_err_t flight_data_publish(const flight_data_t *data)
{
    if (!data)
        return ESP_ERR_INVALID_ARG;

    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)) != pdTRUE)
        return ESP_ERR_TIMEOUT;

    s_flight_data = *data;
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

esp_err_t flight_data_read(flight_data_t *snapshot)
{
    if (!snapshot)
        return ESP_ERR_INVALID_ARG;

    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)) != pdTRUE)
        return ESP_ERR_TIMEOUT;

    *snapshot = s_flight_data;
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

esp_err_t baro_queue_send(const data_baro_t *data)
{
    if (!data)
        return ESP_ERR_INVALID_ARG;

    xQueueOverwrite(s_baro_queue, data);
    return ESP_OK;
}

esp_err_t baro_queue_receive(data_baro_t *out, uint32_t timeout_ms)
{
    if (!out)
        return ESP_ERR_INVALID_ARG;

    if (xQueueReceive(s_baro_queue, out, pdMS_TO_TICKS(timeout_ms)) != pdTRUE)
        return ESP_ERR_TIMEOUT;

    return ESP_OK;
}

esp_err_t calibration_queue_send(const calibration_request_t *req)
{
    if (!req)
        return ESP_ERR_INVALID_ARG;

    if (xQueueSend(s_calibration_queue, req, 0) != pdTRUE)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t calibration_queue_receive(calibration_request_t *out)
{
    if (!out)
        return ESP_ERR_INVALID_ARG;

    if (xQueueReceive(s_calibration_queue, out, 0) != pdTRUE)
        return ESP_ERR_NOT_FOUND;

    return ESP_OK;
}
