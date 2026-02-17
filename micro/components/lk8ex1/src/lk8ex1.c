#include "lk8ex1.h"

#include "lk8ex1_conductor.h"

esp_err_t lk8ex1_format(const lk8ex1_data_t *data, char *buffer, size_t buffer_size)
{
    return lk8ex1_conductor_format(data, buffer, buffer_size);
}

uint8_t lk8ex1_checksum(const char *sentence, size_t len)
{
    return lk8ex1_conductor_checksum(sentence, len);
}

bool lk8ex1_validate(const char *sentence)
{
    return lk8ex1_conductor_validate(sentence);
}
