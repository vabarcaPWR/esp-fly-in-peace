#include "lk8ex1_hardware.h"

#include <stdio.h>
#include <stdlib.h>

esp_err_t lk8ex1_hardware_format_payload(const lk8ex1_data_t *data, char *payload, size_t payload_size,
                                         size_t *payload_len)
{
    if (!data || !payload || !payload_len)
        return ESP_ERR_INVALID_ARG;

    int32_t battery_level = data->battery_mv;
    int32_t abs_temperature_dc = data->temperature_dc < 0 ? -data->temperature_dc : data->temperature_dc;
    int32_t temperature_whole = abs_temperature_dc / 10;
    int32_t temperature_fraction = abs_temperature_dc % 10;
    const char *temperature_sign = data->temperature_dc < 0 ? "-" : "";

    int result = snprintf(payload, payload_size, "LK8EX1,%ld,%ld,%ld,%s%ld.%ld,%ld,", (long)data->pressure_pa,
                          (long)data->altitude_m, (long)data->vario_cms, temperature_sign, (long)temperature_whole,
                          (long)temperature_fraction, (long)battery_level);
    if (result < 0 || (size_t)result >= payload_size)
        return ESP_FAIL;

    *payload_len = (size_t)result;
    return ESP_OK;
}

bool lk8ex1_hardware_parse_hex_checksum(const char *hex_digits, uint8_t *out_checksum)
{
    if (!hex_digits || !out_checksum)
        return false;

    char hex_str[3] = {hex_digits[0], hex_digits[1], '\0'};
    char *endptr = NULL;
    unsigned long value = strtoul(hex_str, &endptr, 16);
    if (&hex_str[2] != endptr)
        return false;

    *out_checksum = (uint8_t)value;
    return true;
}

esp_err_t lk8ex1_hardware_compose_sentence(const char *payload, uint8_t checksum, char *buffer, size_t buffer_size)
{
    if (!payload || !buffer)
        return ESP_ERR_INVALID_ARG;

    int written = snprintf(buffer, buffer_size, "$%s*%02X\r\n", payload, checksum);
    if (written < 0 || (size_t)written >= buffer_size)
        return ESP_ERR_INVALID_SIZE;

    return ESP_OK;
}
