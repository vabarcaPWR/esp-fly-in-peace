#include "lk8ex1.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t xor_over_range(const char *data, size_t len)
{
    uint8_t checksum = 0;
    for (size_t i = 0; i < len; i++)
    {
        checksum ^= (uint8_t)data[i];
    }
    return checksum;
}

static const char *find_char(const char *str, size_t len, char target)
{
    for (size_t i = 0; i < len; i++)
    {
        if (str[i] == target)
        {
            return &str[i];
        }
    }
    return NULL;
}

static bool parse_hex_checksum(const char *hex_digits, uint8_t *out_checksum)
{
    char hex_str[3] = {hex_digits[0], hex_digits[1], '\0'};
    char *endptr = NULL;
    unsigned long value = strtoul(hex_str, &endptr, 16);
    if (endptr != &hex_str[2])
    {
        return false;
    }
    *out_checksum = (uint8_t)value;
    return true;
}

esp_err_t lk8ex1_format(const lk8ex1_data_t *data, char *buffer, size_t buffer_size)
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
    int payload_len =
        snprintf(payload, sizeof(payload), "LK8EX1,%ld,%ld,%ld,%ld,%ld", (long)data->pressure_pa,
                 (long)data->altitude_m, (long)data->vario_cms, (long)data->temperature_dc, (long)data->battery_mv);

    if (payload_len < 0 || (size_t)payload_len >= sizeof(payload))
    {
        return ESP_FAIL;
    }

    uint8_t checksum = xor_over_range(payload, (size_t)payload_len);

    int written = snprintf(buffer, buffer_size, "$%s*%02X\r\n", payload, checksum);
    if (written < 0 || (size_t)written >= buffer_size)
    {
        return ESP_ERR_INVALID_SIZE;
    }

    return ESP_OK;
}

uint8_t lk8ex1_checksum(const char *sentence, size_t len)
{
    if (sentence == NULL || len == 0)
    {
        return 0;
    }

    const char *payload_start = find_char(sentence, len, '$');
    if (payload_start == NULL)
    {
        return 0;
    }
    payload_start++;

    size_t remaining = (size_t)(sentence + len - payload_start);
    const char *payload_end = find_char(payload_start, remaining, '*');
    if (payload_end == NULL)
    {
        return 0;
    }

    return xor_over_range(payload_start, (size_t)(payload_end - payload_start));
}

bool lk8ex1_validate(const char *sentence)
{
    if (sentence == NULL)
    {
        return false;
    }

    size_t len = strlen(sentence);
    if (len < 7 || sentence[0] != '$')
    {
        return false;
    }

    const char *star = find_char(sentence + 1, len - 1, '*');
    if (star == NULL)
    {
        return false;
    }

    size_t chars_after_star = (size_t)(sentence + len - star - 1);
    if (chars_after_star < 2)
    {
        return false;
    }

    uint8_t embedded_checksum = 0;
    if (!parse_hex_checksum(star + 1, &embedded_checksum))
    {
        return false;
    }

    uint8_t computed_checksum = lk8ex1_checksum(sentence, len);
    return (computed_checksum == embedded_checksum);
}
