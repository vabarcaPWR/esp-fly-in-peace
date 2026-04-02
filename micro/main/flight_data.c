#include "flight_data.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#define MUTEX_TIMEOUT_MS 10U

typedef struct flight_data_context_s
{
    SemaphoreHandle_t mutex;
    flight_data_t data;
    QueueHandle_t baro_queue;
    QueueHandle_t calibration_queue;
} flight_data_context_t;

static flight_data_context_t s_self;

esp_err_t flight_data_init(void)
{
    s_self.mutex = xSemaphoreCreateMutex();
    if (!s_self.mutex)
        return ESP_ERR_NO_MEM;

    s_self.baro_queue = xQueueCreate(1, sizeof(data_baro_t));
    if (!s_self.baro_queue)
        return ESP_ERR_NO_MEM;

    s_self.calibration_queue = xQueueCreate(1, sizeof(calibration_request_t));
    if (!s_self.calibration_queue)
        return ESP_ERR_NO_MEM;

    return ESP_OK;
}

esp_err_t flight_data_publish(const flight_data_t *data)
{
    if (!data)
        return ESP_ERR_INVALID_ARG;

    if (xSemaphoreTake(s_self.mutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)) != pdTRUE)
        return ESP_ERR_TIMEOUT;

    s_self.data = *data;
    xSemaphoreGive(s_self.mutex);
    return ESP_OK;
}

esp_err_t flight_data_read(flight_data_t *snapshot)
{
    if (!snapshot)
        return ESP_ERR_INVALID_ARG;

    if (xSemaphoreTake(s_self.mutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)) != pdTRUE)
        return ESP_ERR_TIMEOUT;

    *snapshot = s_self.data;
    xSemaphoreGive(s_self.mutex);
    return ESP_OK;
}

esp_err_t baro_queue_send(const data_baro_t *data)
{
    if (!data)
        return ESP_ERR_INVALID_ARG;

    xQueueOverwrite(s_self.baro_queue, data);
    return ESP_OK;
}

esp_err_t baro_queue_receive(data_baro_t *out, uint32_t timeout_ms)
{
    if (!out)
        return ESP_ERR_INVALID_ARG;

    if (xQueueReceive(s_self.baro_queue, out, pdMS_TO_TICKS(timeout_ms)) != pdTRUE)
        return ESP_ERR_TIMEOUT;

    return ESP_OK;
}

esp_err_t calibration_queue_send(const calibration_request_t *req)
{
    if (!req)
        return ESP_ERR_INVALID_ARG;

    if (xQueueSend(s_self.calibration_queue, req, 0) != pdTRUE)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t calibration_queue_receive(calibration_request_t *out)
{
    if (!out)
        return ESP_ERR_INVALID_ARG;

    if (xQueueReceive(s_self.calibration_queue, out, 0) != pdTRUE)
        return ESP_ERR_NOT_FOUND;

    return ESP_OK;
}
