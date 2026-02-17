#ifndef LK8EX1_CONDUCTOR_H
#define LK8EX1_CONDUCTOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#include "lk8ex1.h"

esp_err_t lk8ex1_conductor_format(const lk8ex1_data_t *data, char *buffer, size_t buffer_size);
uint8_t lk8ex1_conductor_checksum(const char *sentence, size_t len);
bool lk8ex1_conductor_validate(const char *sentence);

#endif
