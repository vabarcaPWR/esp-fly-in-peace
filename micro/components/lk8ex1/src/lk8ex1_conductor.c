#include "lk8ex1_conductor.h"

#include "lk8ex1_hardware.h"
#include "lk8ex1_model.h"

esp_err_t lk8ex1_conductor_format(const lk8ex1_data_t *data, char *buffer, size_t buffer_size)
{
    if (data == NULL || buffer == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (buffer_size < LK8EX1_MAX_SENTENCE_LEN)
    {
        return ESP_ERR_INVALID_SIZE;
    }

    char payload[LK8EX1_MAX_SENTENCE_LEN];
    size_t payload_len = 0;
    esp_err_t format_result = lk8ex1_hardware_format_payload(data, payload, sizeof(payload), &payload_len);
    if (format_result != ESP_OK)
    {
        return format_result;
    }

    uint8_t checksum = lk8ex1_model_checksum_payload(payload, payload_len);
    return lk8ex1_hardware_compose_sentence(payload, checksum, buffer, buffer_size);
}

uint8_t lk8ex1_conductor_checksum(const char *sentence, size_t len)
{
    return lk8ex1_model_checksum_sentence(sentence, len);
}

bool lk8ex1_conductor_validate(const char *sentence)
{
    return lk8ex1_model_validate_sentence(sentence, lk8ex1_hardware_parse_hex_checksum);
}
