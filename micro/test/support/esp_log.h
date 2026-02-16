/**
 * @file esp_log.h
 * @brief Mock ESP-IDF esp_log.h for host-side unit testing.
 *
 * Replaces ESP_LOGx macros with no-ops so business logic can be tested on the host.
 */
#ifndef ESP_LOG_H
#define ESP_LOG_H

typedef enum
{
    ESP_LOG_NONE,
    ESP_LOG_ERROR,
    ESP_LOG_WARN,
    ESP_LOG_INFO,
    ESP_LOG_DEBUG,
    ESP_LOG_VERBOSE
} esp_log_level_t;

#define ESP_LOGE(tag, format, ...)
#define ESP_LOGW(tag, format, ...)
#define ESP_LOGI(tag, format, ...)
#define ESP_LOGD(tag, format, ...)
#define ESP_LOGV(tag, format, ...)

#endif /* ESP_LOG_H */
