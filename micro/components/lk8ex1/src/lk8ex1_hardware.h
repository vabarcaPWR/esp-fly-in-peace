#ifndef LK8EX1_HARDWARE_H
#define LK8EX1_HARDWARE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#include "lk8ex1.h"

esp_err_t lk8ex1_hardware_format_payload(const lk8ex1_data_t *data, char *payload, size_t payload_size,
                                         size_t *payload_len);
bool lk8ex1_hardware_parse_hex_checksum(const char *hex_digits, uint8_t *out_checksum);
esp_err_t lk8ex1_hardware_compose_sentence(const char *payload, uint8_t checksum, char *buffer, size_t buffer_size);

#endif
